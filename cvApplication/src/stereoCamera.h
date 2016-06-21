/*
 * cameraHandler.h
 *
 *  Created on: 2016. ápr. 5.
 *      Author: Máté
 */

#ifndef STEREOCAMERA_H_
#define STEREOCAMERA_H_

#include <opencv2/videoio.hpp>

struct Frame{
	cv::Mat left;
	cv::Mat right;
	int width;
	int height;
};

class StereoCamera {
private:
	volatile bool initialized;
	cv::VideoCapture leftCamera;
	cv::VideoCapture rightCamera;
	std::vector<cv::VideoCapture> cameras;
public:
	StereoCamera();
	bool init(int exposure , int gain , int camera);
	Frame getNextFrame();
	void startRecording(int fps);
	void runRecording();
	virtual ~StereoCamera();
};

#endif /* STEREOCAMERA_H_ */
