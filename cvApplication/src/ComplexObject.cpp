/*
 * ComplexObject.cpp
 *
 *  Created on: 2016. ápr. 14.
 *      Author: Máté
 */

#include "ComplexObject.h"
#include <math.h>
#include <iostream>
#include <algorithm>
#include <string>
#include <opencv2/highgui.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

using namespace std;
using namespace cv;
ComplexObject::ComplexObject(int numberOfParts,PhysicalOrientation physicalOrientation, string id) {
	orientation = physicalOrientation;
	this->numberOfParts = numberOfParts;
	foundObject = false;
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

void ComplexObject::addPart(std::string id, float distanceFromRef,
		ReferencePosition fromRef, ReferencePosition fromPrev) {
	Object o(id, distanceFromRef, fromRef, fromPrev);
	objectIds.push_back(id);
	parts[id] = o;
}

pair<bool, vector<Point2f> > ComplexObject::findMatch(
		vector<cv::Point2f> points) {

	vector<Point2f> matchPoints;
	vector<Point2f> actualPoints;

	switch (orientation) {
	case IN_HORIZONTAL_ROW:
		std::sort(points.begin(), points.end(),
				[](const cv::Point2f &a , const cv::Point2f &b)-> bool {return a.x<b.x;});
		for (auto i = 0; i < points.size(); i++) {
			actualPoints.clear();
			for (auto j = i + 1; j < points.size(); j++) {
				if (fabs(points[i].y - points[j].y) < 10
						&& std::find(matchPoints.begin(), matchPoints.end(),
								points[i]) == matchPoints.end()) {
					actualPoints.push_back(points[j]);
				}
			}

			matchPoints.insert(matchPoints.begin(), actualPoints.begin(),
					actualPoints.end());

			if (actualPoints.size() == numberOfParts - 1) {
				actualPoints.insert(actualPoints.begin(), points[i]);
				return pair<bool, vector<Point2f> >(true, actualPoints);
			}
		}
		break;
	case IN_VERTICAL_ROW:
		std::sort(points.begin(), points.end(),
				[](const cv::Point2f &a , const cv::Point2f &b)-> bool {return a.y<b.y;});
		for (auto i = 0; i < points.size(); i++) {
			actualPoints.clear();
			for (auto j = i + 1; j < points.size(); j++) {
				if (fabs(points[i].x - points[j].x) < 10
						&& std::find(matchPoints.begin(), matchPoints.end(),
								points[i]) == matchPoints.end()) {

					actualPoints.push_back(points[j]);
				}
			}

			matchPoints.insert(matchPoints.begin(), actualPoints.begin(),
					actualPoints.end());

			if (actualPoints.size() == numberOfParts - 1) {
				actualPoints.insert(actualPoints.begin(), points[i]);
				return pair<bool, vector<Point2f> >(true, actualPoints);
			}
		}
		break;
	case OTHER:
		std::sort(points.begin(), points.end(),
				[](const cv::Point2f &a , const cv::Point2f &b)-> bool {return a.x<b.x;});

		for(auto i = 0 ; i<points.size()-1 ; i++){
			if(fabs(points[i].x-points[i+1].x)<10){
				if(points[i].y>points[i+1].y){
					auto temp = points[i];
					points[i]=points[i+1];
					points[i+1]=temp;
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
				auto refPos = parts[objectIds[matchNo]].getReferences();

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

int ComplexObject::findIndex(std::vector<Point2f> points, Point2f element){
	auto res = std::find(points.begin(), points.end(),element);
	return std::distance(points.begin() , res);
}

std::pair<std::vector<int> ,std::vector<int>> ComplexObject::detect(
		std::pair<std::vector<cv::Point2f>, std::vector<cv::Point2f> > points) {

	vector<int> left;
	vector<int> right;
	std::pair<std::vector<int> ,std::vector<int>> MatchPointIdx;
	MatchPointIdx.first = left;
	MatchPointIdx.second = right;

	if (foundObject) {
		int lostCount = 0;

		for (auto i = 0; i < objectIds.size(); i++) {
			if (parts[objectIds[i]].isLost()) {
				lostCount++;
			}
		}

		if (lostCount == numberOfParts) {
			foundObject = false;
		}

	}

	if((points.first.size() > 0 && points.second.size() > 0)) {
		if (!foundObject) {

			std::pair<bool, vector<Point2f>> leftMatchPoints = findMatch(points.first);
			std::pair<bool, vector<Point2f>> rightMatchPoints = findMatch(points.second);

			if (leftMatchPoints.first && rightMatchPoints.first) {
				count++;
			}

			if (count == 10) {
				count = 0;
				foundObject = true;

				parts[objectIds[0]].setPosition(
						std::pair<Point2f, Point2f>(leftMatchPoints.second[0], rightMatchPoints.second[0]));
				auto refPosition = parts[objectIds[0]].getRealPosition(
						leftCamMatrix, rightCamMatrix, r1, r2, p1, p2,
						leftDistCoeffs, rightDistCoeffs);

				MatchPointIdx.second.push_back(findIndex(points.second , rightMatchPoints.second[0]));
				MatchPointIdx.first.push_back(findIndex(points.first ,  leftMatchPoints.second[0]));

				for (auto i = 1; i < objectIds.size(); i++) {
					parts[objectIds[i]].setPosition(
							std::pair<Point2f, Point2f>(leftMatchPoints.second[i],
									rightMatchPoints.second[i]));

					auto position = parts[objectIds[i]].getRealPosition(
							leftCamMatrix, rightCamMatrix, r1, r2, p1, p2,
							leftDistCoeffs, rightDistCoeffs);
					auto distanceFromRef = sqrtf(
							powf(position.x - refPosition.x, 2.0)
									+ powf(position.y - refPosition.y, 2.0)
									+ powf(position.z - refPosition.z, 2.0));

					if (fabs(distanceFromRef - parts[objectIds[i]].getReferenceDistance()) > 3.0f) {
						auto leftIdx = findIndex(points.first , parts[objectIds[i]].getPosition().first);
						auto rightIdx = findIndex(points.second , parts[objectIds[i]].getPosition().second);

						points.first.erase(points.first.begin()+leftIdx);
						points.second.erase(points.second.begin()+rightIdx);

						cout<<leftIdx<<endl;
						cout<<rightIdx<<endl;

						foundObject = false;
					}else{
						MatchPointIdx.second.push_back(findIndex(points.second , rightMatchPoints.second[i]));
						MatchPointIdx.first.push_back(findIndex(points.first ,  leftMatchPoints.second[i]));
					}
				}
			}

		} else {
			for (auto i = 0; i < objectIds.size(); i++) {
				parts[objectIds[i]].refreshPosition(points);
				parts[objectIds[i]].getRealPosition(leftCamMatrix,
						rightCamMatrix, r1, r2, p1, p2, leftDistCoeffs,
						rightDistCoeffs);
				if(!parts[objectIds[i]].isLost()){
					MatchPointIdx.second.push_back(findIndex(points.second , parts[objectIds[i]].getPosition().second));
					MatchPointIdx.first.push_back(findIndex(points.first , parts[objectIds[i]].getPosition().first));
				}

			}
		}
	}else{
		foundObject=false;
	}


	return MatchPointIdx;
}

void ComplexObject::draw(std::pair<cv::Mat, cv::Mat> frames) {
	if (foundObject) {
		stringstream name;
		name << id;

		//putText(frames.first,name.str() , Point(parts["bal"].getPosition().first.x,parts["bal"].getPosition().first.y-60), FONT_HERSHEY_SIMPLEX , 1.0,Scalar(255,255,255), 2.0);

		for (auto i = 0; i < objectIds.size(); i++) {
			parts[objectIds[i]].draw(frames);
		}
	}
}

int ComplexObject::getNumberofParts() const{
	return numberOfParts;
}

std::string ComplexObject::getId(){
	return id;
}

ComplexObject::~ComplexObject() {
	// TODO Auto-generated destructor stub
}

