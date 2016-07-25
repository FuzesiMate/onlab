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

struct CameraMatrices{
	cv::Mat leftCamMatrix;
	cv::Mat rightCamMatrix;
	cv::Mat leftDistCoeffs;
	cv::Mat rightDistCoeffs;
	cv::Mat p1;
	cv::Mat p2;
	cv::Mat r1;
	cv::Mat r2;
};

class StereoCamera {
private:
	volatile bool initialized;
	cv::VideoCapture leftCamera;
	cv::VideoCapture rightCamera;
	std::vector<cv::VideoCapture> cameras;
	CameraMatrices cameraMatrices;

public:
	StereoCamera();
	bool init(int exposure , int gain , int camera , std::string matrices);
	Frame getNextFrame();
	void startRecording(int fps);
	void runRecording();
	CameraMatrices getCameraMatrices();
	virtual ~StereoCamera();
};

#endif /* STEREOCAMERA_H_ */
