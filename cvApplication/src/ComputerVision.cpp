/*
 * ComputerVision.cpp
 *
 *  Created on: 2016. aug. 9.
 *      Author: M�t�
 */

#include "ComputerVision.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <tbb/compat/thread>
#include <cstdio>
#include <thread>
#include <exception>
#include "DataTypes.h"
#include "ArucoImageProcessor.h"
#include "CoordinateTransformer.h"
#include "CircleDetector.h"
#include "IRTDImageProcessor.h"
#include "Visualizer.h"
#include "ArucoImageProcessor.cpp"
#include "CircleDetector.cpp"
#include "IRTDImageProcessor.cpp"
#include "DataProvider.h"
#include "Object.cpp"
#include "Model.cpp"

void ComputerVision::workflowController(std::shared_ptr<Model<t_cfg> > model , tbb::concurrent_unordered_map<MarkerType , int>& numberOfSuccessors ){

	while (isProcessing()) {
		auto objects = model->getObjectNames();

		for (auto& o : objects) {
			auto type = model->getMarkerType(o);

			if (model->isDone(o) && !model->isRemoved(o)) {
				numberOfSuccessors[type]--;
				model->remove(o);
				std::cout<<o<<" is reached it's limit."<<std::endl;
			}
			if (numberOfSuccessors[type] == 0) {
				remove_edge(camera->getProviderNode(),
						imageprocessors[type]->getProcessorNode());
			}
		}

		bool stop = true;
		for (auto& element : numberOfSuccessors) {
			if (element.second > 0) {
				stop = false;
			}
		}
		if (stop) {
			stopProcessing();
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	std::cout << "controller thread stopped!" << std::endl;
}

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

	model = std::make_shared<Model<t_cfg> >();

	initialized = initialized && model->build(config , *this);

	processing = false;
	this->config = config;

	return initialized;
}

void ComputerVision::startProcessing() {

	std::cout<<"processing thread started"<<std::endl;

	//stores the number of active successors of the image processing node
	//mapped by markertype
	tbb::concurrent_unordered_map<MarkerType , int> numberOfSuccessors;

	for(auto& m : config.get_child(IMAGEPROCESSORS)){
		auto type = m.second.get<std::string>("type");

		if(type=="aruco"){
			numberOfSuccessors[MarkerType::ARUCO] = 0;
			imageprocessors[MarkerType::ARUCO] = std::make_shared<ArucoImageProcessor<t_cfg> >(*this);
			imageprocessors[MarkerType::ARUCO]->setProcessingSpecificValues(m.second.get_child("specificvalues"));
		}if(type=="circle"){
			numberOfSuccessors[MarkerType::CIRCLE] = 0;
			imageprocessors[MarkerType::CIRCLE] = std::make_shared<CircleDetector<t_cfg> >(*this);
			imageprocessors[MarkerType::CIRCLE]->setProcessingSpecificValues(m.second.get_child("specificvalues"));
		}if(type == "irtd"){
			numberOfSuccessors[MarkerType::IRTD] = 0;
			imageprocessors[MarkerType::IRTD] = std::make_shared<IRTDImageProcessor<t_cfg> >(*this);
			imageprocessors[MarkerType::IRTD]->setProcessingSpecificValues(m.second.get_child("specificvalues"));
		}
	}

	if(initialized){

		std::cout<<"make edges"<<std::endl;

		processing = true;

		tbb::flow::limiter_node<Frame> FrameLimiter(*this , 5);

		make_edge(camera->getProviderNode() , FrameLimiter);

		tbb::flow::join_node<tbb::flow::tuple<Frame , ModelData> , tbb::flow::queueing  > join(*this);

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

			//make_edge(camera->getProviderNode() , ip.second->getProcessorNode());

			make_edge(FrameLimiter , ip.second->getProcessorNode());
			make_edge(ip.second->getProcessorNode() , *IPsequencers[IPsequencers.size()-1]);
			make_edge(*IPsequencers[IPsequencers.size()-1] , *broadcasters[ip.first]);
		}

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
			//make_edge(camera->getProviderNode() , tbb::flow::input_port<0>(join));
			make_edge(FrameLimiter , tbb::flow::input_port<0>(join));
			make_edge(provider->getProviderNode() , tbb::flow::input_port<1>(join));
			make_edge(join , visualizer.getProcessorNode());
		}

		auto objects = model->getObjectNames();

		tbb::concurrent_vector<std::shared_ptr<tbb::flow::sequencer_node< ObjectData > > > ObjectSequencers;

		for(auto& o : objects){

			numberOfSuccessors[model->getMarkerType(o)]++;

			make_edge(*broadcasters[model->getMarkerType(o)] , model->getObject(o)->getProcessorNode());

			auto tempSeq = std::make_shared<tbb::flow::sequencer_node< ObjectData > >(*this , [](ObjectData pos)->size_t{
				return pos.frameIndex;
			});

			ObjectSequencers.push_back(tempSeq);

			make_edge(model->getObject(o)->getProcessorNode() , *ObjectSequencers[ObjectSequencers.size()-1]);

			make_edge(*ObjectSequencers[ObjectSequencers.size()-1] , provider->getProcessorNode());

		}

		if(config.get<std::string>(SEND_DATA)=="true"){
		//	make_edge(provider->getProviderNode() , sink);
		}


		tbb::tbb_thread controllerThread(std::bind(&ComputerVision::workflowController ,this , model , numberOfSuccessors));

		controllerThread.detach();

		bool started = false;
		tbb::flow::continue_node<tbb::flow::continue_msg> cont(*this , [&](tbb::flow::continue_msg out){
			if(!started){
				provider->start();
				started = true;
			}
		});


		make_edge(provider->getProcessorNode() , cont);

		make_edge(provider->getProcessorNode() , FrameLimiter.decrement);

		CoordinateTransformer transformer(*this);

		transformer.loadMatrices(config.get<std::string>(PATH_TO_MATRICES));

		make_edge(provider->getProviderNode() , transformer.getProcessorNode());

		camera->start();

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

	boost::property_tree::read_json(configFilePath , config);

	for(auto& ip : imageprocessors){
		auto ipList = config.get_child(IMAGEPROCESSORS);

		for(auto& ipElement : ipList){
			auto type = ipElement.second.get<std::string>("type");
			auto values = ipElement.second.get_child(SPECIFICVALUES);

			if (type == "aruco") {
				imageprocessors[MarkerType::ARUCO]->setProcessingSpecificValues(values);
			}
			if (type == "circle") {
				imageprocessors[MarkerType::CIRCLE]->setProcessingSpecificValues(values);
			}
			if (type == "irtd") {
				imageprocessors[MarkerType::IRTD]->setProcessingSpecificValues(values);
			}
		}
	}
}

bool ComputerVision::isProcessing(){
	return processing ;
}
