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

cv::Point2f findCorner(std::vector<cv::Point2f> rectangle){
	float minx=rectangle[0].x;
	float miny=rectangle[0].y;


}

std::pair<std::vector<int> ,std::vector<int>> ArucoObject::detect(PointSet points,std::pair<std::vector< std::vector<cv::Point> > ,std::vector< std::vector<cv::Point> > > contourSet , std::pair<std::vector<int> , std::vector<int> > identifiers){

	int foundMarkers=0;

	for (auto m : markerIds) {
		auto indices = findMatch(m, identifiers);

		if (indices.first >= 0 && indices.second >= 0) {
			StereoPoint pos;
			pos.left = points.left[indices.first];
			pos.right = points.right[indices.second];
			markers[m].setPosition(pos);

			PointSet posSet;
			for(size_t i = 0 ; i<contourSet.first[indices.first].size() ; i++){
				/*
				float w = fabs(contourSet.first[indices.first][0].x-contourSet.first[indices.first][1].x);
				float h = fabs(contourSet.first[indices.first][0].y-contourSet.first[indices.first][3].y);
				markerROI.push_back(cv::Rect(contourSet.first[indices.first][2].x ,contourSet.first[indices.first][2].y,w,h));
				*/
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
	}

	std::pair<std::vector<int>,std::vector<int> > ret ;
	return ret;
}

void ArucoObject::track(Frame frame , Frame prevFrame){
	/*
	for(auto r : markerROI){
		cv::rectangle(frame.left , r , cv::Scalar(0,0,255) , 2.0);
	}
	*/
	for(auto m : markerIds){
		if(markers[m].isLost()){
			tracked = false;
		}
		markers[m].refreshPosition(frame , prevFrame);
	}
}

ArucoObject::~ArucoObject() {
	// TODO Auto-generated destructor stub
}

