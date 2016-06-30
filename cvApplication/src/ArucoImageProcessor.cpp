/*
 * ArucoImageProcessor.cpp
 *
 *  Created on: 2016. jún. 30.
 *      Author: Máté
 */

#include "ArucoImageProcessor.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <opencv2/highgui.hpp>

ArucoImageProcessor::ArucoImageProcessor() {
	detectorParams =  cv::aruco::DetectorParameters::create();
	dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::PREDEFINED_DICTIONARY_NAME(0));
}

std::vector< std::vector<cv::Point> >ArucoImageProcessor::processImage(cv::Mat frame){

	std::vector< std::vector< cv::Point2f > > corners, rejected;

	foundMarkerIdentifiers.clear();

	cv::aruco::detectMarkers(frame, dictionary, corners, foundMarkerIdentifiers, detectorParams, rejected);

	std::vector< std::vector<cv::Point> > transformed(corners.size());

	for(auto i = 0 ; i<corners.size() ; i++){
		for(auto j = 0 ; j<corners[i].size() ; j++){
			transformed[i].push_back(cv::Point(corners[i][j].x , corners[i][j].y));
		}
	}

	return transformed;
}
	//set a window to show the processed image
void ArucoImageProcessor::setWindow(std::string winname){
	this->winname = winname;
	cv::namedWindow(winname , CV_WINDOW_AUTOSIZE);
}
	//set the processing specific filter values
void ArucoImageProcessor::setFilterValues(boost::property_tree::ptree propertyTree){

}
	//get the processing specific additional information to identify contours
std::vector<int> ArucoImageProcessor::getMarkerIdentifiers(){
	return foundMarkerIdentifiers;
}

ArucoImageProcessor::~ArucoImageProcessor(){}

