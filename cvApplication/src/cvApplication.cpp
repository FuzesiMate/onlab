//============================================================================
// Name        : cvApplication.cpp
// Author      : Fuzesi MAte
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <opencv2/core.hpp>
#include<opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/video.hpp>
#include <opencv2/videoio.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
#include <map>
#include "cameraHandler.h"
#include "ImageProcessor.h"
#include "Object.h"
#include "ComplexObject.h"
#include <boost/assign/list_of.hpp>

using namespace std;
using namespace cv;

std::map<string,PhysicalOrientation> mapToPhysicalOrientation = boost::assign::map_list_of("IN_HORIZONTAL_ROW" ,IN_HORIZONTAL_ROW)("IN_VERTICAL_ROW",IN_VERTICAL_ROW)
		("OTHER",OTHER);

std::map<string,ReferencePosition> mapToReferencePosition = boost::assign::map_list_of("UPPER",UPPER)("LOWER",LOWER)("IN_ROW",IN_ROW);

vector<ComplexObject> buildModel(boost::property_tree::ptree propertyTree){

	vector<ComplexObject> objects;

	for (boost::property_tree::ptree::value_type &object : propertyTree.get_child("objects")){
		auto nameofObject = object.second.get<string>("name");
		auto numberofParts  = object.second.get<int>("numberofparts");
		auto orientation = object.second.get<string>("orientation");

		ComplexObject compObject(numberofParts,mapToPhysicalOrientation[orientation],nameofObject);

		boost::property_tree::ptree parts = object.second.get_child("parts");

		for (boost::property_tree::ptree::value_type &part : parts){
			if(orientation=="OTHER"){

				compObject.addPart(part.second.get<string>("id") , part.second.get<float>("distancefromreference"),
						mapToReferencePosition[part.second.get<string>("orientationfromreference")],
						mapToReferencePosition[part.second.get<string>("orientationfromprevious")]);
			}

			//compObject.addPart(part.second.get<string>("id") , part.second.get<float>("distancefromreference"));

		}

		objects.push_back(compObject);
	}

	return objects;
}

int main(int argc , char **argv) {

	CameraHandler camHandler;
	camHandler.init(20000,1);
	ImageProcessor leftImageProcessor("leftprocessed");
	ImageProcessor rightImageProcessor("rightprocessed");

	boost::property_tree::ptree pt;
	boost::property_tree::read_json("input.json", pt);

	auto objectsToTrack = buildModel(pt);

	char esc = 'a';

	//VideoWriter left_writer;
	//VideoWriter right_writer;

	//left_writer.open("left_stream_stereo.avi" , -1 , 30  , Size(1280,1024) , false);
	//right_writer.open("right_stream_stereo.avi" , -1 , 30  , Size(1280,1024) , false);

	std::sort(objectsToTrack.begin(), objectsToTrack.end() ,
			[](const ComplexObject &a , const ComplexObject &b)-> bool{
			int ai= a.getNumberofParts();
			int bi = b.getNumberofParts();
			return ai>bi;
			});

	while(esc!='x'){

		std::pair<Mat,Mat> images = camHandler.getNextFrame();

		auto leftObjectPoints=leftImageProcessor.processImage(images.first);
		auto rightObjectPoints=rightImageProcessor.processImage(images.second);

		for(auto i = 0 ; i<objectsToTrack.size(); i++){

			auto pointIdxToDelete=objectsToTrack[i].detect(
					std::pair<vector<Point2f>,vector<Point2f> >(leftObjectPoints,rightObjectPoints));

			for(auto j = 0 ; j<pointIdxToDelete.first.size() ; j++){
				if(pointIdxToDelete.first[j]<leftObjectPoints.size()){
					leftObjectPoints.erase(leftObjectPoints.begin()+pointIdxToDelete.first[j]);
				}
			}

			for(auto k = 0 ; k<pointIdxToDelete.second.size() ; k++){
				if(pointIdxToDelete.second[k]<rightObjectPoints.size()){
					rightObjectPoints.erase(rightObjectPoints.begin()+pointIdxToDelete.second[k]);
				}
			}

			objectsToTrack[i].draw(images);
		}

		resize(images.first,images.first, Size(640,480));
		resize(images.second,images.second, Size(640,480));

		imshow("kep1orig" , images.first);
		imshow("kep2orig" , images.second);
		esc='a';
		esc=waitKey(5);
	}

	//left_writer.release();
	//right_writer.release();

	return 0;
}
