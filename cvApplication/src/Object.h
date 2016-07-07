/*
 * ComplexObject.h
 *
 *  Created on: 2016. ápr. 14.
 *      Author: Máté
 */

#ifndef OBJECT_H_
#define OBJECT_H_

#include <opencv2/core.hpp>
#include <map>
#include "Marker.h"
#include "stereoCamera.h"
#include <iostream>

class Object {
protected:
	int numberOfParts;
	bool tracked;
	int count = 0;
	std::map<std::string , Marker> markers;
	std::string name;
	std::vector<std::string>markerIds;
	cv::Mat leftCamMatrix;
	cv::Mat rightCamMatrix;
	cv::Mat leftDistCoeffs;
	cv::Mat rightDistCoeffs;
	cv::Mat p1;
	cv::Mat p2;
	cv::Mat r1;
	cv::Mat r2;

	int findIndex(std::vector<cv::Point2f> points, cv::Point2f element);

public:
	Object();
	void initializeObject(int numberOfParts , std::string id);

	virtual std::pair<std::vector<int> ,std::vector<int>> detect(PointSet points,std::pair<std::vector< std::vector<cv::Point> > ,std::vector< std::vector<cv::Point> > > contourSet , std::pair<std::vector<int> , std::vector<int> > identifiers)=0;
	virtual void track(Frame frame , Frame prevFrame)=0;

	void draw(Frame frames);
	void addPart(std::string id, float distanceFromRef, ReferencePosition fromRef , ReferencePosition fromPrev);
	void addPart(std::string id  , int markerIdentifier);
	int getNumberofParts() const;
	std::string getId();
	std::vector<std::string>getMarkerNames();
	std::vector<int> getMarkerIds();
	cv::Point3f getMarkerPosition(std::string markerId);
	bool isTracked(std::string markerName);
	bool isTracked();
	virtual ~Object();
};

#endif /* OBJECT_H_ */
