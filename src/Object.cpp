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
#include <opencv2/calib3d.hpp>


using namespace std;
using namespace cv;

Object::Object():tracked(false){

}

void Object::initializeObject(int numberOfParts, string id){
	this->numberOfParts = numberOfParts;
	tracked = false;
	this->name = id;

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

void Object::addPart(std::string id  , int markerIdentifier){
	Marker o(id, markerIdentifier);
	markerIds.push_back(id);
	markers[id] = o;
}



int Object::findIndex(std::vector<Point2f> points, Point2f element) {
	auto res = std::find(points.begin(), points.end(), element);
	if(res!=points.end()){
		return std::distance(points.begin(), res);
	}else{
		return -1;
	}

}




	void Object::draw(Frame frames) {
		if (tracked) {
			for (auto i = 0; i < markerIds.size(); i++) {

				/*
				 * Augmented reality, just for fun :)

				 *
				Mat rMat,tVec,camMatrix;

				Point3f mReal = markers[markerIds[i]].getRealPosition(leftCamMatrix, rightCamMatrix,
										r1, r2, p1, p2, leftDistCoeffs, rightDistCoeffs);

								decomposeProjectionMatrix(p1 , camMatrix , rMat , tVec);

								vector<Point3f> realPoints;
								vector<Point2f> imagePoints;

								for(int i = 0 ; i<15  ; i++){
									realPoints.push_back(Point3f(mReal.x, mReal.y+i,mReal.z));
								}

								Mat rotVec;
								Rodrigues(r1 , rotVec);

								vector<float> tv;

								tv.push_back(0);
								tv.push_back(0);
								tv.push_back(0);

								projectPoints(realPoints , rotVec , tv , leftCamMatrix , leftDistCoeffs , imagePoints);

								for(Point2f p : imagePoints){
									circle(frames.left , Point(p.x,p.y) , 2 , Scalar(255,255,255) , 2);
								}
				*/

				markers[markerIds[i]].draw(frames);
			}
		}
	}

	cv::Point3f Object::getMarkerPosition(std::string markerId){
		return markers[markerId].getRealPosition(leftCamMatrix, rightCamMatrix,
				  r1 , r2 , p1 , p2, leftDistCoeffs , rightDistCoeffs);
	}

	std::vector<std::string> Object::getMarkerNames(){
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
		return name;
	}

	Object::~Object()
	{
		// TODO Auto-generated destructor stub
	}

