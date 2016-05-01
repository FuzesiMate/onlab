/*
 * ComplexObject.h
 *
 *  Created on: 2016. ápr. 14.
 *      Author: Máté
 */

#ifndef COMPLEXOBJECT_H_
#define COMPLEXOBJECT_H_

#include "Object.h"
#include <opencv2/core.hpp>
#include <map>

enum PhysicalOrientation{
	OTHER,
	IN_HORIZONTAL_ROW,
	IN_VERTICAL_ROW
};

class ComplexObject {
private:
	PhysicalOrientation orientation;
	int numberOfParts;
	bool foundObject;
	int count = 0;
	std::map<std::string , Object> parts;
	std::string id;
	std::vector<std::string>objectIds;
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
	ComplexObject(int numberOfParts , PhysicalOrientation physicalOrientation , std::string id);
	std::pair<std::vector<int> ,std::vector<int>> detect(std::pair<std::vector<cv::Point2f> , std::vector<cv::Point2f> > points);
	void draw(std::pair<cv::Mat,cv::Mat> frames);
	void addPart(std::string id, float distanceFromRef, ReferencePosition fromRef , ReferencePosition fromPrev);
	int getNumberofParts() const;
	std::string getId();
	virtual ~ComplexObject();
};

#endif /* COMPLEXOBJECT_H_ */
