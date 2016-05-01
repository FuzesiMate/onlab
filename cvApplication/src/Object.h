/*
 * Object.h
 *
 *  Created on: 2016. ápr. 13.
 *      Author: Máté
 */
#include <opencv2/core.hpp>

#ifndef OBJECT_H_
#define OBJECT_H_

enum ReferencePosition{
	UPPER,
	LOWER,
	IN_ROW,
};

class Object {
private:
	std::pair<cv::Point2f,cv::Point2f> screenPosition;
	std::string id;
	ReferencePosition fromReference;
	ReferencePosition fromPrevious;
	float distanceFromReference;
	bool lost;
	int lostCount;

	cv::Point2f findClosest(std::vector<cv::Point2f> points, cv::Point2f reference);
public:

	Object();
	Object(std::string id, float distanceFromRef, ReferencePosition fromRef , ReferencePosition fromPrev);


	void setPosition(std::pair<cv::Point2f,cv::Point2f> position);
	void refreshPosition(std::pair<std::vector<cv::Point2f>,std::vector<cv::Point2f> > points);

	std::pair<cv::Point2f,cv::Point2f> getPosition();
	cv::Point3f getRealPosition(cv::Mat leftCamMatrix , cv::Mat rightCamMatrix , cv::Mat r1, cv::Mat r2 , cv::Mat p1 , cv::Mat p2,
				cv::Mat leftDistCoeffs , cv::Mat rightDistCoeffs);
	std::pair<ReferencePosition,ReferencePosition> getReferences();
	float getReferenceDistance();

	bool isLost();

	void draw(std::pair<cv::Mat,cv::Mat> frames);

	virtual ~Object();
};

#endif /* OBJECT_H_ */
