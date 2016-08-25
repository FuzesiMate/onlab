/*
 * ComputerVision.cpp
 *
 *  Created on: 2016. aug. 9.
 *      Author: Máté
 */

#include "ComputerVision.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <cstdio>
#include <exception>
#include "ArucoImageProcessor.h"
#include "CircleDetector.h"
#include "Visualizer.h"
#include "ArucoImageProcessor.cpp"
#include "CircleDetector.cpp"
#include "DataProvider.h"
#include "Object.cpp"
#include "Model.cpp"

bool ComputerVision::initialize(std::string configFilePath) {

	boost::property_tree::ptree config;
	try{
		boost::property_tree::read_json(configFilePath, config);
	}catch(std::exception& e){
		std::cout<<"JSON file is missing or invalid! Error message: "<<e.what()<<std::endl;
		initialized = false;
		return initialized;
	}

	camera = std::unique_ptr<Camera>(
			new Camera(config.get<int>(FPS) , config.get<int>(EXPOSURE) , config.get<int>(GAIN) , config.get<int>(NUMBEROFCAMERAS) , *this));

	auto cameraType = config.get<std::string>(CAMERATYPE);

	if(cameraType=="ximea"){
		initialized = camera->init(CV_CAP_XIAPI);
	}else if(cameraType=="default"){
		initialized = camera->init(DEFAULT);
	}

	if(config.get<int>(NUMBEROFCAMERAS)==2){
		initialized = initialized && camera->loadMatrices(config.get<std::string>(PATH_TO_MATRICES));
	}

	model = std::make_shared<Model<t_cfg> >();

	initialized = initialized && model->build(config , *this);

	processing = false;
	this->config = config;

	return initialized;
}

void ComputerVision::startProcessing() {

	std::cout<<"processing thread started"<<std::endl;

	//instantiate image processors and map them by markertype
	tbb::concurrent_unordered_map<MarkerType , std::shared_ptr<ImageProcessor<t_cfg> > > imageprocessors;

	//stores the number of active successors of the image processing node
	//mapped by markertype
	tbb::concurrent_unordered_map<MarkerType , int> numberOfSuccessors;

	for(auto& m : config.get_child(MARKERTYPES)){
		auto type = m.second.get<std::string>("");

		if(type=="aruco"){
			numberOfSuccessors[MarkerType::ARUCO] = 0;
			imageprocessors[MarkerType::ARUCO] = std::make_shared<ArucoImageProcessor<t_cfg> >(*this);
		}if(type=="circle"){
			numberOfSuccessors[MarkerType::CIRCLE] = 0;
			imageprocessors[MarkerType::CIRCLE] = std::make_shared<CircleDetector<t_cfg> >(*this);
		}
	}

	if(initialized){

		processing = true;

		tbb::flow::join_node<tbb::flow::tuple<Frame , ImageProcessingResult> , tbb::flow::queueing  > join(*this);

		tbb::concurrent_vector<std::shared_ptr<tbb::flow::sequencer_node<ImageProcessingData<t_cfg> > > > IPsequencers;

		tbb::concurrent_unordered_map<MarkerType , std::shared_ptr<tbb::flow::broadcast_node<ImageProcessingData<t_cfg> > > > broadcasters;

		for(auto& ip : imageprocessors){
			auto sequencer = std::make_shared<tbb::flow::sequencer_node<ImageProcessingData<t_cfg> > >
										(*this , [](ImageProcessingData<t_cfg> data)->size_t{
											return data.frameIndex;
										});

			broadcasters[ip.first] = std::make_shared<tbb::flow::broadcast_node<ImageProcessingData<t_cfg> > >(*this);

			IPsequencers.push_back(sequencer);

			/*
			 * Camera --> imageProcessor --> Sequencer --> broadcaster --> Object
			 */

			make_edge(camera->getProviderNode() , ip.second->getProcessorNode());
			make_edge(ip.second->getProcessorNode() , *IPsequencers[IPsequencers.size()-1]);
			make_edge(*IPsequencers[IPsequencers.size()-1] , *broadcasters[ip.first]);
		}

		tbb::flow::function_node<ImageProcessingResult , tbb::flow::continue_msg , tbb::flow::queueing> sink(
				*this, 1,
				[&](ImageProcessingResult data) {

					for(auto& o : data) {
						std::cout<<"object: "<<o.first<<std::endl;
						for(auto& m : o.second) {
							std::cout<<"    marker: "<<m.first<<std::endl;
								auto p = camera->getRealPosition(m.second);
								std::cout<<"       position: "<<p<<std::endl;
						}
					}
				});


		Visualizer visualizer(*this);


		provider = std::unique_ptr<DataProvider>(new DataProvider(model->getObjectNames().size() , *this));

		/*
		 * camera ----|
		 * 			  |
		 * 			join node --> Visualizer
		 * 			  |
		 * Provider --|--> DataSender
		 */

		if(config.get<std::string>(SHOW_WINDOW) == "true"){
			make_edge(camera->getProviderNode() , tbb::flow::input_port<0>(join));
			make_edge(provider->getProviderNode() , tbb::flow::input_port<1>(join));
			make_edge(join , visualizer.getProcessorNode());
		}

		auto objects = model->getObjectNames();

		tbb::concurrent_vector<std::shared_ptr<tbb::flow::sequencer_node< MarkerPosition > > > ObjectSequencers;

		for(auto& o : objects){
			numberOfSuccessors[model->getMarkerType(o)]++;
			make_edge(*broadcasters[model->getMarkerType(o)] , model->getObject(o)->getProcessorNode());

			auto tempSeq = std::make_shared<tbb::flow::sequencer_node< MarkerPosition > >(*this , [](MarkerPosition pos)->size_t{
				return pos.frameIndex;
			});

			ObjectSequencers.push_back(tempSeq);

			make_edge(model->getObject(o)->getProcessorNode() , *ObjectSequencers[ObjectSequencers.size()-1]);

			make_edge(*ObjectSequencers[ObjectSequencers.size()-1] , provider->getProcessorNode());
		}

		if(config.get<std::string>(SEND_DATA)=="true"){
			make_edge(provider->getProviderNode() , sink);
		}

		tbb::tbb_thread controllerThread(
						[&]() {
							while(isProcessing()) {
								auto objects = model->getObjectNames();

								for(auto& o : objects) {
									auto type = model->getMarkerType(o);

									if(model->isDone(o) && !model->isRemoved(o)) {
										numberOfSuccessors[type]--;
										model->remove(o);
									}
									if(numberOfSuccessors[type]==0) {
										remove_edge(camera->getProviderNode() , imageprocessors[type]->getProcessorNode());
									}
								}

								bool stop = true;
								for(auto& element : numberOfSuccessors){
									if(element.second>0){
										stop = false;
									}
								}
								if(stop){
									stopProcessing();
								}
								Sleep(100);
							}
							std::cout<<"controller thread stopped!"<<std::endl;
						});

		controllerThread.detach();


		bool started = false;
		tbb::flow::continue_node<tbb::flow::continue_msg> cont(*this , [&](tbb::flow::continue_msg out){
			if(!started){
				provider->start();
				started = true;
			}
		});


		make_edge(provider->getProcessorNode() , cont);

		camera->start();

		std::cout<<"wait for all"<<std::endl;
		this->wait_for_all();

		std::cout<<"Processing thread stopped!"<<std::endl;
	}else{
		std::cout<<"CV module is not initialized!"<<std::endl;
	}
}

void ComputerVision::stopProcessing() {
	std::cout<<"stop processing"<<std::endl;
	processing = false;
	camera->stop();
	provider->stop();
}

void ComputerVision::reconfigure(std::string configFilePath) {

}

bool ComputerVision::isProcessing(){
	return processing ;
}
