/*
 * ComputerVision.cpp
 *
 *  Created on: 2016. máj. 13.
 *      Author: Máté
 */

#include "ComputerVision.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/assign/list_of.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

std::map<std::string,ImageProcessingType> mapToImgProcType = boost::assign::map_list_of("COLOR" ,COLOR)("IR",IR);
std::map<std::string,CameraType> mapToCamType = boost::assign::map_list_of("XIMEA" , XIMEA);

ComputerVision::ComputerVision() {
	initialized = false;
}

bool ComputerVision::initialize(std::string configFilePath){
	bool success = false;

	boost::property_tree::ptree pt;
	boost::property_tree::read_json(configFilePath, pt);

	int exposure = pt.get<int>(EXPOSURE);
	float gain = pt.get<float>(GAIN);

	ImageProcessingType imgProcType = mapToImgProcType[pt.get<std::string>(TYPEOFPROCESSING)];
	CameraType camType = mapToCamType[pt.get<std::string>(CAMERATYPE)];

	success = camera.init(exposure , gain, CV_CAP_XIAPI);

	switch(imgProcType){
	case IR:
		leftImageProcessor = std::unique_ptr<IImageProcessor>(new IRImageProcessor);
		rightImageProcessor = std::unique_ptr<IImageProcessor>(new IRImageProcessor);
		break;
	case COLOR:

		break;
	default:
		cout<<"unknown processing type"<<endl;
		return false;
		break;
	}

	leftImageProcessor->setWindow("leftProcessed");
	leftImageProcessor->setFilterValues(pt);

	rightImageProcessor->setWindow("rightProcessed");

	cout<<"set filter values"<<endl;
	rightImageProcessor->setFilterValues(pt);
	cout<<"build model"<<endl;
	success = success && model.buildModel(pt);

	if(success){
		initialized = true;
	}

	return success;
}

void ComputerVision::reConfigure(std::string configFilePath){
	boost::property_tree::ptree pt;
	boost::property_tree::read_json(configFilePath, pt);
	leftImageProcessor->setFilterValues(pt);
	rightImageProcessor->setFilterValues(pt);
}

void ComputerVision::setupDataSender(std::string brokerURL){
	publisher = new MQTTPublisher(brokerURL.c_str() , COMPUTERVISION_ID);
	publisher->connect(nullptr , nullptr , nullptr);
}

void ComputerVision::captureFrame(){

	if(initialized){
		prevFrame.left = frame.left.clone();
		prevFrame.right = frame.right.clone();
		frame = camera.getNextFrame();
	}
 }

Frame ComputerVision::getCurrentFrame(){
	return frame;
}

void ComputerVision::processCurrentFrame(){
	if(initialized){

		std::pair<std::vector< std::vector<cv::Point> > ,std::vector< std::vector<cv::Point> > > contourSet;
		PointSet points;

		bool allTracked = true;

		for(string &oid : getObjectIds()){
			if(!model.isTracked(oid)){
				allTracked = false;
			}
		}

		if(!allTracked){
			contourSet.first = leftImageProcessor->processImage(frame.left);
			contourSet.second = rightImageProcessor->processImage(frame.right);

			cv::Point2f center;
			float radius;

			for(auto i = 0 ; i<contourSet.first.size() ; i++){

				if(cv::contourArea(contourSet.first[i])<500){
					cv::minEnclosingCircle(contourSet.first[i] , center , radius);
					if(radius>1 && radius<20){
						points.left.push_back(center);
					}
				}
			}

			for(auto i = 0 ; i<contourSet.second.size() ; i++){

				if(cv::contourArea(contourSet.second[i])<500){
					cv::minEnclosingCircle(contourSet.second[i] , center , radius);
						if(radius>1 && radius<20){
							points.right.push_back(center);
						}
				}
			}
		}

		model.updateModel(points, contourSet , frame , prevFrame);

		drawing.left = frame.left.clone();
		drawing.right = frame.right.clone();
		model.draw(drawing);
	}
}

std::map<std::string , cv::Point3f> ComputerVision::getObjectPosition(std::string objectId){
	std::map<std::string , cv::Point3f> position;

	if(!model.isTracked(objectId)){
		return position;
	}

	auto markerIds = model.getMarkerIds(objectId);

	for(std::string &markerId : markerIds){
		position[markerId] = model.getPosition(objectId , markerId);
	}

	return position;
}

cv::Point3f ComputerVision::getMarkerPosition(std::string objectId , std::string markerId){
	if(model.isTracked(objectId)){
		return model.getPosition(objectId , markerId);
	}
	else return cv::Point3f(0,0,0);
}

void ComputerVision::showImage(){
	if(initialized){
		cv::Mat f1,f2;
		resize(drawing.left,f1,cv::Size(1280/2,1024/2));
		resize(drawing.right,f2,cv::Size(1280/2,1024/2));
		cv::imshow("left" , f1);
		cv::imshow("right" , f2);
		cv::waitKey(5);
	}
}

bool ComputerVision::isTracked(std::string objectId){
	return model.isTracked(objectId);
}

std::vector<std::string>ComputerVision::getObjectIds(){
	return model.getObjectIds();
}

std::vector<std::string> ComputerVision::getMarkerIds(std::string objectId){
	return model.getMarkerIds(objectId);
}

void ComputerVision::sendData(std::string topic){
	std::stringstream message;

	message<<"{ \"objects\": [";

	auto objectIds = model.getObjectIds();

	for(auto o = 0 ; o<objectIds.size() ; o++){

		message<<"{ \"name\":"<<"\""<<objectIds[o]<<"\""<<",";
		message<<"\"markers\":[";

		auto markerIds = model.getMarkerIds(objectIds[o]);

		auto position = this->getObjectPosition(objectIds[o]);

		for(auto i = 0 ; i<markerIds.size() ; i++){
			auto markerpos = position[markerIds[i]];
			message<<"{";
			message<<"\"id\":"<<"\""<<markerIds[i]<<"\""<<",";
			message<<"\"x\":"<<markerpos.x<<",";
			message<<"\"y\":"<<markerpos.y<<",";
			message<<"\"z\":"<<markerpos.z;
			message<<"}";
			if(i<markerIds.size()-1){
				message<<",";
			}
		}
		message<<"]";
		message<<"}";
		if(o<objectIds.size()-1){
			message<<",";
		}

	}

	message<<"]";
	message<<"}";

	publisher->publishMessage(topic , 0 , message.str().c_str());
}

void ComputerVision::sendData(std::string topic , std::string objectId){
	std::stringstream message;
	message<<"{";
	message<<"\""<<objectId<<"\":";
	message<<"[";

	auto position = this->getObjectPosition(objectId);
	auto markerIds = model.getMarkerIds(objectId);

	for(auto i = 0 ; i<markerIds.size() ; i++){
		auto markerpos = model.getPosition(objectId , markerIds[i]);
		message<<"{";
		message<<"\"id\":";
		message<<"\""<<markerIds[i]<<"\",";
		message<<"\"x\":"<<markerpos.x<<",";
		message<<"\"y\":"<<markerpos.y<<",";
		message<<"\"z\":"<<markerpos.z;
		message<<"}";
		if(i<markerIds.size()-1){
			message<<",";
		}
	}

	message<<"]";
	message<<"}";

	publisher->publishMessage(topic , 0 , message.str().c_str());
}

void ComputerVision::sendData(std::string topic , std::string objectId , std::string markerId){

}

void ComputerVision::showGrid(bool show){
	model.setShowGrid(show);
}

ComputerVision::~ComputerVision() {
	delete publisher;
}

