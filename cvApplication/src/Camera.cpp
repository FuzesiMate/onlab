/*
 * Camera.cpp
 *
 *  Created on: 2016. aug. 9.
 *      Author: Máté
 */

#include "Camera.h"

bool Camera::capture(Frame &frame) {

	if (!recording) {
		return false;
	}

	auto time = std::chrono::steady_clock::now();
	auto currentTimestamp =
			std::chrono::duration_cast<std::chrono::milliseconds>(
					time.time_since_epoch()).count();

	auto delay = (1000/fps) - (currentTimestamp - lastTimestamp);

	if (delay > 0 && lastTimestamp!=0) {
		Sleep(delay);
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

	std::stringstream fpsstring;
	fpsstring <<1000/(currentTimestamp-lastTimestamp);

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

bool Camera::loadMatrices(std::string path){
	cv::FileStorage file;
	if(!file.open(path , cv::FileStorage::READ)){
		return false;
	}

	cv::Mat temp;
	file["left_camMatrix"]>>temp;
	matrices.cameraMatrix.push_back(temp.clone());
	file["right_camMatrix"]>>temp;
	matrices.cameraMatrix.push_back(temp.clone());
	file["left_distCoeffs"]>>temp;
	matrices.distCoeffs.push_back(temp.clone());
	file["right_distCoeffs"]>>temp;
	matrices.distCoeffs.push_back(temp.clone());
	file["p1"]>>temp;
	matrices.projectionMatrix.push_back(temp.clone());
	file["p2"]>>temp;
	matrices.projectionMatrix.push_back(temp.clone());
	file["r1"]>>temp;
	matrices.rotationMatrix.push_back(temp.clone());
	file["r2"]>>temp;
	matrices.rotationMatrix.push_back(temp.clone());

	canTransform = true;
	return true;
}

cv::Point3f Camera::getRealPosition(tbb::concurrent_vector<cv::Point2f> screenPosition){
	std::vector<cv::Point2f> p1;
	std::vector<cv::Point2f> p2;

	float x = 0;
	float y = 0;
	float z = 0;

	if(screenPosition.size()==2 && canTransform){
		p1.push_back(screenPosition[0]);
		p2.push_back(screenPosition[1]);

		cv::undistortPoints(p1, p1 , matrices.cameraMatrix[0] , matrices.distCoeffs[0] , matrices.rotationMatrix[0] , matrices.projectionMatrix[0]);
		cv::undistortPoints(p1, p1 , matrices.cameraMatrix[1] , matrices.distCoeffs[1] , matrices.rotationMatrix[1] , matrices.projectionMatrix[1]);

		cv::Mat cord;
		cv::triangulatePoints(matrices.projectionMatrix[0] , matrices.projectionMatrix[1] , p1 , p2 , cord);

			for(int i = 0 ; i<cord.cols ; i++){

				float w = cord.at<float>(3,i);
					  x = cord.at<float>(0,i)/w;
					  y = cord.at<float>(1,i)/w;
					  z = cord.at<float>(2,i)/w;
			}
	}
	return cv::Point3f(x,y,z);
}

void Camera::stopRecording(){
	recording = false;
}

void Camera::startRecording(){
	recording = true;
	this->activate();
}


Camera::~Camera() {
	for(auto c : cameras){
		c.release();
	}
}

