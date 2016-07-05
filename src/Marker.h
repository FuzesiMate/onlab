/*
 * Object.h
 *
 *  Created on: 2016. ápr. 13.
 *      Author: Máté
 */
#include <opencv2/core.hpp>
#include "stereoCamera.h"

#ifndef MARKER_H_
#define MARKER_H_

enum ReferencePosition{
	UPPER,
	LOWER,
	IN_ROW,
};

struct StereoPoint{
	cv::Point2f left;
	cv::Point2f right;
};

struct PointSet{
	std::vector<cv::Point2f> left;
	std::vector<cv::Point2f> right;
};

class Marker {
private:
	StereoPoint screenPositionCenter;
	PointSet screenPosition;
	std::string name;
	int id;
	ReferencePosition fromReference;
	ReferencePosition fromPrevious;

	float distanceFromReference;
	bool lost;
	int lostCount;

	cv::Point2f findClosest(std::vector<cv::Point2f> points, cv::Point2f reference);
	cv::Point2f calculateOpticalFlow(std::vector<cv::Point2f> currentPoints,
				std::vector<cv::Point2f>& nextPosition , cv::Mat currentFrame , cv::Mat prevFrame);
public:

	Marker();
	Marker(std::string id, float distanceFromRef, ReferencePosition fromRef , ReferencePosition fromPrev);
	Marker(std::string id , int markerId);

	Marker(std::string id , std::vector<ReferencePosition> references , std::vector<float> distances);

	void setPosition(StereoPoint position);
	void setPosition(PointSet position);

	void refreshPosition(PointSet points);
	void refreshPosition(Frame frame , Frame prevFrame);

	StereoPoint getPosition();
	cv::Point3f getRealPosition(cv::Mat leftCamMatrix , cv::Mat rightCamMatrix , cv::Mat r1, cv::Mat r2 , cv::Mat p1 , cv::Mat p2,
				cv::Mat leftDistCoeffs , cv::Mat rightDistCoeffs);
	std::pair<ReferencePosition,ReferencePosition> getReferences();

	void updateReference(ReferencePosition fromPrev, ReferencePosition fromRef,float referenceDistance);

	float getReferenceDistance();

	int getId();

	bool isLost();

	void setLost(bool lost);

	void draw(Frame frames);

	virtual ~Marker();
};

#endif /* MARKER_H_ */
