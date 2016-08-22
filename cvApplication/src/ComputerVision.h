/*
 * ComputerVision.h
 *
 *  Created on: 2016. aug. 9.
 *      Author: Máté
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
#include "DataProvider.h"

#define CAMERATYPE		"cameratype"
#define NUMBEROFCAMERAS "numberofcameras"
#define EXPOSURE		"exposure"
#define GAIN			"gain"
#define FPS				"fps"
#define MARKERTYPES		"markertypes"

#define DEFAULT 		0

using t_cfg = TEMPLATE_CONFIG<tbb::concurrent_vector<cv::Point2f> , tbb::concurrent_vector<int > >;

class ComputerVision :public tbb::flow::graph{
private:
	std::unique_ptr<Camera> camera;
	std::shared_ptr<Model<t_cfg> > model;
	std::shared_ptr<DataProvider<t_cfg> > provider;

	boost::property_tree::ptree config;
	std::atomic_bool initialized;
	std::atomic_bool processing;

public:
	ComputerVision():initialized(false),processing(false){};
	bool initialize(std::string configFilePath);
	void startProcessing();
	void stopProcessing();
	void reconfigure(std::string configFilePath);
	bool isProcessing();
	virtual ~ComputerVision()=default;
};

#endif /* COMPUTERVISION_H_ */
