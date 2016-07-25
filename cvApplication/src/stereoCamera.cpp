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
#include <iostream>

StereoCamera::StereoCamera(){
	initialized = false;
}

//initializes the two cameras
bool StereoCamera::init(int exposure , int gain , int camera , std::string matrices){
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

		cv::FileStorage fs;
		if(fs.open(matrices, cv::FileStorage::READ)){
			fs["left_camMatrix"] >> cameraMatrices.leftCamMatrix;
			fs["right_camMatrix"] >> cameraMatrices.rightCamMatrix;
			fs["p1"] >> cameraMatrices.p1;
			fs["p2"] >> cameraMatrices.p2;
			fs["r1"] >> cameraMatrices.r1;
			fs["r2"] >> cameraMatrices.r2;
			fs["left_distCoeffs"] >> cameraMatrices.leftDistCoeffs;
			fs["right_distCoeffs"] >> cameraMatrices.rightDistCoeffs;
			fs.release();
		}else{
			std::cout<<"Could not read camera matrices!"<<std::endl;
			success = false;
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

CameraMatrices StereoCamera::getCameraMatrices(){
	return cameraMatrices;
}

