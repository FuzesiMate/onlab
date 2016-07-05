/*
 * Object.cpp
 *
 *  Created on: 2016. ápr. 13.
 *      Author: Máté
 */

#include "Marker.h"

#include <opencv2/core.hpp>
#include<opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/video.hpp>
#include <math.h>
#include <iostream>


using namespace std;

float getDistance(cv::Point2f point1 , cv::Point2f point2){
	return sqrtf(powf(point1.x-point2.x , 2.0)+powf(point1.y-point2.y , 2.0));
}

Marker::Marker() {
	name 				= "" ;
	lost 			= false;
	lostCount 		= 0;
	screenPositionCenter.left = cv::Point2f(0,0);
	screenPositionCenter.right = cv::Point2f(0,0);
}

Marker::Marker(std::string id, float distanceFromRef ,ReferencePosition fromRef , ReferencePosition fromPrev ){
	lost 					= false;
	lostCount 				= 0;
	this->name 				= id;
	fromReference 			= fromRef;
	fromPrevious 			= fromPrev;
	distanceFromReference 	= distanceFromRef;
}

Marker::Marker(std::string id , int markerId){
	this->name = id;
	this->id = markerId;
}

StereoPoint Marker::getPosition(){
	return screenPositionCenter;
}

void Marker::updateReference(ReferencePosition fromPrev, ReferencePosition fromRef , float referenceDistance){
	fromPrevious = fromPrev;
	fromReference = fromRef;
	distanceFromReference = referenceDistance;
}

cv::Point2f Marker::findClosest(vector<cv::Point2f> points, cv::Point2f reference){

	cv::Point2f result;
	if(points.size()>0){
		float mindistance = getDistance(points[0] , reference);

		int minIndex = 0;
		for(auto i = 0 ; i<points.size() ; i++){
			float dist = getDistance(points[i],reference);

			if(dist<mindistance){
				mindistance=dist;
				minIndex=i;
			}
		}

		if(mindistance<30)
			result = points[minIndex];
	}
	return result;
}

cv::Point2f Marker::calculateOpticalFlow(std::vector<cv::Point2f> currentPoints,
		std::vector<cv::Point2f>& nextPosition , cv::Mat currentFrame , cv::Mat prevFrame){

	std::vector<uchar> status;
	std::vector<float> error;

	cv::TermCriteria termcrit(cv::TermCriteria::COUNT|cv::TermCriteria::EPS,20,0.03);

	try{
		cv::calcOpticalFlowPyrLK(prevFrame , currentFrame ,currentPoints , nextPosition,
					status, error , cv::Size(10,10) , 10 , termcrit);
	}catch(cv::Exception &e){
		throw e;
	}

	float averageError=0;

	for(float& e : error){
		averageError+=e;
	}

	averageError = averageError/error.size();

	if(averageError>5.0f){
		lost = true;
	}

	cv::Point2f center;

	if(!nextPosition.empty()){
		float r;
		cv::minEnclosingCircle(nextPosition , center ,r);
	}

	return center;
}

void Marker::refreshPosition(Frame currentFrame , Frame prevFrame){
	PointSet nextPosition;
	try{
		screenPositionCenter.left = calculateOpticalFlow(screenPosition.left , nextPosition.left , currentFrame.left , prevFrame.left);
		screenPositionCenter.right = calculateOpticalFlow(screenPosition.right , nextPosition.right , currentFrame.right , prevFrame.right);
	}catch(cv::Exception &e){
		return;
	}

	screenPosition = nextPosition;
}

void Marker::setPosition(PointSet position){
	screenPosition = position;
	lost = false;
}

void Marker::refreshPosition(PointSet points){

	cv::Point2f leftClosest;
	cv::Point2f rightClosest;

	leftClosest=findClosest(points.left,screenPositionCenter.left);
	rightClosest=findClosest(points.right,screenPositionCenter.right);

	if((leftClosest.x==0 && leftClosest.y==0)||(rightClosest.x==0 && rightClosest.y==0)){
		lostCount++;
		if(lostCount==30){
			lostCount=0;
			lost = true;
		}

	}else if(fabs(leftClosest.x-rightClosest.x)<900 && fabs(leftClosest.x-rightClosest.x)>100){
		lostCount=0;
		screenPositionCenter.left = leftClosest;
		screenPositionCenter.right = rightClosest;
		lost = false;
	}

}

bool Marker::isLost(){
	return lost;
}

void Marker::setPosition(StereoPoint position){
	screenPositionCenter=position;
	lost = false;
}

void Marker::draw(Frame frames){
	stringstream text;
	if(!lost){
		text<<name;
		for(size_t i = 0 ; i<screenPosition.left.size() ; i++){
			cv::circle(frames.left,cv::Point(screenPositionCenter.left.x,screenPositionCenter.left.y) , 10, cv::Scalar(255,255,255), 2.0 );
		}

		for(size_t i = 0 ; i<screenPosition.right.size() ; i++){
			cv::circle(frames.right,cv::Point(screenPositionCenter.right.x,screenPositionCenter.right.y) , 10, cv::Scalar(255,255,255), 2.0 );
		}
	}else{
		text<<"object "<<name<<" is lost";
	}

	cv::putText(frames.left,text.str(),cv::Point(screenPositionCenter.left.x , screenPositionCenter.left.y-30),
			cv::FONT_HERSHEY_SIMPLEX, 1.0 , cv::Scalar(255,255,255) , 2.0);
	cv::putText(frames.right,text.str(),cv::Point(screenPositionCenter.right.x , screenPositionCenter.right.y-30),
			cv::FONT_HERSHEY_SIMPLEX, 1.0 , cv::Scalar(255,255,255) , 2.0);
}


cv::Point3f Marker::getRealPosition(cv::Mat leftCamMatrix , cv::Mat rightCamMatrix , cv::Mat r1, cv::Mat r2 , cv::Mat p1 , cv::Mat p2,
		cv::Mat leftDistCoeffs , cv::Mat rightDistCoeffs){

	vector<cv::Point2f> leftPoints;
	leftPoints.push_back(screenPositionCenter.left);

	vector<cv::Point2f> rightPoints;
	rightPoints.push_back(screenPositionCenter.right);

	undistortPoints(leftPoints,leftPoints,leftCamMatrix,leftDistCoeffs ,r1,p1);
	undistortPoints(rightPoints,rightPoints,rightCamMatrix,rightDistCoeffs , r1 ,p1);

	cv::Mat cord;
	cv::triangulatePoints(p1,p2,leftPoints,rightPoints,cord );

	float x,y,z;
	for(int i = 0 ; i<cord.cols ; i++){

		float w = cord.at<float>(3,i);
			  x = cord.at<float>(0,i)/w;
			  y = cord.at<float>(1,i)/w;
			  z = cord.at<float>(2,i)/(w);
			 //cout<<"x: "<< x <<" y: "<<y<<" z: "<<z<<endl;
	}

	return cv::Point3f(x,y,z);
}

std::pair<ReferencePosition,ReferencePosition> Marker::getReferences(){
	return std::pair<ReferencePosition,ReferencePosition>(fromReference,fromPrevious);
}

float Marker::getReferenceDistance(){
	return distanceFromReference;
}

int Marker::getId(){
	return id;
}

void Marker::setLost(bool lost){
  this->lost = lost;
}

Marker::~Marker() {
}

