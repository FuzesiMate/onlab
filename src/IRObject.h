/*
 * IRObject.h
 *
 *  Created on: 2016. jún. 30.
 *      Author: Máté
 */

#ifndef IROBJECT_H_
#define IROBJECT_H_

#include "Object.h"
#include <string>
#include <iostream>
#include <opencv2/highgui.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>


class IRObject: public Object {
private:
	std::pair<bool, std::vector<cv::Point2f> > findMatch(std::vector<cv::Point2f> points) ;
public:
	IRObject();
	virtual std::pair<std::vector<int> ,std::vector<int>> detect(PointSet points,std::pair<std::vector< std::vector<cv::Point> > ,std::vector< std::vector<cv::Point> > > contourSet , std::pair<std::vector<int> , std::vector<int> > identifiers) override;
	virtual void track(Frame frame , Frame prevFrame) override;
	virtual ~IRObject();
};

#endif /* IROBJECT_H_ */
