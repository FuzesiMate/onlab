/*
 * cameraHandler.cpp
 *
 *  Created on: 2016. ápr. 5.
 *      Author: Máté
 */

#include "cameraHandler.h"
#include <opencv2/core.hpp>
#include<opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/video.hpp>
#include <windows.h>

CameraHandler::CameraHandler(){
	initialized = false;
	recording = false;
}

//initializes the two cameras
bool CameraHandler::init(int exposure , int gain){
	bool success = false;

	if(!initialized){
		success = left_cam.open(CV_CAP_XIAPI+0);
		success = success && right_cam.open(CV_CAP_XIAPI+1);
		cameras.push_back(left_cam);
		cameras.push_back(right_cam);

		//disable auto exposure and gain
		left_cam.set(CV_CAP_PROP_XI_AEAG  , 0.0);
		right_cam.set(CV_CAP_PROP_XI_AEAG , 0.0);

		//set default exposure manually
		right_cam.set(CV_CAP_PROP_EXPOSURE, exposure);
		left_cam.set(CV_CAP_PROP_EXPOSURE , exposure);

		//set default gain manually
		left_cam.set(CV_CAP_PROP_GAIN , gain);
		right_cam.set(CV_CAP_PROP_GAIN , gain);

		if(success){
			initialized = true;
		}
	}
	return success;
}

CameraHandler::~CameraHandler() {
	for(auto i = 0 ; i<cameras.size() ; i++){
		cameras[i].release();
	}
}

void CameraHandler::startRecording(int fps){
	left_cam.set(CV_CAP_PROP_FPS , fps);
	right_cam.set(CV_CAP_PROP_FPS , fps);
}

std::pair<cv::Mat,cv::Mat> CameraHandler::getNextFrame(){
	for(auto i = 0 ; i<cameras.size() ; i++){
		cameras[i].grab();
	}
	cv::Mat left;
	cv::Mat right;
	left_cam.retrieve(left);
	right_cam.retrieve(right);

	return std::pair<cv::Mat,cv::Mat>(left,right);
}

