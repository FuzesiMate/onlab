/*
 * Model.cpp
 *
 *  Created on: 2016. máj. 13.
 *      Author: Máté
 */

#include "Model.h"
#include <iostream>
#include <boost/assign/list_of.hpp>
#include <map>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include "ComputerVision.h"


std::map<std::string,ReferencePosition> mapToReferencePosition = boost::assign::map_list_of("UPPER",UPPER)("LOWER",LOWER)("IN_ROW",IN_ROW);


Model::Model() {
	showGrid = false;
}

bool Model::buildModel(boost::property_tree::ptree propertyTree){

	std::vector<std::shared_ptr<Object> > listOfObjects;

	auto imageProcessingType = propertyTree.get<std::string>("typeofprocessing");

	auto InputObjects = propertyTree.get_child("objects");

	for (auto &object : InputObjects){
			auto nameofObject = object.second.get<std::string>("name");
			auto numberofParts  = object.second.get<int>("numberofparts");

			std::shared_ptr<Object> compObject;

			if(imageProcessingType=="IR"){
				compObject= std::shared_ptr<Object>(new IRObject());
			}else if(imageProcessingType=="ARUCO_MARKER"){
				compObject= std::shared_ptr<Object>(new ArucoObject());
			}

			compObject->initializeObject(numberofParts,nameofObject);

			auto markers = object.second.get_child("markers");

			for (auto &marker : markers){
				if(imageProcessingType=="IR"){
					compObject->addPart(marker.second.get<std::string>("name") , marker.second.get<float>("distancefromreference"),
							mapToReferencePosition[marker.second.get<std::string>("orientationfromreference")],
							mapToReferencePosition[marker.second.get<std::string>("orientationfromprevious")]);
				}else if(imageProcessingType =="ARUCO_MARKER"){
					compObject->addPart(marker.second.get<std::string>("name") , marker.second.get<int>("id"));
				}
			}

			listOfObjects.push_back(compObject);
		}

	//decreasing size of objects
	//important for detection

	std::sort(listOfObjects.begin(), listOfObjects.end() ,
				[](const shared_ptr<Object> &a , const shared_ptr<Object> &b)-> bool{
				int ai= a->getNumberofParts();
				int bi = b->getNumberofParts();
				return ai>bi;
				});


	//fill the map and store the order
	//maybe the map is overkill, just makes the query easier
	for(auto &o : listOfObjects){
		objects[o->getId()] = o;
		objectIds.push_back(o->getId());
	}

	return true ;
}

void Model::updateModel(std::pair<std::vector< std::vector<cv::Point> > ,std::vector< std::vector<cv::Point> > > contourSet,std::pair<std::vector<int> , std::vector<int> > identifiers, Frame frame , Frame prevFrame ){

	PointSet points;

	cout<<"find circles"<<endl;
	for(auto contour : contourSet.first){
		cv::Point2f center;
		float r;
		cv::minEnclosingCircle(contour , center ,r);
		points.left.push_back(center);
	}

	for(auto contour : contourSet.second){
		cv::Point2f center;
		float r;
		cv::minEnclosingCircle(contour , center ,r);
		points.right.push_back(center);
	}

	cout<<"find circles end"<<endl;

	cout<<"detect methods"<<endl;
	for(std::string &objectId : objectIds){

		if(!objects[objectId]->isTracked()){
			objects[objectId]->detect(points ,contourSet , identifiers);
		}else{
			//objects[objectId]->track(frame , prevFrame);
			// no tracking currently
			objects[objectId]->detect(points ,contourSet , identifiers);
		}

	}

	cout<<"detect methods end"<<endl;
}

std::vector<std::string> Model::getObjectNames(){
	return objectIds;
}

std::vector<std::string> Model::getMarkerNames(std::string objectId){
	return objects[objectId]->getMarkerNames();
}

cv::Point3f Model::getPosition(std::string objectId  , std::string markerId){
	return objects[objectId]->getMarkerPosition(markerId);
}

void Model::draw(Frame frame){

	if(showGrid){
		for(int i = 0 ; i<1280 ; i+=20){
			line(frame.left, cv::Point(i,0) , cv::Point(i,1024), cv::Scalar(255,255,255));
		}

		for(int i = 0 ; i<1024 ; i+=20){
			line(frame.left, cv::Point(0,i) , cv::Point(1280,i), cv::Scalar(255,255,255));
		}
	}

	for(std::string &o : objectIds){
		objects[o]->draw(frame);
	}
}

bool Model::isTracked(std::string objectId){
	return objects[objectId]->isTracked();
}

void Model::setShowGrid(bool show){
	showGrid = show;
}


Model::~Model() {}

