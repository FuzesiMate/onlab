/*
 * Camera.h
 *
 *  Created on: 2016. aug. 9.
 *      Author: M�t�
 */

#ifndef CAMERA_H_
#define CAMERA_H_

#include <opencv2/videoio.hpp>
#include "ZeroMQDataSender.h"
#include "LedController.h"
#include <tbb/flow_graph.h>
#include <tbb/concurrent_vector.h>
#include <chrono>
#include <atomic>
#include <tbb/tbb.h>
#include <ctime>
#include <time.h>
#include <thread>
#include "FrameProvider.h"
#include "DataTypes.h"

class Camera: public FrameProvider {
	int numberOfCameras;
	int exposure;
	int gain;
	int fps;
	int frameCounter ;

	int64_t lastTimestamp;

	std::vector<cv::VideoCapture> cameras;

public:

	Camera(int fps , int exposure , int gain, int numberOfCameras, tbb::flow::graph& g) :
			FrameProvider(g), numberOfCameras(numberOfCameras), exposure(
					exposure), gain(gain),fps(fps),frameCounter(0),lastTimestamp(0){};

	bool provide(Frame &images);
	bool init(int cameraType);
	void setFPS(int fps);
	void setExposure(int exposure);
	void setGain(float gain);

	virtual ~Camera();
};

#endif /* CAMERA_H_ */
