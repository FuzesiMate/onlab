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
#include <thread>
#include <exception>
#include "DataTypes.h"
#include "ImageProcessorFactory.h"
#include "DataSenderFactory.h"
#include "ObjectDataCollector.h"
#include "FrameProvider.h"
#include "Camera.h"
#include "FrameProviderFactory.h"
#include "ArucoImageProcessor.h"
#include "CoordinateTransformer.h"
#include "CircleDetector.h"
#include "IRTDImageProcessor.h"
#include "Visualizer.h"
#include "ArucoImageProcessor.cpp"
#include "CircleDetector.cpp"
#include "IRTDImageProcessor.cpp"
#include "Object.cpp"

void ComputerVision::workflowController(tbb::concurrent_unordered_map<std::string, std::shared_ptr<Object<t_cfg> > >& objects, tbb::concurrent_unordered_map<std::string, int>& aliveObjects) {

	while (isProcessing()) {

		for (auto& object : objects) {
			auto type = object.second->getMarkerType();

			if (object.second->isDone() && !object.second->isRemoved()) {
				aliveObjects[type]--;
				object.second->remove();
				std::cout << object.first << " is reached it's limit" << std::endl;
			}
			if (aliveObjects[type] == 0) {
				remove_edge(frameProvider->getProviderNode(),
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
	 * to so this step before we start the processing thread in order to faster startup
	 *
	 * datasenders are also initialized in this step for the same reasons
	 */


	try {
		boost::property_tree::read_json(configFilePath, config);
	}
	catch (std::exception& e) {
		std::cout << "JSON file is missing or invalid! Error message: " << e.what() << std::endl;
		initialized = false;
		return initialized;
	}

	try {
		auto cameraConfig = config.get_child(FRAME_SOURCE);
		auto type = cameraConfig.get<std::string>(TYPE);

		try {
			frameProvider = FrameProviderFactory::createFrameProvider(cameraConfig, *this);
			initialized = true;
		}
		catch (std::exception& e) {
			std::cout << "Error occured while creating frame provider! Error message: " << e.what() << std::endl;
			initialized = false;
			return initialized;
		}

	}
	catch (std::exception& e) {
		std::cout << "Error occured while parsing camera configuration! Error message: " << e.what() << std::endl;
		initialized = false;
		return initialized;
	}

	try {
		auto senderConfig = config.get_child("objectdatasenders");
		for (auto& sender : senderConfig) {

			auto temp = DataSenderFactory::createDataSender(sender.second, *this);
				dataSenders.push_back(temp);
			}
		}catch (std::exception& e) {
			std::cout << "failed " << e.what() << std::endl;
		}

	return initialized;
}

void ComputerVision::startProcessing() {

	std::cout << "processing thread started!" << std::endl;

	if (initialized) {

		/*
		 * this function blocks it's caller until the processing workflow ends
		 * we do not need these nodes outside this function, so they are instantiated locally
		 * if the processing ends, they will be destructed
		 */

		 //objects mapped by their name
		tbb::concurrent_unordered_map<std::string, std::shared_ptr<Object<t_cfg> > > objects;

		//after each object there is a sequencer node, which restores the original order of the data
		tbb::concurrent_vector<std::shared_ptr<tbb::flow::sequencer_node< ObjectData > > > ObjectDataSequencers;

		//stores the number of active successors of the image processing node
		//mapped by markertype
		tbb::concurrent_unordered_map<std::string, int> aliveObjects;

		//limiter node limits the number of buffered frames in the system in order to reduce memory consumption
		tbb::flow::limiter_node<Frame> FrameLimiter(*this, 10);

		//after each imageprocessor there is a sequencer node, which restores the original order of the data
		tbb::concurrent_vector<std::shared_ptr<tbb::flow::sequencer_node<ImageProcessingData<t_cfg> > > > IpDatasequencers;

		//after each sequencer node there is a broadcaster node that broadcasts the output of the sequencer node
		tbb::concurrent_unordered_map<std::string, std::shared_ptr<tbb::flow::broadcast_node<ImageProcessingData<t_cfg> > > > IpDataBroadcasters;

		//visualizer to show the result of object tracking
		std::unique_ptr<Visualizer> visualizer;

		//Local model data store to provide marker positions locally
		model = std::unique_ptr<ModelDataStore>(new ModelDataStore(*this));

		/*
		 * read image processor configuration, instantiate image processors
		 * in case of new image processing method add the markertype to the enumeration
		 * map the string and markertype in res_MarkerType
		 * instantiate your image processor class like the example below
		 */

		try {

			for (auto& imageProcessor : config.get_child(IMAGEPROCESSORS)) {
				auto type = imageProcessor.second.get<std::string>(TYPE);

				try {
					imageProcessors[type] = ImageProcessorFactory::createImageProcessor<t_cfg>(imageProcessor.second, *this);

					aliveObjects[type] = 0;

					for (auto& objectReference : imageProcessor.second.get_child(OBJECTS)) {
						imageProcessors[type]->addObject(objectReference.second.get<std::string>(""));
						aliveObjects[type]++;
					}

					auto sequencer = std::make_shared<tbb::flow::sequencer_node<ImageProcessingData<t_cfg> > >
						(*this, [](ImageProcessingData<t_cfg> data)->size_t {
						return data.frameIndex;
					});

					IpDatasequencers.push_back(sequencer);

					IpDataBroadcasters[type] = std::make_shared<tbb::flow::broadcast_node<ImageProcessingData<t_cfg> > >(*this);
				}
				catch (std::exception& e) {
					std::cout << "Error while creating data sender! Error message: " << e.what() << std::endl;
				}
			}
		}
		catch (std::exception& e) {
			std::cout << "Error while parsing image processor configuration! Error message: " << e.what() << std::endl;
			processing = false;

			std::cout << "Abort start processing..." << std::endl;
			return;
		}

		/*
		 * Parse object configuration and instantiate objects
		 */

		try {
			dataCollector = std::unique_ptr<ObjectDataCollector>(new ObjectDataCollector(config.get_child(OBJECTS).size(), *this));

			for (auto& object : config.get_child(OBJECTS)) {
				auto name = object.second.get<std::string>("name");
				auto limit = object.second.get<int>("limit");
				auto type = object.second.get<std::string>("markertype");

				objects[name] = std::make_shared<Object <t_cfg> >(name, 0, type, limit, *this);

				for (auto& marker : object.second.get_child("markers")) {
					objects[name]->addMarker(marker.second.get<std::string>("name"), marker.second.get<int>("id"));
				}

				auto sequencer = std::make_shared<tbb::flow::sequencer_node< ObjectData > >(*this, [](ObjectData pos)->size_t {
					return pos.frameIndex;
				});

				ObjectDataSequencers.push_back(sequencer);

			}
		}
		catch (std::exception& e) {
			std::cout << "Error while parsing objects! Error message: " << e.what() << std::endl;
			processing = false;

			std::cout << "Abort start processing..." << std::endl;
			return;
		}

		//build the data flow graph
		//TODO comment more

		tbb::flow::join_node<tbb::flow::tuple<Frame, ModelData>, tbb::flow::queueing  > FrameModelDataJoiner(*this);

		try {
			auto visConfig = config.get_child(VISUALIZER);
			visualizer = std::unique_ptr<Visualizer>(new Visualizer(*this));
			make_edge(FrameLimiter, tbb::flow::input_port<0>(FrameModelDataJoiner));
			make_edge(dataCollector->getProviderNode(), tbb::flow::input_port<1>(FrameModelDataJoiner));
			make_edge(FrameModelDataJoiner, visualizer->getProcessorNode());
		}
		catch (std::exception& e) {
			std::cout << "No visualizer node instantiated!" << std::endl;
		}

		bool started = false;
		tbb::flow::continue_node<tbb::flow::continue_msg> dataCollectorTrigger(*this, [&](tbb::flow::continue_msg out) {
			if (!started) {
				dataCollector->start();
				started = true;
			}
		});

		make_edge(dataCollector->getProcessorNode(), dataCollectorTrigger);

		make_edge(frameProvider->getProviderNode(), FrameLimiter);

		int i = 0;
		for (auto& imageProcessor : imageProcessors) {
			make_edge(FrameLimiter, imageProcessor.second->getProcessorNode());
			make_edge(imageProcessor.second->getProcessorNode(), *IpDatasequencers[i]);
			make_edge(*IpDatasequencers[i], *IpDataBroadcasters[imageProcessor.first]);
			i++;

			for (auto& object : imageProcessor.second->getObjects()) {
				make_edge(*IpDataBroadcasters[imageProcessor.first], objects[object]->getProcessorNode());
			}
		}

		int j = 0;
		for (auto& object : objects) {
			make_edge(object.second->getProcessorNode(), *ObjectDataSequencers[j]);
			make_edge(*ObjectDataSequencers[j], dataCollector->getProcessorNode());
			j++;
		}

		make_edge(dataCollector->getProcessorNode(), FrameLimiter.decrement);

		for (auto& sender : dataSenders) {
			make_edge(dataCollector->getProviderNode(), sender->getProcessorNode());
		}

		make_edge(dataCollector->getProviderNode() , model->getProcessorNode());

		processing = true;

		tbb::tbb_thread flowController(std::bind(&ComputerVision::workflowController, this, objects, aliveObjects));
		flowController.detach();

		frameProvider->start();

		std::cout << "Flow graph has been built successfully, start processing workflow" << std::endl;

		this->wait_for_all();

		std::cout << "Processing thread stopped!" << std::endl;
	}
	else {
		std::cout << "CV module is not initialized! Please initialize before start the processing workflow" << std::endl;
	}
}

void ComputerVision::stopProcessing() {
	std::cout << "stop processing" << std::endl;
	processing = false;
	frameProvider->stop();
	dataCollector->stop();
}

void ComputerVision::reconfigure(std::string configFilePath) {
	if (processing) {

		boost::property_tree::read_json(configFilePath, config);

		auto cameraConfig = config.get_child(FRAME_SOURCE);

		frameProvider->setFPS(cameraConfig.get<int>(FPS));
		frameProvider->setExposure(cameraConfig.get<int>(EXPOSURE));
		frameProvider->setGain(cameraConfig.get<int>(GAIN));

		for (auto& ip : imageProcessors) {
			auto ipList = config.get_child(IMAGEPROCESSORS);

			for (auto& ipElement : ipList) {
				auto type = ipElement.second.get<std::string>(TYPE);

				imageProcessors[type]->setProcessingSpecificValues(ipElement.second);

			}
		}
	}
}

ModelData ComputerVision::getData() {
	if (processing) {
		return model->getData();
	}
	else {
		ModelData dummy;
		return dummy;
	}
}

bool ComputerVision::isProcessing() {
	return processing;
}
