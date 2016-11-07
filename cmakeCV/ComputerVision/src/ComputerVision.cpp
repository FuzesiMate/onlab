/*
 * ComputerVision.cpp
 *
 *  Created on: 2016. aug. 9.
 *      Author: M�t�
 */
#include "winsock2.h"
#include "ComputerVision.h"
#include <boost/property_tree/json_parser.hpp>
#include <tbb/tbb.h>
#include <thread>
#include <exception>
#include "DataTypes.h"
#include "ImageProcessorFactory.h"
#include "DataSenderFactory.h"
#include "ObjectDataCollector.h"
#include "FrameProvider.h"
#include "FrameProviderFactory.h"
#include "CoordinateTransformer.h"
#include "VisualizerFactory.h"
#include "Object.cpp"

bool ComputerVision::initialize(std::string configFilePath) {

	/*
	 * initialize method needs to be called before the start of the processing workflow
	 * the initialization of some cameras can take more time, so it is recommended
	 * to so this step before we start the processing thread in order to faster startup
	 *
	 * datasenders are also initialized in this step for the same reasons
	 */

	 
	//parse the config file into a boost property tree object
	try {
		boost::property_tree::read_json(configFilePath, config);
	}
	catch (std::exception& e) {
		std::cout << "JSON file is missing or invalid! Error message: " << e.what() << std::endl;
		initialized = false;
		return initialized;
	}

	frameProvider.reset();

	/*
	parse frame provider configuration and create an instance
	*/
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

	objectDataSenders.clear();
	ipDataSenders.clear();

	if (config.find(DATA_SENDERS) != config.not_found()) {

		try {
			auto senderConfig = config.get_child(DATA_SENDERS);
			for (auto& sender : senderConfig) {

				if (sender.second.find(IMAGEPROCESSOR) != sender.second.not_found()) {
					auto dataSender = DataSenderFactory::createDataSender<ImageProcessingData<t_cfg> >(sender.second, *this);
					auto ipType = sender.second.get<std::string>(IMAGEPROCESSOR);
					ipDataSenders[ipType] = dataSender;
				}
				else {
					auto dataSender = DataSenderFactory::createDataSender<ModelData>(sender.second, *this);
					objectDataSenders.push_back(dataSender);
				}
			}
		}
		catch (std::exception& e) {
			std::cout << "Error while creating data sender! Error message: " << e.what() << std::endl;
			initialized = false;
			return initialized;
		}
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

		//limiter node limits the number of buffered frames in the system in order to reduce memory consumption
		tbb::flow::limiter_node<Frame> FrameLimiter(*this, 10);

		//after each imageprocessor there is a sequencer node, which restores the original order of the data
		tbb::concurrent_vector<std::shared_ptr<tbb::flow::sequencer_node<ImageProcessingData<t_cfg> > > > IpDatasequencers;

		//after each sequencer node there is a broadcaster node that broadcasts the output of the sequencer node
		tbb::concurrent_unordered_map<std::string, std::shared_ptr<tbb::flow::broadcast_node<ImageProcessingData<t_cfg> > > > IpDataBroadcasters;

		//if all corresponding objects are limited the image processor will be also limited
		tbb::concurrent_unordered_map<std::string, std::shared_ptr< tbb::flow::limiter_node<Frame > > > ipLimiters;

		//if an object has a detection limit a limiter node will be applied before
		tbb::concurrent_unordered_map<std::string, std::shared_ptr<tbb::flow::limiter_node<ImageProcessingData<t_cfg> > > > objectLimiters;

		//visualizer to show the result of object tracking
		std::unique_ptr<Visualizer> visualizer;

		//transformer performs 2D-3D transformation from a stereo point
		std::unique_ptr<CoordinateTransformer> transformer;

		//data collector collects object data that corresponds to the same frame
		std::unique_ptr<ObjectDataCollector> dataCollector;

		//Local model data store to provide marker positions locally
		model = std::unique_ptr<ModelDataStore>(new ModelDataStore(*this));

		/* 
		 * CREATE IMAGE PROCESSORS 
		 * read image processor configuration, instantiate image processors
		 */
		try {

			for (auto& imageProcessor : config.get_child(IMAGEPROCESSORS)) {
				auto type = imageProcessor.second.get<std::string>(TYPE);

				try {
					imageProcessors[type] = ImageProcessorFactory::createImageProcessor<t_cfg>(imageProcessor.second, *this);

					auto sequencer = std::make_shared<tbb::flow::sequencer_node<ImageProcessingData<t_cfg> > >
						(*this, [](ImageProcessingData<t_cfg> data)->uint64_t {
						return data.frameIndex;
					});

					IpDatasequencers.push_back(sequencer);

					IpDataBroadcasters[type] = std::make_shared<tbb::flow::broadcast_node<ImageProcessingData<t_cfg> > >(*this);

				}
				catch (std::exception& e) {
					std::cout << "Error while creating image processor! Error message: " << e.what() << std::endl;
				}
			}
		}
		catch (std::exception& e) {
			std::cout << "Error while parsing image processor configuration! Error message: " << e.what() << std::endl;
			processing = false;
			return;
		}

		/*
		 * CREATE OBJECTS
		 * Parse object configuration and instantiate objects
		 */
		
		std::map<std::string, int> maxObjectLimitations;

		try {
			dataCollector = std::unique_ptr<ObjectDataCollector>(new ObjectDataCollector(config.get_child(OBJECTS).size(), *this));

			for (auto& object : config.get_child(OBJECTS)) {
				auto name = object.second.get<std::string>(NAME);
				auto limit = object.second.get<int>(LIMIT);
				auto type = object.second.get<std::string>(MARKER_TYPE);

				/*
				Find the limitation value to imageprocessors
				They will be stopped if there is no more object
				that needs their output
				*/
				if (maxObjectLimitations.find(type) == maxObjectLimitations.end()) {
					maxObjectLimitations[type] = limit;
				}
				else if ((maxObjectLimitations[type] < limit || limit<0) && maxObjectLimitations[type]>0 ) {
					maxObjectLimitations[type] = limit;
				}

				objects[name] = std::make_shared<Object <t_cfg> >(name, type, limit, *this);

				if (limit != -1) {
					objectLimiters[name] = std::make_shared<tbb::flow::limiter_node<ImageProcessingData<t_cfg> > >(*this , limit);
				}

				for (auto& marker : object.second.get_child(MARKERS)) {
					objects[name]->addMarker(marker.second.get<std::string>(NAME), marker.second.get<int>(ID));
				}
			}
		}
		catch (std::exception& e) {
			std::cout << "Error while parsing objects! Error message: " << e.what() << std::endl;
			processing = false;
			return;
		}

		//build the data flow graph

		/*
		connect the camera and the frame limiter node
		*/
		make_edge(frameProvider->getProviderNode(), FrameLimiter);
		
		/*
		Parse the configuration and instantiate visualizer module if required
		*/

		/*
		merge output data with the corresponding frame
		*/
		tbb::flow::join_node<tbb::flow::tuple<Frame, ModelData>, tbb::flow::queueing  > FrameModelDataJoiner(*this);

		if (config.find(VISUALIZER) != config.not_found()) {

			try {
				auto visConfig = config.get_child(VISUALIZER);

				try {
					visualizer = VisualizerFactory::createVisualizer(visConfig, *this);
				}
				catch (std::exception& e) {
					std::cout << "Error occured while creating visualizer! Error message: " << e.what() << std::endl;
					return;
				}

				/*
				the join node gets the output model instances and frames from the frame provider
				forwards the merged data to the visualizer module
				*/
				make_edge(FrameLimiter, tbb::flow::input_port<0>(FrameModelDataJoiner));
				make_edge(tbb::flow::output_port<0>(dataCollector->getCollectorNode()), tbb::flow::input_port<1>(FrameModelDataJoiner));
				make_edge(FrameModelDataJoiner, visualizer->getProcessorNode());
			}
			catch (std::exception& e) {
				std::cout << "Error occured while reading visualizer configuration! Error message: " << e.what() << std::endl;
				return;
			}
		}

		/*
		connect image processors to camera and corresponding objects
		if all objects assigned to an image processor is limited
		then the image processor will be also limited
		*/

		int i = 0;
		for (auto& imageProcessor : imageProcessors) {

			if (maxObjectLimitations[imageProcessor.first] < 0) {
				make_edge(FrameLimiter, imageProcessor.second->getProcessorNode());
			}
			else {
				ipLimiters[imageProcessor.first] = std::make_shared<tbb::flow::limiter_node<Frame> >(*this, maxObjectLimitations[imageProcessor.first]);
				make_edge(FrameLimiter, *ipLimiters[imageProcessor.first]);
				make_edge(*ipLimiters[imageProcessor.first], imageProcessor.second->getProcessorNode());
			}
			
			make_edge(imageProcessor.second->getProcessorNode(), *IpDatasequencers[i]);
			make_edge(*IpDatasequencers[i], *IpDataBroadcasters[imageProcessor.first]);

			if (ipDataSenders.find(imageProcessor.first) != ipDataSenders.end()) {
				make_edge(*IpDataBroadcasters[imageProcessor.first], ipDataSenders[imageProcessor.first]->getProcessorNode());
			}

			i++;
		}

		/*
		connect the object to the object data collector module
		if the object is limited a limiter node is applied before
		*/

		int j = 0;
		for (auto& object : objects) {
			if (objectLimiters.find(object.first)!=objectLimiters.end()) {
				make_edge(*IpDataBroadcasters[object.second->getMarkerType()], *objectLimiters[object.first]);
				make_edge(*objectLimiters[object.first], object.second->getProcessorNode());
			}
			else {
				make_edge(*IpDataBroadcasters[object.second->getMarkerType()], object.second->getProcessorNode());
			}

			make_edge(object.second->getProcessorNode(), dataCollector->getCollectorNode());

			j++;
		}

		/*
		trigger the camera when the final output is sent out
		*/
		make_edge(tbb::flow::output_port<1>(dataCollector->getCollectorNode()), FrameLimiter.decrement);

		

		/*
		apply 3D reconstruction
		insert a transformer module between object data collector and modules that receive its output
		*/
		if (config.find(TRANSFORMER) != config.not_found()) {
			try {
				auto transConfig = config.get_child(TRANSFORMER);
				auto pathToMatrices = transConfig.get<std::string>(PATH_TO_MATRICES);
				transformer = std::unique_ptr<CoordinateTransformer>(new CoordinateTransformer(*this));
				transformer->loadMatrices(pathToMatrices);
			}
			catch (std::exception& e) {
				std::cout << "Error occured while creating transformer! Error message: " << e.what() << std::endl;
				return;
			}

			make_edge(tbb::flow::output_port<0>(dataCollector->getCollectorNode()), transformer->getProcessorNode());
			make_edge(transformer->getProcessorNode(), model->getProcessorNode());
		}
		else {
			make_edge(tbb::flow::output_port<0>(dataCollector->getCollectorNode()), model->getProcessorNode());
		}
		
		for (auto& sender : objectDataSenders) {
			make_edge(model->getProcessorNode(), sender->getProcessorNode());
		}

		/*
		start the frame provider
		*/
		processing = true;
		frameProvider->start();
		
		std::cout << "Flow graph has been built successfully, start processing workflow" << std::endl;
		
		/*
		block until the processing workflow finishes
		*/
		try {
			this->wait_for_all();
		}
		catch (int& ex) {
			std::cout << "Error occured in the processing workflow!" << std::endl;
			this->reset();
		}

		std::cout << "Processing thread stopped!" << std::endl;
	}
	else {
		std::cout << "CV module is not initialized! Please initialize before start the processing workflow!" << std::endl;
	}
}

void ComputerVision::stopProcessing() {
	std::cout << "stop processing" << std::endl;
	processing = false;
	frameProvider->stop();
	//this->reset();
	//this->reset(tbb::flow::rf_clear_edges);
	//this->reset(tbb::flow::rf_reset_bodies);
}

void ComputerVision::reconfigure(std::string configFilePath) {
	if (processing) {

		try {
			boost::property_tree::read_json(configFilePath, config);

			auto cameraConfig = config.get_child(FRAME_SOURCE);

			frameProvider->setFPS(cameraConfig.get<int>(FPS));
			frameProvider->setExposure(cameraConfig.get<int>(EXPOSURE));
			frameProvider->setGain(cameraConfig.get<float>(GAIN));

			for (auto& ipConfig : config.get_child(IMAGEPROCESSORS)) {
				auto type = ipConfig.second.get<std::string>(TYPE);

				imageProcessors[type]->setProcessingSpecificValues(ipConfig.second);

			}

		}
		catch (std::exception& e) {
			std::cout << "Error occured while reconfiguring components! Error message: " << e.what() << std::endl;
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
