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

void ComputerVision::workflowController(std::shared_ptr<Model<t_cfg> > model , tbb::concurrent_unordered_map<MarkerType , int>& aliveObjects ){

	while (isProcessing()) {
		auto objects = model->getObjectNames();

		for (auto& o : objects) {
			auto type = model->getMarkerType(o);

			if (model->isDone(o) && !model->isRemoved(o)) {
				aliveObjects[type]--;
				model->remove(o);
				std::cout<<o<<" is reached it's limit"<<std::endl;
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

	//read object configuration,instantiate objects and add markers
	try{
		dataCollector= std::unique_ptr<ObjectDataCollector>(new ObjectDataCollector(3 , *this));

		for(auto& object : config.get_child(OBJECTS)){
			auto name = object.second.get<std::string>("name");
			auto limit = object.second.get<int>("limit");

			objects[name]= std::make_shared<Object <t_cfg> >(name , 0 ,MarkerType::ARUCO , limit , *this);

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
	}

	//read image processor configuration, instantiate image processors
	try{
		auto IpConfig = config.get_child(ARUCO_IMAGE_PROCESSOR);
		aliveObjects[MarkerType::ARUCO] = 0;
		imageProcessors[MarkerType::ARUCO] = std::make_shared<ArucoImageProcessor<t_cfg> >(*this);
		imageProcessors[MarkerType::ARUCO]->setProcessingSpecificValues(IpConfig);

		auto sequencer = std::make_shared<tbb::flow::sequencer_node<ImageProcessingData<t_cfg> > >
												(*this , [](ImageProcessingData<t_cfg> data)->size_t{
													return data.frameIndex;
												});

		IpDatasequencers.push_back(sequencer);

		IpDataBroadcasters[MarkerType::ARUCO] = std::make_shared<tbb::flow::broadcast_node<ImageProcessingData<t_cfg> > >(*this);

		for(auto& objectRef : IpConfig.get_child("objects")){
			imageProcessors[MarkerType::ARUCO]->addObject(objectRef.second.get<std::string>(""));
		}

	}catch(std::exception& e){

	}

	try{
		auto IpConfig = config.get_child(IRTD_IMAGE_PROCESSOR);
		aliveObjects[MarkerType::IRTD] = 0;
		imageProcessors[MarkerType::IRTD] = std::make_shared<IRTDImageProcessor<t_cfg> >(*this);
		imageProcessors[MarkerType::IRTD]->setProcessingSpecificValues(IpConfig);

		auto sequencer = std::make_shared<tbb::flow::sequencer_node<ImageProcessingData<t_cfg> > >
														(*this , [](ImageProcessingData<t_cfg> data)->size_t{
															return data.frameIndex;
														});

		IpDatasequencers.push_back(sequencer);

		IpDataBroadcasters[MarkerType::IRTD] = std::make_shared<tbb::flow::broadcast_node<ImageProcessingData<t_cfg> > >(*this);

		for(auto& objectRef : IpConfig.get_child("objects")){
			imageProcessors[MarkerType::IRTD]->addObject(objectRef.second.get<std::string>(""));
		}

	}catch(std::exception& e){

	}

	try{
		auto IpConfig = config.get_child(CIRCLE_IMAGE_PROCESSOR);
		aliveObjects[MarkerType::CIRCLE]=0;
		imageProcessors[MarkerType::CIRCLE] = std::make_shared<IRTDImageProcessor<t_cfg> >(*this);
		imageProcessors[MarkerType::CIRCLE]->setProcessingSpecificValues(IpConfig);

		auto sequencer = std::make_shared<tbb::flow::sequencer_node<ImageProcessingData<t_cfg> > >
														(*this , [](ImageProcessingData<t_cfg> data)->size_t{
															return data.frameIndex;
														});

		IpDatasequencers.push_back(sequencer);

		IpDataBroadcasters[MarkerType::CIRCLE] = std::make_shared<tbb::flow::broadcast_node<ImageProcessingData<t_cfg> > >(*this);

		for(auto& objectRef : IpConfig.get_child("objects")){
			imageProcessors[MarkerType::CIRCLE]->addObject(objectRef.second.get<std::string>(""));
		}

	}catch(std::exception& e){

	}

	tbb::flow::join_node<tbb::flow::tuple<Frame , ModelData> , tbb::flow::queueing  > FrameModelDataJoiner(*this);

	try{
		auto visConfig = config.get_child(VISUALIZER);
		visualizer = std::unique_ptr<Visualizer>(new Visualizer(*this));
		make_edge(FrameLimiter , tbb::flow::input_port<0>(FrameModelDataJoiner));
		make_edge(dataCollector->getProviderNode() , tbb::flow::input_port<1>(FrameModelDataJoiner));
		make_edge(FrameModelDataJoiner , visualizer->getProcessorNode());
	}catch(std::exception& e){

	}

	bool started = false;
		tbb::flow::continue_node<tbb::flow::continue_msg> dataCollectorTrigger(*this , [&](tbb::flow::continue_msg out){
			if(!started){
				dataCollector->start();
				started = true;
			}
		});

	make_edge(dataCollector->getProcessorNode() , dataCollectorTrigger);

	//build the data flow graph

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


/*
	for(auto& m : config.get_child(IMAGEPROCESSORS)){
		auto type = m.second.get<std::string>("type");

		if(type=="aruco"){
			aliveObjects[MarkerType::ARUCO] = 0;
			imageProcessors[MarkerType::ARUCO] = std::make_shared<ArucoImageProcessor<t_cfg> >(*this);
			imageProcessors[MarkerType::ARUCO]->setProcessingSpecificValues(m.second.get_child("specificvalues"));
		}if(type=="circle"){
			aliveObjects[MarkerType::CIRCLE] = 0;
			imageProcessors[MarkerType::CIRCLE] = std::make_shared<CircleDetector<t_cfg> >(*this);
			imageProcessors[MarkerType::CIRCLE]->setProcessingSpecificValues(m.second.get_child("specificvalues"));
		}if(type == "irtd"){
			aliveObjects[MarkerType::IRTD] = 0;
			imageProcessors[MarkerType::IRTD] = std::make_shared<IRTDImageProcessor<t_cfg> >(*this);
			imageProcessors[MarkerType::IRTD]->setProcessingSpecificValues(m.second.get_child("specificvalues"));
		}
	}

	*/



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
	dataCollector->stop();
}

void ComputerVision::reconfigure(std::string configFilePath) {
	if(processing){

		boost::property_tree::read_json(configFilePath , config);

		camera->setFPS(config.get<int>(FPS));
		camera->setExposure(config.get<int>(EXPOSURE));
		camera->setGain(config.get<int>(GAIN));

		for(auto& ip : imageProcessors){
			auto ipList = config.get_child(IMAGEPROCESSORS);

			for(auto& ipElement : ipList){
				auto type = ipElement.second.get<std::string>("type");
				auto values = ipElement.second.get_child(SPECIFICVALUES);

				if (type == "aruco") {
					imageProcessors[MarkerType::ARUCO]->setProcessingSpecificValues(values);
				}
				if (type == "circle") {
					imageProcessors[MarkerType::CIRCLE]->setProcessingSpecificValues(values);
				}
				if (type == "irtd") {
					imageProcessors[MarkerType::IRTD]->setProcessingSpecificValues(values);
				}
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
