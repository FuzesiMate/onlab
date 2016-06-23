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

enum PhysicalOrientation{
	OTHER,
	IN_HORIZONTAL_ROW,
	IN_VERTICAL_ROW
};

class Object {
private:
	PhysicalOrientation orientation;
	int numberOfParts;
	bool tracked;
	int count = 0;
	std::map<std::string , Marker> markers;
	std::string id;
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
	std::pair<bool,std::vector<cv::Point2f> > findMatch(std::vector<cv::Point2f> points);

public:
	Object();
	void initializeObject(int numberOfParts , PhysicalOrientation physicalOrientation , std::string id);

	std::pair<std::vector<int> ,std::vector<int>> detect(PointSet points,std::pair<std::vector< std::vector<cv::Point> > ,std::vector< std::vector<cv::Point> > > contourSet);
	void track(Frame frame , Frame prevFrame);
	void updateState();

	void draw(Frame frames);
	void addPart(std::string id, float distanceFromRef, ReferencePosition fromRef , ReferencePosition fromPrev);
	int getNumberofParts() const;
	std::string getId();
	std::vector<std::string>getMarkerIds();
	cv::Point3f getMarkerPosition(std::string markerId);
	bool isTracked();
	virtual ~Object();
};

#endif /* OBJECT_H_ */
