/*
 * ComputerVision.h
 *
 *  Created on: 2016. máj. 13.
 *      Author: Máté
 */

#ifndef COMPUTERVISION_H_
#define COMPUTERVISION_H_


#include <opencv2/core.hpp>
#include <map>
#include "Model.h"
#include "IImageProcessor.h"
#include "IRImageProcessor.h"
#include "stereoCamera.h"
#include <Publisher.h>

#define COMPUTERVISION_ID		"computervision"
#define EXPOSURE				"exposure"
#define GAIN					"gain"
#define CAMERATYPE				"cameratype"
#define TYPEOFPROCESSING		"typeofprocessing"

enum CameraType{
	XIMEA
};

enum ImageProcessingType{
	COLOR,
	IR
};

class ComputerVision {
private:
	Model model;
	StereoCamera camera;
	std::unique_ptr<IImageProcessor> leftImageProcessor;
	std::unique_ptr<IImageProcessor> rightImageProcessor;
	MQTTPublisher *publisher;
	Frame frame;
	Frame prevFrame;
	Frame drawing;
	bool initialized;
public:
	ComputerVision();
	bool initialize(std::string configFilePath);
	void reConfigure(std::string configFilePath);
	void setupDataSender(std::string brokerURL);
	void captureFrame();
	Frame getCurrentFrame();
	void processCurrentFrame();
	void sendData(std::string topic);
	void sendData(std::string topic , std::string objectId);
	void sendData(std::string topic , std::string objectId , std::string markerId);
	cv::Point3f getMarkerPosition(std::string objectId , std::string markerId);
	std::map<std::string , cv::Point3f> getObjectPosition(std::string objectId);
	std::vector<std::string> getObjectIds();
	std::vector<std::string> getMarkerIds(std::string objectId);
	void showImage();
	void showGrid(bool show);
	bool isTracked(std::string objectId);
	virtual ~ComputerVision();
};

#endif /* COMPUTERVISION_H_ */
