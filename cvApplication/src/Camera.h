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

struct Frame{
	tbb::concurrent_vector<cv::Mat> images;
	int64_t timestamp;
	int64_t frameIndex;
};

class Camera: public tbb::flow::source_node< Frame > {
	int numberOfCameras;
	int exposure;
	int gain;
	int fps;
	int frameCounter ;

	std::atomic_bool recording;
	std::atomic_bool initialized;
	int64_t lastTimestamp;

	std::vector<cv::VideoCapture> cameras;

public:

	Camera(int fps , int exposure, int gain, int numberOfCameras, tbb::flow::graph& g) :
			tbb::flow::source_node< Frame >(g,
					std::bind(&Camera::capture, this, std::placeholders::_1),
					false), numberOfCameras(numberOfCameras), exposure(
					exposure), gain(gain),fps(fps),frameCounter(0), recording(false),initialized(
					false),lastTimestamp(0){};

	bool capture(Frame &images);
	bool init(int cameraType);
	void stopRecording();
	void startRecording();

	virtual ~Camera();
};

#endif /* CAMERA_H_ */
