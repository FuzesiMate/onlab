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
#include "Object.cpp"
#include "Model.cpp"
#include "ObjectDataCollector.h"

std::map<std::string , MarkerType> res_MarkerType = {{"aruco",MarkerType::ARUCO},{"irtd", MarkerType::IRTD},{"circle",MarkerType::CIRCLE}};

void ComputerVision::workflowController(tbb::concurrent_unordered_map<std::string , std::shared_ptr<Object<t_cfg> > >& objects, tbb::concurrent_unordered_map<MarkerType , int>& aliveObjects ){

	while (isProcessing()) {

		for (auto& object : objects) {
			auto type = object.second->getMarkerType();

			if (object.second->isDone() && !object.second->isRemoved()) {
				aliveObjects[type]--;
				object.second->remove();
				std::cout<<object.first<<" is reached it's limit"<<std::endl;
			}
			if (aliveObjects[type] == 0) {
				remove_edge(camera->getProviderNode(),
						imageProcessors[type]->getProcessorNode());
			}
		}

		bool stop = true;
		for (auto& element : aliveObjects) {
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

	/*
	 * initialize method needs to be called before the start of the processing workflow
	 * the initialization of some cameras can take more time, so it is recommended
	 * to so this step before we start the processing thread in order to faster
	 * startup
	 */

	try{
		boost::property_tree::read_json(configFilePath, config);
	}catch(std::exception& e){
		std::cout<<"JSON file is missing or invalid! Error message: "<<e.what()<<std::endl;
		initialized = false;
		return initialized;
	}

	auto cameraConfig = config.get_child(CAMERA);
	camera = std::unique_ptr<Camera>(new Camera(cameraConfig.get<int>(FPS) , cameraConfig.get<int>(EXPOSURE) , cameraConfig.get<float>(GAIN) , cameraConfig.get<int>(NUMBEROFCAMERAS), *this));
	auto cameraType = cameraConfig.get<std::string>(TYPE);

	if(cameraType=="ximea"){
		initialized = camera->init(CV_CAP_XIAPI);
	}else if(cameraType=="default"){
		initialized = camera->init(DEFAULT_CAMERA);
	}else{
		initialized = false;
		return initialized;
	}

	return initialized;

}

void ComputerVision::startProcessing() {

	std::cout<<"processing thread started!"<<std::endl;

	if(initialized){

	/*
	 * this function blocks it's caller until the processing workflow ends
	 * we do not need these nodes outside this function, so they are instantiated locally
	 * if the processing ends, they will be destructed
	 */

	//objects mapped by their name
		tbb::concurrent_unordered_map<std::string , std::shared_ptr<Object<t_cfg> > > objects;

	//after each object there is a sequencer node, which restores the original order of the data
		tbb::concurrent_vector<std::shared_ptr<tbb::flow::sequencer_node< ObjectData > > > ObjectDataSequencers;

	//stores the number of active successors of the image processing node
	//mapped by markertype
		tbb::concurrent_unordered_map<MarkerType, int> aliveObjects;

	//limiter node limits the number of buffered frames in the system in order to reduce memory consumption
		tbb::flow::limiter_node<Frame> FrameLimiter(*this , 50);

	//after each imageprocessor there is a sequencer node, which restores the original order of the data
		tbb::concurrent_vector<std::shared_ptr<tbb::flow::sequencer_node<ImageProcessingData<t_cfg> > > > IpDatasequencers;

	//after each sequencer node there is a broadcaster node that broadcasts the output of the sequencer node
		tbb::concurrent_unordered_map<MarkerType , std::shared_ptr<tbb::flow::broadcast_node<ImageProcessingData<t_cfg> > > > IpDataBroadcasters;

	//visualizer to show the result of object tracking
		std::unique_ptr<Visualizer> visualizer;

	/*
	 * read image processor configuration, instantiate image processors
	 * in case of new image processing method add the markertype to the enumeration
	 * map the string and markertype in res_MarkerType
	 * instantiate your image processor class like the example below
	 */

	try{

		for(auto& imageProcessor : config.get_child("imageprocessors")){
			auto type = res_MarkerType[imageProcessor.second.get<std::string>("type")];

			switch(type){
				case MarkerType::ARUCO:
					imageProcessors[MarkerType::ARUCO] = std::make_shared<ArucoImageProcessor<t_cfg> >(*this);
					break;
				case MarkerType::IRTD:
					imageProcessors[MarkerType::IRTD] = std::make_shared<IRTDImageProcessor<t_cfg> >(*this);
					break;
				case MarkerType::CIRCLE:
					break;
			}

			aliveObjects[type] = 0;

			imageProcessors[type]->setProcessingSpecificValues(imageProcessor.second);

			for(auto& objectReference : imageProcessor.second.get_child(OBJECTS)){
				imageProcessors[type]->addObject(objectReference.second.get<std::string>(""));
				aliveObjects[type]++;
			}

			auto sequencer = std::make_shared<tbb::flow::sequencer_node<ImageProcessingData<t_cfg> > >
															(*this , [](ImageProcessingData<t_cfg> data)->size_t{
																return data.frameIndex;
															});

			IpDatasequencers.push_back(sequencer);

			IpDataBroadcasters[type] = std::make_shared<tbb::flow::broadcast_node<ImageProcessingData<t_cfg> > >(*this);
		}
	}catch(std::exception& e){
			std::cout<<"error while parsing image processor configuration! Error: "<<e.what()<<std::endl;
			processing = false;

			std::cout<<"Abort start processing..."<<std::endl;
			return;
		}

	/*
	 * Parse object configuration and instantiate objects
	 */

	try{
			dataCollector= std::unique_ptr<ObjectDataCollector>(new ObjectDataCollector(config.get_child(OBJECTS).size() , *this));

			for(auto& object : config.get_child(OBJECTS)){
				auto name = object.second.get<std::string>("name");
				auto limit = object.second.get<int>("limit");
				auto type = res_MarkerType[object.second.get<std::string>("markertype")];

				objects[name]= std::make_shared<Object <t_cfg> >(name , 0 , type , limit , *this);

				for(auto& marker : object.second.get_child("markers")){
					objects[name]->addMarker(marker.second.get<std::string>("name") , marker.second.get<int>("id"));
				}

				auto sequencer = std::make_shared<tbb::flow::sequencer_node< ObjectData > >(*this , [](ObjectData pos)->size_t{
							return pos.frameIndex;
					});

				ObjectDataSequencers.push_back(sequencer);

			}
		}catch(std::exception& e){
			std::cout<<"Error while parsing objects! Error message: "<<e.what()<<std::endl;
			processing = false;

			std::cout<<"Abort start processing..."<<std::endl;
			return;
		}

	//build the data flow graph
	//TODO comment more

	tbb::flow::join_node<tbb::flow::tuple<Frame , ModelData> , tbb::flow::queueing  > FrameModelDataJoiner(*this);

	try{
		auto visConfig = config.get_child(VISUALIZER);
		visualizer = std::unique_ptr<Visualizer>(new Visualizer(*this));
		make_edge(FrameLimiter , tbb::flow::input_port<0>(FrameModelDataJoiner));
		make_edge(dataCollector->getProviderNode() , tbb::flow::input_port<1>(FrameModelDataJoiner));
		make_edge(FrameModelDataJoiner , visualizer->getProcessorNode());
	}catch(std::exception& e){
		std::cout<<"No visualizer node instantiated!"<<std::endl;
	}

	bool started = false;
		tbb::flow::continue_node<tbb::flow::continue_msg> dataCollectorTrigger(*this , [&](tbb::flow::continue_msg out){
			if(!started){
				dataCollector->start();
				started = true;
			}
		});

	make_edge(dataCollector->getProcessorNode() , dataCollectorTrigger);

	make_edge(camera->getProviderNode() , FrameLimiter);

	int i = 0;
	for(auto& imageProcessor : imageProcessors){
		make_edge(FrameLimiter , imageProcessor.second->getProcessorNode());
		make_edge(imageProcessor.second->getProcessorNode() , *IpDatasequencers[i]);
		make_edge(*IpDatasequencers[i] , *IpDataBroadcasters[imageProcessor.first]);
		i++;

		for(auto& object : imageProcessor.second->getObjects()){
			make_edge(*IpDataBroadcasters[imageProcessor.first] , objects[object]->getProcessorNode());
		}
	}

	int j = 0;
	for(auto& object : objects){
		make_edge(object.second->getProcessorNode() , *ObjectDataSequencers[j]);
		make_edge(*ObjectDataSequencers[j] , dataCollector->getProcessorNode());
		j++;
	}

	make_edge(dataCollector->getProcessorNode() , FrameLimiter.decrement);

/*

		make_edge(camera->getProviderNode() , FrameLimiter);

		tbb::flow::join_node<tbb::flow::tuple<Frame , ModelData> , tbb::flow::queueing  > join(*this);





		for(auto& ip : imageProcessors){
			auto sequencer = std::make_shared<tbb::flow::sequencer_node<ImageProcessingData<t_cfg> > >
										(*this , [](ImageProcessingData<t_cfg> data)->size_t{
											return data.frameIndex;
										});

			IpDataBroadcasters[ip.first] = std::make_shared<tbb::flow::broadcast_node<ImageProcessingData<t_cfg> > >(*this);

			IpDatasequencers.push_back(sequencer);

			/*
			 * Camera --> imageProcessor --> Sequencer --> broadcaster --> Object


			//make_edge(camera->getProviderNode() , ip.second->getProcessorNode());

			make_edge(FrameLimiter , ip.second->getProcessorNode());
			make_edge(ip.second->getProcessorNode() , *IpDatasequencers[IpDatasequencers.size()-1]);
			make_edge(*IpDatasequencers[IpDatasequencers.size()-1] , *IpDataBroadcasters[ip.first]);
		}

		Visualizer visualizer(*this);


		provider = std::unique_ptr<ObjectDataCollector>(new ObjectDataCollector(model->getObjectNames().size() , *this));

		/*
		 * camera ----|
		 * 			  |
		 * 			join node --> Visualizer
		 * 			  |
		 * Provider --|--> DataSender


		if(config.get<std::string>(SHOW_WINDOW) == "true"){
			//make_edge(camera->getProviderNode() , tbb::flow::input_port<0>(join));
			make_edge(FrameLimiter , tbb::flow::input_port<0>(join));
		make_edge(provider->getProviderNode() , tbb::flow::input_port<1>(join));
			make_edge(join , visualizer.getProcessorNode());
		}

		auto objects = model->getObjectNames();



		for(auto& o : objects){

			aliveObjects[model->getMarkerType(o)]++;

			make_edge(*IpDataBroadcasters[model->getMarkerType(o)] , model->getObject(o)->getProcessorNode());

			auto tempSeq = std::make_shared<tbb::flow::sequencer_node< ObjectData > >(*this , [](ObjectData pos)->size_t{
				return pos.frameIndex;
			});

			ObjectDataSequencers.push_back(tempSeq);

			make_edge(model->getObject(o)->getProcessorNode() , *ObjectDataSequencers[ObjectDataSequencers.size()-1]);

			make_edge(*ObjectDataSequencers[ObjectDataSequencers.size()-1] , provider->getProcessorNode());

		}

		if(config.get<std::string>(SEND_DATA)=="true"){
		//	make_edge(provider->getProviderNode() , sink);
		}


		tbb::tbb_thread controllerThread(std::bind(&ComputerVision::workflowController ,this , model , aliveObjects));

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

		make_edge(transformer.getProcessorNode() , model->getProcessorNode());

		*/

		processing = true;

		tbb::tbb_thread flowController(std::bind(&ComputerVision::workflowController, this , objects , aliveObjects));
		flowController.detach();

		camera->start();

		this->wait_for_all();

		std::cout<<"Processing thread stopped!"<<std::endl;
	}else{
		std::cout<<"CV module is not initialized! Please initialize before start the processing workflow"<<std::endl;
	}
}

void ComputerVision::stopProcessing() {
	std::cout<<"stop processing"<<std::endl;
	processing = false;
	camera->stop();
	dataCollector->stop();
}

void ComputerVision::reconfigure(std::string configFilePath) {
	if(processing){

		boost::property_tree::read_json(configFilePath , config);

		auto cameraConfig = config.get_child(CAMERA);

		camera->setFPS(cameraConfig.get<int>(FPS));
		camera->setExposure(cameraConfig.get<int>(EXPOSURE));
		camera->setGain(cameraConfig.get<int>(GAIN));

		for(auto& ip : imageProcessors){
			auto ipList = config.get_child(IMAGEPROCESSORS);

			for(auto& ipElement : ipList){
				auto type = res_MarkerType[ipElement.second.get<std::string>("type")];

				imageProcessors[type]->setProcessingSpecificValues(ipElement.second);

			}
		}
	}
}

ModelData ComputerVision::getData(){
	if(processing){
		return model->getData();
	}
}

bool ComputerVision::isProcessing(){
	return processing ;
}
