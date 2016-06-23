/*
 * ComplexObject.cpp
 *
 *  Created on: 2016. ápr. 14.
 *      Author: Máté
 */

#include "Object.h"
#include "Marker.h"

#include <math.h>
#include <iostream>
#include <algorithm>
#include <string>
#include <opencv2/highgui.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>


using namespace std;
using namespace cv;
Object::Object(){
	tracked = false;
}

void Object::initializeObject(int numberOfParts,PhysicalOrientation physicalOrientation, string id){
	orientation = physicalOrientation;
	this->numberOfParts = numberOfParts;
	tracked = false;
	this->id = id;

	FileStorage fs;
	fs.open("matrices.yml", FileStorage::READ);
	fs["left_camMatrix"] >> leftCamMatrix;
	fs["right_camMatrix"] >> rightCamMatrix;
	fs["p1"] >> p1;
	fs["p2"] >> p2;
	fs["r1"] >> r1;
	fs["r2"] >> r2;
	fs["left_distCoeffs"] >> leftDistCoeffs;
	fs["right_distCoeffs"] >> rightDistCoeffs;
	fs.release();
}

string toString(int i) {
	stringstream s;
	s << i;
	return s.str();
}

void Object::addPart(std::string id, float distanceFromRef,
		ReferencePosition fromRef, ReferencePosition fromPrev) {
	Marker o(id, distanceFromRef, fromRef, fromPrev);
	markerIds.push_back(id);
	markers[id] = o;
}

pair<bool, vector<Point2f> > Object::findMatch(
		vector<cv::Point2f> points) {

	vector<Point2f> matchPoints;
	vector<Point2f> actualPoints;

	switch (orientation) {
	case IN_HORIZONTAL_ROW:
		break;
	case IN_VERTICAL_ROW:
		break;
	case OTHER:
		std::sort(points.begin(), points.end(),
				[](const cv::Point2f &a , const cv::Point2f &b)-> bool {return a.x<b.x;});

		for (auto i = 0; i < points.size() - 1; i++) {
			if (fabs(points[i].x - points[i + 1].x) < 10) {
				if (points[i].y < points[i + 1].y) {
					auto temp = points[i];
					points[i] = points[i + 1];
					points[i + 1] = temp;
				}
			}
		}

		for (auto i = 0; i < points.size(); i++) {
			Point2f reference = points[i];
			actualPoints.clear();

			int matchNo = 1;

			Point2f lastMatchPoint = points[i];

			for (auto j = i + 1; j < points.size(); j++) {

				bool match = false;
				auto refPos = markers[markerIds[matchNo]].getReferences();

				switch (refPos.first) {
				case UPPER:
					match = (reference.y - points[j].y) > 15;
					break;
				case LOWER:
					match = (points[j].y - reference.y) > 15;
					break;
				case IN_ROW:
					match = fabs(points[j].y - reference.y) < 15;
					break;
				}
				switch (refPos.second) {
				case UPPER:
					match = match && (points[j - 1].y - points[j].y) > 15;
					break;
				case LOWER:
					match = match && (points[j].y - points[j - 1].y) > 15;
					break;
				case IN_ROW:
					match = match && fabs(points[j].y - lastMatchPoint.y) < 15;
					break;
				}
				if (match) {
					lastMatchPoint = points[j];
					actualPoints.push_back(points[j]);
					matchNo++;
					if (matchNo == numberOfParts) {
						actualPoints.insert(actualPoints.begin(), reference);
						return pair<bool, vector<Point2f> >(true, actualPoints);
					}
				}
			}
		}
		break;
	}

	return pair<bool, vector<Point2f> >(false, actualPoints);
}

int Object::findIndex(std::vector<Point2f> points, Point2f element) {
	auto res = std::find(points.begin(), points.end(), element);
	if(res!=points.end()){
		return std::distance(points.begin(), res);
	}else{
		return -1;
	}

}

std::pair<std::vector<int>, std::vector<int>> Object::detect(PointSet points,std::pair<std::vector< std::vector<cv::Point> > ,std::vector< std::vector<cv::Point> > > contourSet, Frame frame , Frame prevFrame) {

	vector<int> left;
	vector<int> right;
	std::pair<std::vector<int>, std::vector<int>> MatchPointIdx;
	MatchPointIdx.first = left;
	MatchPointIdx.second = right;

	if(points.left.size()==0 || points.right.size()==0){
		tracked = false;
		return MatchPointIdx;
	}

	if (tracked) {
		int lostCount = 0;

		for (auto i = 0; i < markerIds.size(); i++) {
			if (markers[markerIds[i]].isLost()) {
				tracked = false;
			}
		}

	}

	if (tracked) {
		for (auto i = 0; i < markerIds.size(); i++) {

		PointSet p;

		markers[markerIds[i]].refreshPosition(p , frame , prevFrame);

		if (!markers[markerIds[i]].isLost()) {
			MatchPointIdx.second.push_back(
					findIndex(points.left,
					markers[markerIds[i]].getPosition().left));
			MatchPointIdx.first.push_back(
					findIndex(points.right,
					markers[markerIds[i]].getPosition().right));
				}
			}

	}else{
	int c=0;
	while (!tracked) {
		c++;

		MatchPointIdx.first.clear();
		MatchPointIdx.second.clear();

		if(c==20)break;

		auto leftMatchPoints = findMatch(points.left);
		auto rightMatchPoints = findMatch(points.right);

		if (leftMatchPoints.first && rightMatchPoints.first) {

			tracked = true;

			cout<<"foundObject"<<endl;

			StereoPoint newPosition;
			newPosition.left = leftMatchPoints.second[0];
			newPosition.right = rightMatchPoints.second[0];

			PointSet pointset;

			for(size_t i = 0 ; i<contourSet.first.size() ; i++){
				if(cv::pointPolygonTest(contourSet.first[i] , newPosition.left , false )>0){
					for(size_t k = 0 ; k<contourSet.first[i].size() ; k++){
						pointset.left.push_back(cv::Point2f(contourSet.first[i][k].x,contourSet.first[i][k].y));
					}
					break;
				}
			}

			for(size_t i = 0 ; i<contourSet.second.size() ; i++){
				if(cv::pointPolygonTest(contourSet.second[i] , newPosition.right , false )>0){
					for(size_t k = 0 ; k<contourSet.second[i].size() ; k++){
						pointset.right.push_back(cv::Point2f(contourSet.second[i][k].x,contourSet.second[i][k].y));
					}
					break;
				}
			}

			markers[markerIds[0]].setPosition(newPosition);

			markers[markerIds[0]].setPosition(pointset);

			auto refPosition = markers[markerIds[0]].getRealPosition(
					leftCamMatrix, rightCamMatrix, r1, r2, p1, p2,
					leftDistCoeffs, rightDistCoeffs);

			MatchPointIdx.second.push_back(
					findIndex(points.right, rightMatchPoints.second[0]));
			MatchPointIdx.first.push_back(
					findIndex(points.left, leftMatchPoints.second[0]));

			for (auto i = 1; i < markerIds.size(); i++) {
				StereoPoint newPosition;
				newPosition.left = leftMatchPoints.second[i];
				newPosition.right = rightMatchPoints.second[i];

				PointSet pointset;

				for(size_t j = 0 ; j<contourSet.first.size() ; j++){
					if(cv::pointPolygonTest(contourSet.first[j] , newPosition.left , false )>0){
						for(size_t k = 0 ; k<contourSet.first[j].size() ; k++){
							pointset.left.push_back(cv::Point2f(contourSet.first[j][k].x,contourSet.first[j][k].y));
						}
						break;
					}
				}

				for(size_t j = 0 ; j<contourSet.second.size() ; j++){
					if(cv::pointPolygonTest(contourSet.second[j] , newPosition.right , false )>0){
						for(size_t k = 0 ; k<contourSet.second[j].size() ; k++){
							pointset.right.push_back(cv::Point2f(contourSet.second[j][k].x,contourSet.second[j][k].y));
						}
						break;
					}
				}

				markers[markerIds[i]].setPosition(newPosition);

				markers[markerIds[i]].setPosition(pointset);

				auto position = markers[markerIds[i]].getRealPosition(
						leftCamMatrix, rightCamMatrix, r1, r2, p1, p2,
						leftDistCoeffs, rightDistCoeffs);
				auto distanceFromRef = sqrtf(
						powf(position.x - refPosition.x, 2.0)
								+ powf(position.y - refPosition.y, 2.0)
								+ powf(position.z - refPosition.z, 2.0));

				if (fabs(
						distanceFromRef
								- markers[markerIds[i]].getReferenceDistance())
						> 1.0f) {

					cout<<distanceFromRef<<endl;

					auto leftIdx = findIndex(points.left,
							markers[markerIds[i]].getPosition().left);
					auto rightIdx = findIndex(points.right,
							markers[markerIds[i]].getPosition().right);

						if(points.left.size()>leftIdx){
							points.left.erase(points.left.begin() + leftIdx);
						}

						if(points.right.size()>rightIdx){
							points.right.erase(points.right.begin() + rightIdx);
						}


					tracked = false;

					if (points.left.size() == 0 || points.right.size() == 0) {
						break;
					}


				} else {
					MatchPointIdx.second.push_back(
							findIndex(points.right,
									rightMatchPoints.second[i]));
					MatchPointIdx.first.push_back(
							findIndex(points.left, leftMatchPoints.second[i]));
				}
			}
			}else{
				tracked = false;
			}
		}
	}
		if(!tracked){
			MatchPointIdx.first.clear();
			MatchPointIdx.second.clear();
		}
		return MatchPointIdx;
	}

	void Object::draw(Frame frames) {
		if (tracked) {
			for (auto i = 0; i < markerIds.size(); i++) {
				markers[markerIds[i]].draw(frames);
			}
		}
	}

	cv::Point3f Object::getMarkerPosition(std::string markerId){
		return markers[markerId].getRealPosition(leftCamMatrix, rightCamMatrix,
				  r1 , r2 , p1 , p2, leftDistCoeffs , rightDistCoeffs);
	}

	std::vector<std::string> Object::getMarkerIds(){
		return markerIds;
	}

	int Object::getNumberofParts() const {
		return numberOfParts;
	}

	bool Object::isTracked(){
		return tracked;
	}

	std::string
	Object::getId()
	{
		return id;
	}

	Object::~Object()
	{
		// TODO Auto-generated destructor stub
	}

