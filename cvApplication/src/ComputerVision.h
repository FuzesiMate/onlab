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
#include "TemplateConfiguration.h"
#include "ImageProcessor.h"
#include "ArucoImageProcessor.h"
#include "Model.h"
#include "Camera.h"
#include "ObjectDataCollector.h"

#define CAMERA					"camera"
#define VISUALIZER				"visualizer"
#define OBJECTS					"objects"
#define TYPE					"type"
#define NUMBEROFCAMERAS 		"number"
#define EXPOSURE				"exposure"
#define GAIN					"gain"
#define FPS						"fps"
#define IMAGEPROCESSORS			"imageprocessors"
#define SHOW_WINDOW				"show_window"
#define SEND_DATA				"send_data"
#define SEND_RAW_DATA			"send_raw_data"
#define PATH_TO_MATRICES 		"path_to_matrices"
#define SPECIFICVALUES			"specificvalues"
#define ARUCO_IMAGE_PROCESSOR	"arucoimageprocessor"
#define IRTD_IMAGE_PROCESSOR	"irtimageprocessor"
#define CIRCLE_IMAGE_PROCESSOR	"circleimageprocessor"

#define DEFAULT_CAMERA	0

using t_cfg = TEMPLATE_CONFIG<tbb::concurrent_vector<cv::Point2f> , tbb::concurrent_vector<int > >;

class ComputerVision :public tbb::flow::graph{
private:
	std::unique_ptr<Camera> camera;
	std::shared_ptr<Model<t_cfg> > model;
	std::unique_ptr<ObjectDataCollector> dataCollector;

	//image processors mapped by markertype
	tbb::concurrent_unordered_map<MarkerType , std::shared_ptr<ImageProcessor<t_cfg> > > imageProcessors;

	boost::property_tree::ptree config;
	std::atomic_bool initialized;
	std::atomic_bool processing;

	void workflowController(std::shared_ptr<Model<t_cfg> > model , tbb::concurrent_unordered_map<MarkerType , int>& numberOfSuccessors );

public:
	ComputerVision():initialized(false),processing(false){};
	bool initialize(std::string configFilePath);
	void startProcessing();
	void stopProcessing();
	ModelData getData();
	void reconfigure(std::string configFilePath);
	bool isProcessing();
	virtual ~ComputerVision()=default;
};

#endif /* COMPUTERVISION_H_ */
