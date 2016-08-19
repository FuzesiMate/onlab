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
#include "Visualizer.h"
#include "DataProvider.cpp"
#include "ArucoImageProcessor.cpp"
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
		initialized = camera->init(0);
	}

	model = std::make_shared<Model<t_cfg> >();

	initialized = initialized && model->build(config , *this);

	provider = std::make_shared<DataProvider<t_cfg> >(model , *this);

	processing = false;
	this->config = config;

	return initialized;
}

void ComputerVision::startProcessing() {

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
			imageprocessors[MarkerType::CIRCLE] = std::make_shared<ArucoImageProcessor<t_cfg> >(*this);
		}
	}

	if(initialized){

		processing = true;

		tbb::flow::join_node<tbb::flow::tuple<Frame , ImageProcessingResult> , tbb::flow::queueing  > join(*this);

		tbb::concurrent_vector<std::shared_ptr<tbb::flow::sequencer_node<ImageProcessingData<t_cfg> > > > sequencers;

		tbb::concurrent_unordered_map<MarkerType , std::shared_ptr<tbb::flow::broadcast_node<ImageProcessingData<t_cfg> > > > broadcasters;

		for(auto& ip : imageprocessors){
			auto sequencer = std::make_shared<tbb::flow::sequencer_node<ImageProcessingData<t_cfg> > >
										(*this , [](ImageProcessingData<t_cfg> data)->size_t{
											return data.frameIndex;
										});

			broadcasters[ip.first] = std::make_shared<tbb::flow::broadcast_node<ImageProcessingData<t_cfg> > >(*this);

			sequencers.push_back(sequencer);

			make_edge(*camera , *ip.second);
			make_edge(*ip.second , *sequencers[sequencers.size()-1]);
			make_edge(*sequencers[sequencers.size()-1] , *broadcasters[ip.first]);
		}

		tbb::flow::function_node<ImageProcessingResult> sink(
				*this, 1,
				[&](ImageProcessingResult data) {


					for(auto& o : data) {
						std::cout<<"object: "<<o.first<<std::endl;
						for(auto& m : o.second) {
							std::cout<<"    marker: "<<m.first<<std::endl;
							for(auto& p : m.second) {
								std::cout<<"       position: "<<p<<std::endl;
							}
						}
					}
				});

		Visualizer visualizer(*this);

	//	make_edge(*provider , sink);
		make_edge(*camera , tbb::flow::input_port<0>(join));
		make_edge(*provider , tbb::flow::input_port<1>(join));
		make_edge(join , visualizer);

		auto objects = model->getObjectNames();

		for(auto o : objects){
			numberOfSuccessors[model->getMarkerType(o)]++;
			make_edge(*broadcasters[model->getMarkerType(o)] , *model->getObject(o));
		}

		provider->start();
		camera->startRecording();

		tbb::tbb_thread controllerThread(
						[&]() {
							while(isProcessing()) {
								auto objects = model->getObjectNames();

								for(auto& o : objects) {
									auto type = model->getMarkerType(o);

									if(model->isDone(o) && !model->isRemoved(o)) {
										remove_edge(*broadcasters[type] , *model->getObject(o));
										numberOfSuccessors[type]--;
										model->remove(o);
									}
									if(numberOfSuccessors[type]==0) {
										remove_edge(*camera , *imageprocessors[type]);
									}
								}
								Sleep(20);
								bool stop = true;
								for(auto& element : numberOfSuccessors){
									if(element.second>0){
										stop = false;
									}
								}
								if(stop){
									stopProcessing();
								}
							}
						});

		controllerThread.detach();

		this->wait_for_all();
	}
}

void ComputerVision::stopProcessing() {
	//TODO does not stop properly...
	std::cout<<"stop processing"<<std::endl ;
	processing = false;
	camera->stopRecording();
	provider->stop();
}

void ComputerVision::reconfigure(std::string configFilePath) {

}

bool ComputerVision::isProcessing(){
	return processing ;
}
