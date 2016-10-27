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
#include <opencv2/opencv.hpp>
#include <atomic>
#include <iostream>
#include <boost/property_tree/ptree.hpp>
#include "Object.h"
#include "FrameProvider.h"
#include "TemplateConfiguration.h"
#include "ImageProcessor.h"
#include "ModelDataStore.h"
#include "ObjectDataCollector.h"
#include "DataSender.h"

#define FRAME_SOURCE			"frameprovider"
#define VISUALIZER				"visualizer"
#define OBJECTS					"objects"
#define TYPE					"type"
#define NUMBEROFCAMERAS 		"number"
#define EXPOSURE				"exposure"
#define GAIN					"gain"
#define FPS						"fps"
#define IMAGEPROCESSORS			"imageprocessors"
#define SHOW_WINDOW				"show_window"
#define OBJECT_DATA_SENDERS		"objectdatasenders"
#define PATH_TO_MATRICES 		"path_to_matrices"
#define SPECIFICVALUES			"specificvalues"
#define ARUCO_IMAGE_PROCESSOR	"arucoimageprocessor"
#define IRTD_IMAGE_PROCESSOR	"irtdimageprocessor"
#define CIRCLE_IMAGE_PROCESSOR	"circleimageprocessor"

#define DEFAULT_CAMERA	0

using t_cfg = TEMPLATE_CONFIG<tbb::concurrent_vector<cv::Point2f>, tbb::concurrent_vector<int > >;

class ComputerVision :public tbb::flow::graph {
private:
	std::shared_ptr<FrameProvider> frameProvider;
	std::shared_ptr<ModelDataStore> model;
	std::unique_ptr<ObjectDataCollector> dataCollector;

	std::vector<std::shared_ptr<DataSender<ModelData> > > dataSenders;

	//image processors mapped by markertype
	tbb::concurrent_unordered_map<std::string, std::shared_ptr<ImageProcessor<t_cfg> > > imageProcessors;

	boost::property_tree::ptree config;
	std::atomic<bool> initialized;
	std::atomic<bool> processing;

	void workflowController(tbb::concurrent_unordered_map<std::string, std::shared_ptr<Object<t_cfg> > >& objects, tbb::concurrent_unordered_map<std::string, int>& numberOfSuccessors);

public:
	ComputerVision() :initialized(false), processing(false) {};
	bool initialize(std::string configFilePath);
	void startProcessing();
	void stopProcessing();
	ModelData getData();
	void reconfigure(std::string configFilePath);
	bool isProcessing();
	virtual ~ComputerVision() = default;
};

#endif /* COMPUTERVISION_H_ */
