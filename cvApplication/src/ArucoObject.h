/*
 * ArucoObject.h
 *
 *  Created on: 2016. jún. 30.
 *      Author: Máté
 */

#ifndef ARUCOOBJECT_H_
#define ARUCOOBJECT_H_

#include "Object.h"

class ArucoObject: public Object {
private:
	std::vector<cv::Rect> markerROI;
	std::pair<int,int> findMatch(std::string markerId , std::pair<std::vector<int> , std::vector<int> > identifiers);
public:
	ArucoObject();

	virtual std::pair<std::vector<int> ,std::vector<int>> detect(PointSet points,std::pair<std::vector< std::vector<cv::Point> > ,std::vector< std::vector<cv::Point> > > contourSet , std::pair<std::vector<int> , std::vector<int> > identifiers);
	virtual void track(Frame frame , Frame prevFrame);

	virtual ~ArucoObject();
};

#endif /* ARUCOOBJECT_H_ */
