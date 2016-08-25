/*
 * Camera.h
 *
 *  Created on: 2016. aug. 9.
 *      Author: Máté
 */

#ifndef CAMERA_H_
#define CAMERA_H_

#include <opencv2/videoio.hpp>
#include <opencv2/opencv.hpp>
#include <tbb/flow_graph.h>
#include <tbb/concurrent_vector.h>
#include <chrono>
#include <atomic>
#include "tbb/tbb.h"
#include <windows.h>
#include <ctime>
#include <time.h>
#include "Provider.h"

struct Matrices{
	tbb::concurrent_vector<cv::Mat> cameraMatrix;
	tbb::concurrent_vector<cv::Mat> distCoeffs;
	tbb::concurrent_vector<cv::Mat> projectionMatrix;
	tbb::concurrent_vector<cv::Mat> rotationMatrix;
};

struct Frame{
	tbb::concurrent_vector<cv::Mat> images;
	int64_t timestamp;
	int64_t frameIndex;
	int fps;
};

class Camera:public Provider< Frame > {
	int numberOfCameras;
	int exposure;
	int gain;
	int fps;
	int frameCounter ;

	bool canTransform;
	std::atomic_bool initialized;
	int64_t lastTimestamp;

	Matrices matrices;
	std::vector<cv::VideoCapture> cameras;

public:

	Camera(int fps , int exposure , int gain, int numberOfCameras, tbb::flow::graph& g) :
			Provider< Frame >(g), numberOfCameras(numberOfCameras), exposure(
					exposure), gain(gain),fps(fps),frameCounter(0),canTransform(false),initialized(
					false),lastTimestamp(0){};

	bool provide(Frame &images);
	bool init(int cameraType);
	bool loadMatrices(std::string path);
	cv::Point3f getRealPosition(tbb::concurrent_vector<cv::Point2f> screenPosition);

	virtual ~Camera();
};

#endif /* CAMERA_H_ */
