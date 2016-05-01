/*
 * Object.cpp
 *
 *  Created on: 2016. ápr. 13.
 *      Author: Máté
 */

#include "Object.h"
#include <opencv2/core.hpp>
#include<opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>
#include <math.h>
#include <iostream>


using namespace std;
using namespace cv;

float getDistance(Point2f point1 , Point2f point2){
	return sqrtf(powf(point1.x-point2.x , 2.0)+powf(point1.y-point2.y , 2.0));
}

Object::Object() {
	id 				= "" ;
	lost 			= false;
	lostCount 		= 0;
}

Object::Object(std::string id, float distanceFromRef ,ReferencePosition fromRef , ReferencePosition fromPrev ){
	lost 					= false;
	lostCount 				= 0;
	this->id 				= id;
	fromReference 			= fromRef;
	fromPrevious 			= fromPrev;
	distanceFromReference 	= distanceFromRef;
}


std::pair<cv::Point2f,cv::Point2f> Object::getPosition(){
	return screenPosition;
}

Point2f Object::findClosest(vector<Point2f> points, Point2f reference){

	Point2f result;
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

void Object::refreshPosition(std::pair<vector<Point2f>,vector<Point2f> > points){

	Point2f leftClosest;
	Point2f rightClosest;

	leftClosest=findClosest(points.first,screenPosition.first);
	rightClosest=findClosest(points.second,screenPosition.second);

	if((leftClosest.x==0 && leftClosest.y==0)||(rightClosest.x==0 && rightClosest.y==0)){
		lostCount++;
		if(lostCount==10){
			lostCount=0;
			lost = true;
		}

	}else if(fabs(leftClosest.x-rightClosest.x)<900 && fabs(leftClosest.x-rightClosest.x)>100){
		lostCount=0;
		screenPosition.first = leftClosest;
		screenPosition.second = rightClosest;
		lost = false;
	}
}

bool Object::isLost(){
	return lost;
}

void Object::setPosition(std::pair<cv::Point2f,cv::Point2f> position){
	screenPosition=position;
	lost = false;
}

void Object::draw(pair<Mat,Mat> frames){
	stringstream text;
	if(!lost){
		text<<id;
		circle(frames.first,Point(screenPosition.first.x,screenPosition.first.y) , 10, Scalar(255,255,255), 2.0 );
		circle(frames.second,Point(screenPosition.second.x,screenPosition.second.y) , 10, Scalar(255,255,255), 2.0 );
	}else{
		text<<"object "<<id<<" is lost";
	}

	putText(frames.first,text.str(),Point(screenPosition.first.x , screenPosition.first.y-30),
			FONT_HERSHEY_SIMPLEX, 1.0 , Scalar(255,255,255) , 2.0);
	putText(frames.second,text.str(),Point(screenPosition.second.x , screenPosition.second.y-30),
			FONT_HERSHEY_SIMPLEX, 1.0 , Scalar(255,255,255) , 2.0);
}


cv::Point3f Object::getRealPosition(cv::Mat leftCamMatrix , cv::Mat rightCamMatrix , cv::Mat r1, cv::Mat r2 , cv::Mat p1 , cv::Mat p2,
		cv::Mat leftDistCoeffs , cv::Mat rightDistCoeffs){

	vector<Point2f> leftPoints;
	leftPoints.push_back(screenPosition.first);

	vector<Point2f> rightPoints;
	rightPoints.push_back(screenPosition.second);

	undistortPoints(leftPoints,leftPoints,leftCamMatrix,leftDistCoeffs ,r1,p1);
	undistortPoints(rightPoints,rightPoints,rightCamMatrix,rightDistCoeffs , r1 ,p1);

	Mat cord;
	triangulatePoints(p1,p2,leftPoints,rightPoints,cord );

	float x,y,z;
	for(int i = 0 ; i<cord.cols ; i++){

		float w = cord.at<float>(3,i);
			  x = cord.at<float>(0,i)/w;
			  y = cord.at<float>(1,i)/w;
			  z = cord.at<float>(2,i)/(w);
			 //cout<<"x: "<< x <<" y: "<<y<<" z: "<<z<<endl;
	}

	return Point3f(x,y,z);
}

std::pair<ReferencePosition,ReferencePosition> Object::getReferences(){
	return std::pair<ReferencePosition,ReferencePosition>(fromReference,fromPrevious);
}

float Object::getReferenceDistance(){
	return distanceFromReference;
}

Object::~Object() {
}

