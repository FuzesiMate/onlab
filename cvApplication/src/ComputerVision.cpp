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

std::map<std::string,ImageProcessingType> mapToImgProcType = boost::assign::map_list_of("COLOR" ,COLOR)("IR",IR)("ARUCO_MARKER",ARUCO_MARKER);
std::map<std::string,CameraType> mapToCamType = boost::assign::map_list_of("XIMEA" , XIMEA);

ComputerVision::ComputerVision() {
	initialized = false;
}

bool ComputerVision::initialize(std::string configFilePath){
	bool success = false;

	boost::property_tree::ptree pt;
	ImageProcessingType imgProcType;

	try {
		boost::property_tree::read_json(configFilePath, pt);

		int exposure = pt.get<int>(EXPOSURE);
		float gain = pt.get<float>(GAIN);

		imgProcType = mapToImgProcType[pt.get<std::string>(
				TYPEOFPROCESSING)];
		CameraType camType = mapToCamType[pt.get<std::string>(CAMERATYPE)];

		cout<<"Initializing cameras..."<<endl;

		success = camera.init(exposure, gain, CV_CAP_XIAPI , pt.get<std::string>("matrices"));

		if (!success) {
			cout << "Camera initialization failed!" << endl;
			return false;
		}

	} catch (exception &e) {
		cout << "The JSON file is not valid or missing! Error message: "
				<< e.what() << endl;
		return false;
	}

	switch(imgProcType){
	case IR:
		leftImageProcessor = std::unique_ptr<IImageProcessor>(new IRImageProcessor);
		rightImageProcessor = std::unique_ptr<IImageProcessor>(new IRImageProcessor);
		break;
	case COLOR:

		break;
	case ARUCO_MARKER:
		leftImageProcessor = std::unique_ptr<IImageProcessor>(new ArucoImageProcessor);
		rightImageProcessor = std::unique_ptr<IImageProcessor>(new ArucoImageProcessor);
		break;
	default:
		cout<<"unknown processing type"<<endl;
		return false;
		break;
	}

	std::vector<int> markerIds;

	try {
		auto objects = pt.get_child("objects");

		for (auto &object : objects) {
			auto markers = object.second.get_child("markers");

			for (auto &marker : markers) {
				int id = marker.second.get<int>("id");
				markerIds.push_back(id);
			}
		}

	}catch(exception &e){
		cout<<"Problem occured while reading JSON file! Error message: "<<e.what()<<endl;
		return false;
	}

	leftImageProcessor->setMarkerIdentifiers(markerIds);
	rightImageProcessor->setMarkerIdentifiers(markerIds);

	leftImageProcessor->setWindow("leftProcessed");
	leftImageProcessor->setFilterValues(pt);

	rightImageProcessor->setWindow("rightProcessed");
	rightImageProcessor->setFilterValues(pt);

	cout<<"Building model..."<<endl;
	success = success && model.buildModel(pt);

	model.setCamera(camera);

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


		bool allTracked = true;

		for(string &oid : getObjectNames()){
			if(!model.isTracked(oid)){
				allTracked = false;
			}
		}

		if(!allTracked){
			contourSet.first = leftImageProcessor->processImage(frame.left);
			contourSet.second = rightImageProcessor->processImage(frame.right);
		}

		std::pair<std::vector<int> , std::vector<int> > identifiers;
		identifiers.first = leftImageProcessor->getMarkerIdentifiers();
		identifiers.second = rightImageProcessor->getMarkerIdentifiers();

		model.updateModel(contourSet,identifiers, frame , prevFrame);

		//cv::cvtColor(frame.left ,drawing.left  , CV_GRAY2RGB);
		//cv::cvtColor(frame.right,drawing.right , CV_GRAY2RGB);
		drawing.left = frame.left.clone();
		drawing.right = frame.right.clone();

		model.draw(drawing);
	}
}

std::map<std::string , cv::Point3f> ComputerVision::getObjectPosition(std::string objectId){
	std::map<std::string , cv::Point3f> position;

	auto markerIds = model.getMarkerNames(objectId);

	for(std::string &markerId : markerIds){
		position[markerId] = model.getPosition(objectId , markerId);
	}

	return position;
}

cv::Point3f ComputerVision::getMarkerPosition(std::string objectId , std::string markerId){
		return model.getPosition(objectId , markerId);
}

void ComputerVision::showImage(){
	if(initialized){
		cv::Mat f1,f2;
		resize(drawing.left,f1,cv::Size(1280/2,1024/2));
		resize(drawing.right,f2,cv::Size(1280/2,1024/2));
		cv::imshow("left" , f1);
		cv::imshow("right" , f2);
		//cv::imshow("origi" , frame.left);
		cv::waitKey(5);
	}
}

bool ComputerVision::isTracked(std::string objectId){
	return model.isTracked(objectId);
}

std::vector<std::string>ComputerVision::getObjectNames(){
	return model.getObjectNames();
}

std::vector<std::string> ComputerVision::getMarkerNames(std::string objectId){
	return model.getMarkerNames(objectId);
}

void ComputerVision::sendData(std::string topic){
	std::stringstream message;

	message<<"{ \"objects\": [";

	auto objectIds = model.getObjectNames();

	for(auto o = 0 ; o<objectIds.size() ; o++){

		message<<"{ \"name\":"<<"\""<<objectIds[o]<<"\""<<",";
		message<<"\"markers\":[";

		auto markerNames = model.getMarkerNames(objectIds[o]);

		auto position = this->getObjectPosition(objectIds[o]);

		for(auto i = 0 ; i<markerNames.size() ; i++){

			auto markerpos = position[markerNames[i]];

			if(model.isTracked(objectIds[o] , markerNames[i])){

			}

			message<<"{";
			message<<"\"tracked\":";
			if(model.isTracked(objectIds[o] , markerNames[i])){
				message<<"\"true\",";
			}else{
				message<<"\"false\",";
			}

			message<<"\"id\":"<<"\""<<markerNames[i]<<"\""<<",";
			message<<"\"x\":"<<markerpos.x<<",";
			message<<"\"y\":"<<markerpos.y<<",";
			message<<"\"z\":"<<markerpos.z;
			message<<"}";
			if(i<markerNames.size()-1){
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

	cout<<">>>>>>>>"<<objectId<<" positions:"<<endl;

	auto position = this->getObjectPosition(objectId);
	auto markerIds = model.getMarkerNames(objectId);

	for(auto i = 0 ; i<markerIds.size() ; i++){

		auto markerpos = model.getPosition(objectId , markerIds[i]);
		cout<<markerIds[i]<<":"<<endl;
		cout<<markerpos<<endl;
		cout<<endl<<endl;

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

