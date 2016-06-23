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

std::map<std::string,PhysicalOrientation> mapToPhysicalOrientation = boost::assign::map_list_of("IN_HORIZONTAL_ROW" ,IN_HORIZONTAL_ROW)("IN_VERTICAL_ROW",IN_VERTICAL_ROW)("OTHER",OTHER);

std::map<std::string,ReferencePosition> mapToReferencePosition = boost::assign::map_list_of("UPPER",UPPER)("LOWER",LOWER)("IN_ROW",IN_ROW);


Model::Model() {
	showGrid = false;
}

bool Model::buildModel(boost::property_tree::ptree propertyTree){

	std::vector<Object> listOfObjects;

	for (boost::property_tree::ptree::value_type &object : propertyTree.get_child("objects")){
			auto nameofObject = object.second.get<std::string>("name");
			auto numberofParts  = object.second.get<int>("numberofparts");
			auto orientation = object.second.get<std::string>("orientation");

			Object compObject;
			compObject.initializeObject(numberofParts,mapToPhysicalOrientation[orientation],nameofObject);

			boost::property_tree::ptree parts = object.second.get_child("parts");

			for (boost::property_tree::ptree::value_type &part : parts){
				if(orientation=="OTHER"){

					compObject.addPart(part.second.get<std::string>("id") , part.second.get<float>("distancefromreference"),
							mapToReferencePosition[part.second.get<std::string>("orientationfromreference")],
							mapToReferencePosition[part.second.get<std::string>("orientationfromprevious")]);
				}
			}

			listOfObjects.push_back(compObject);
		}

	//decreasing size of objects
	//important for detection

	std::sort(listOfObjects.begin(), listOfObjects.end() ,
				[](const Object &a , const Object &b)-> bool{
				int ai= a.getNumberofParts();
				int bi = b.getNumberofParts();
				return ai>bi;
				});


	//fill the map and store the order
	//maybe the map is overkill, just makes the query easier
	for(Object &o : listOfObjects){
		objects[o.getId()] = o;
		listOfObjectIds.push_back(o.getId());
	}

	return true ;
}

void Model::updateModel(PointSet points,std::pair<std::vector< std::vector<cv::Point> > ,std::vector< std::vector<cv::Point> > > contourSet , Frame frame , Frame prevFrame){

	for(std::string &objectId : listOfObjectIds){

		auto pointIdxToDelete = objects[objectId].detect(points ,contourSet, frame , prevFrame);

		for(auto j = 0 ; j<pointIdxToDelete.first.size() ; j++){
			if(pointIdxToDelete.first[j]<points.left.size()){
				points.left.erase(points.left.begin()+pointIdxToDelete.first[j]);
			}
		}

		for(auto j = 0 ; j<pointIdxToDelete.second.size() ; j++){
			if(pointIdxToDelete.second[j]<points.right.size()){
				points.right.erase(points.right.begin()+pointIdxToDelete.second[j]);
			}
		}
	}
}

std::vector<std::string> Model::getObjectIds(){
	return listOfObjectIds;
}

std::vector<std::string> Model::getMarkerIds(std::string objectId){
	return objects[objectId].getMarkerIds();
}

cv::Point3f Model::getPosition(std::string objectId  , std::string markerId){
	return objects[objectId].getMarkerPosition(markerId);
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

	for(std::string &o : listOfObjectIds){
		objects[o].draw(frame);
	}

	imshow("wfwf" , frame.left);
	imshow("wfwfdqd" , frame.right);
}

bool Model::isTracked(std::string objectId){
	return objects[objectId].isTracked();
}

void Model::setShowGrid(bool show){
	showGrid = show;
}


Model::~Model() {
	// TODO Auto-generated destructor stub
}

