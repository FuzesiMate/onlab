/*
 * cameraHandler.cpp
 *
 *  Created on: 2016. ápr. 5.
 *      Author: Máté
 */

#include "stereoCamera.h"

#include <opencv2/core.hpp>
#include<opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/video.hpp>


StereoCamera::StereoCamera(){
	initialized = false;
}

//initializes the two cameras
bool StereoCamera::init(int exposure , int gain , int camera){
	bool success = false;

	if(!initialized){
		success = leftCamera.open(camera+0);
		success = success && rightCamera.open(camera+1);
		cameras.push_back(leftCamera);
		cameras.push_back(rightCamera);

		//disable auto exposure and gain
		leftCamera.set(CV_CAP_PROP_XI_AEAG  , 0.0);
		rightCamera.set(CV_CAP_PROP_XI_AEAG , 0.0);

		//set default exposure manually
		rightCamera.set(CV_CAP_PROP_EXPOSURE, exposure);
		leftCamera.set(CV_CAP_PROP_EXPOSURE , exposure);

		//set default gain manually
		leftCamera.set(CV_CAP_PROP_GAIN , gain);
		rightCamera.set(CV_CAP_PROP_GAIN , gain);

		if(success){
			initialized = true;
		}
	}
	return success;
}

StereoCamera::~StereoCamera() {
	for(auto i = 0 ; i<cameras.size() ; i++){
		cameras[i].release();
	}
}

void StereoCamera::startRecording(int fps){
	leftCamera.set(CV_CAP_PROP_FPS , fps);
	rightCamera.set(CV_CAP_PROP_FPS , fps);
}

Frame StereoCamera::getNextFrame(){
	for(auto i = 0 ; i<cameras.size() ; i++){
		cameras[i].grab();
	}
	cv::Mat left;
	cv::Mat right;
	leftCamera.retrieve(left);
	rightCamera.retrieve(right);

	Frame fr;
	fr.left = left;
	fr.right = right;

	return fr;
}

