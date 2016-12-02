/*
 * ComputerVision.h
 *
 *  Created on: 2016. aug. 9.
 *      Author: M�t�
 */

#ifndef COMPUTERVISION_H_
#define COMPUTERVISION_H_

#include <tbb/flow_graph.h>
#include <tbb/concurrent_vector.h>
#include <tbb/concurrent_unordered_map.h>
#include <opencv2/opencv.hpp>
#include <atomic>
#include <iostream>
#include <map>
#include <boost/property_tree/ptree.hpp>
#include "Object.h"
#include "FrameProvider.h"
#include "TemplateConfiguration.h"
#include "ImageProcessor.h"
#include "ModelDataStore.h"
#include "ObjectDataCollector.h"
#include "DataSender.h"
#include "DataTypes.h"

#define FRAME_PROVIDER			"frameprovider"
#define VISUALIZER				"visualizer"
#define OBJECTS					"objects"
#define TYPE					"type"
#define NUMBEROFCAMERAS 		"number"
#define EXPOSURE				"exposure"
#define GAIN					"gain"
#define FPS						"fps"
#define IMAGEPROCESSORS			"imageprocessors"
#define IMAGEPROCESSOR			"imageprocessor"
#define SHOW_WINDOW				"show_window"
#define DATA_SENDERS			"datasenders"
#define PATH_TO_MATRICES 		"path_to_matrices"
#define TRANSFORMER				"transformer"
#define NAME					"name"
#define LIMIT					"limit"
#define MARKER_TYPE				"markertype"
#define ID						"id"
#define MARKERS					"markers"

#define DEFAULT_CAMERA	0

using t_cfg					= TEMPLATE_CONFIG<tbb_vector<cv::Point2f>, tbb_vector<int > >;

using ip_data_sequencer		= tbb::flow::sequencer_node<ImageProcessingData<t_cfg> >;
using ip_data_broadcaster	= tbb::flow::broadcast_node<ImageProcessingData<t_cfg> >;
using frame_limiter			= tbb::flow::limiter_node<Frame >;
using object_limiter		= tbb::flow::limiter_node<ImageProcessingData<t_cfg> >;

class ComputerVision  :public tbb::flow::graph {
private:
	std::unique_ptr<FrameProvider> frameProvider;
	std::unique_ptr<ModelDataStore> model;
	std::vector<std::shared_ptr<DataSender<ModelData> > > objectDataSenders;
	//raw image processing data senders mapped by markertype
	std::map < std::string, std::shared_ptr<DataSender<ImageProcessingData<t_cfg> > > > ipDataSenders;
	//image processors mapped by markertype
	std::map <std::string, std::shared_ptr<ImageProcessor<t_cfg> > > imageProcessors;
	//configuration instance
	boost::property_tree::ptree config;
	std::atomic<bool> initialized;
	std::atomic<bool> processing;

public:
	ComputerVision():initialized(false), processing(false),model(std::make_unique<ModelDataStore>(*this)) {};
	bool initialize(const std::string configFilePath);
	void startProcessing();
	void stopProcessing();
	bool getData(ModelData& output);
	void reconfigure(const std::string configFilePath);
	bool isProcessing();
	virtual ~ComputerVision() = default;
};

#endif /* COMPUTERVISION_H_ */
