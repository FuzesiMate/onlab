/*
 *
 *  Created on: 2016. aug. 9.
 *      Author: M�t�
 */

#include "Camera.h"

bool Camera::provide(Frame &frame) {

	if (!providing) {
		std::cout<<"Stop recording"<<std::endl;
		return false;
	}

	auto time = std::chrono::steady_clock::now();
	auto currentTimestamp =
			std::chrono::duration_cast<std::chrono::milliseconds>(
					time.time_since_epoch()).count();

	auto delay = (1000/fps) - (currentTimestamp - lastTimestamp);

	if(delay<0){
		delay = 20;
	}

	if (delay > 0 && lastTimestamp!=0) {
		std::this_thread::sleep_for(std::chrono::milliseconds(delay));
	}

	time = std::chrono::steady_clock::now();
	currentTimestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
			time.time_since_epoch()).count();

	frame.images = tbb::concurrent_vector<cv::Mat>(numberOfCameras);

	tbb::parallel_for(size_t(0), cameras.size(), [&](size_t i ) {
		bool success = cameras[i].grab();
		if(!success){
			std::cout<<"grab failed"<<std::endl;
		}
	});

	tbb::parallel_for(size_t(0), cameras.size(), [&](size_t i ) {
		cameras[i].retrieve(frame.images[i]);
	});

	int currentFps;

	if((currentTimestamp-lastTimestamp)>0){
		currentFps = 1000/(currentTimestamp-lastTimestamp);
	}

	std::stringstream fpsstring;
	fpsstring <<currentFps;

	frame.fps = currentFps;


	for(auto& i : frame.images){

		cv::putText(i , fpsstring.str() , cv::Point(100,100) , cv::FONT_HERSHEY_SIMPLEX ,1.0 ,cv::Scalar(255,255,255) , 2.0);
	}


	frame.frameIndex = frameCounter;
	frame.timestamp = currentTimestamp;

	frameCounter++;
	lastTimestamp = currentTimestamp;

	return true;
}

bool Camera::init(int cameraType) {
	cameras = std::vector<cv::VideoCapture>(numberOfCameras);

	for(int i = 0 ; i<numberOfCameras ; i++){

		if(!cameras[i].open(cameraType+i)){
			return false;
		}
		cameras[i].set(cv::CAP_PROP_XI_AEAG , 0.0);
		cameras[i].set(cv::CAP_PROP_XI_EXPOSURE , exposure);
		cameras[i].set(cv::CAP_PROP_XI_GAIN , gain);
	}
	return true;
}

void Camera::setFPS(int fps){
	this->fps = fps;
}

void Camera::setExposure(int exposure){
	this->exposure = exposure;

	for(auto& camera : cameras){
		camera.set(cv::CAP_PROP_XI_EXPOSURE , exposure);
	}
}

void Camera::setGain(float gain){
	this->gain = gain;

	for(auto& camera : cameras){
		camera.set(cv::CAP_PROP_XI_GAIN , gain);
	}
}


Camera::~Camera() {
	for(auto c : cameras){
		c.release();
	}
}

