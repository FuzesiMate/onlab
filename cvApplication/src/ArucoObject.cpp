/*
 * ArucoObject.cpp
 *
 *  Created on: 2016. jún. 30.
 *      Author: Máté
 */

#include "ArucoObject.h"
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>

using namespace std;

ArucoObject::ArucoObject() {
	firstTrack = false;
}

std::pair<int,int> ArucoObject::findMatch(std::string markerId , std::pair<std::vector<int> , std::vector<int> > identifiers){
	int leftIdentifier= -1;
	int rightIdentifier= -1;

	for(size_t i = 0 ; i<identifiers.first.size() ; i++){
		if(identifiers.first[i] == markers[markerId].getId()){
			leftIdentifier = i;
		}
	}

	for(size_t i = 0 ; i<identifiers.second.size() ; i++){
		if(identifiers.second[i] == markers[markerId].getId()){
			rightIdentifier = i;
		}
	}

	return std::make_pair(leftIdentifier , rightIdentifier);
}


std::pair<std::vector<int> ,std::vector<int>> ArucoObject::detect(PointSet points,std::pair<std::vector< std::vector<cv::Point> > ,std::vector< std::vector<cv::Point> > > contourSet , std::pair<std::vector<int> , std::vector<int> > identifiers){

	int foundMarkers=0;
	markerROI.first.clear();
	markerROI.second.clear();

	for (auto m : markerIds) {
		auto indices = findMatch(m, identifiers);

		if (indices.first >= 0 && indices.second >= 0) {
			StereoPoint pos;
			pos.left = points.left[indices.first];
			pos.right = points.right[indices.second];
			markers[m].setPosition(pos);

			PointSet posSet;

			auto rect = cv::boundingRect(contourSet.first[indices.first]);
			markerROI.first.push_back(rect);


			rect = cv::boundingRect(contourSet.second[indices.second]);
			markerROI.second.push_back(rect);

			for(size_t i = 0 ; i<contourSet.first[indices.first].size() ; i++){
				posSet.left.push_back(contourSet.first[indices.first][i]);
			}

			for(size_t i = 0 ; i<contourSet.second[indices.second].size() ; i++){
				posSet.right.push_back(contourSet.second[indices.second][i]);
			}

			markers[m].setPosition(posSet);

			foundMarkers++;
		} else {
			markers[m].setLost(true);
			tracked = false;
		}
	}

	if(foundMarkers == markerIds.size()){
		tracked = true;
		firstTrack = true;
	}

	std::pair<std::vector<int>,std::vector<int> > ret ;
	return ret;
}

void ArucoObject::track(Frame frame , Frame prevFrame){

	int i = 0 ;

	for(auto m : markerIds){
		if(markers[m].isLost()){
			tracked = false;
			return;
		}

		//if the object was just detected we should find the features of the marker
		//and set the position of the markers

		if (firstTrack) {
			cv::Mat ROI = frame.left(markerROI.first[i]);
			cv::cvtColor(ROI, ROI, CV_RGB2GRAY);

			std::vector<cv::Point2f> features;
			cv::goodFeaturesToTrack(ROI, features, 10, 0.1, 1);

			PointSet pos;

			for (auto p : features) {

				p.x+=markerROI.first[i].x;
				p.y+=markerROI.first[i].y;

				pos.left.push_back(p);
			}

			ROI = frame.right(markerROI.second[i]);
			cv::cvtColor(ROI, ROI, CV_RGB2GRAY);

			cv::goodFeaturesToTrack(ROI, features, 15, 0.01, 2);

			for (auto p : features) {

				p.x+=markerROI.second[i].x;
				p.y+=markerROI.second[i].y;

				pos.right.push_back(p);
			}

			markers[m].setPosition(pos);

		}
		i++;
		markers[m].refreshPosition(frame , prevFrame);
	}

	firstTrack = false;

}

ArucoObject::~ArucoObject() {
	// TODO Auto-generated destructor stub
}

