/*
 * cameraHandler.h
 *
 *  Created on: 2016. �pr. 5.
 *      Author: M�t�
 */

#ifndef CAMERAHANDLER_H_
#define CAMERAHANDLER_H_

#include <opencv2/videoio.hpp>
//#include "BlockingQueue.h"
#include <windows.h>

class CameraHandler {
private:
	volatile bool recording;
	volatile bool initialized;
	cv::VideoCapture left_cam;
	cv::VideoCapture right_cam;
	std::vector<cv::VideoCapture> cameras;
	//BlockingQueue <std::pair<cv::Mat,cv::Mat>> frames;
public:
	CameraHandler();
	bool init(int exposure , int gain);
	std::pair<cv::Mat,cv::Mat> getNextFrame();
	void startRecording(int fps);
	void runRecording();
	virtual ~CameraHandler();
};

#endif /* CAMERAHANDLER_H_ */
