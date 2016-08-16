/*
 * Marker.h
 *
 *  Created on: 2016. aug. 12.
 *      Author: Máté
 */

#ifndef MARKER_H_
#define MARKER_H_

#include <opencv2/core.hpp>
#include <tbb/concurrent_vector.h>
#include <iostream>

class Marker {
private:
	std::string name;
	int id;
	bool tracked;
	tbb::concurrent_vector<cv::Point2f> screenPosition;

public:
	Marker(std::string name , int id ):name(name),id(id),tracked(false){};
	void setPosition(tbb::concurrent_vector<cv::Point2f> position);
	tbb::concurrent_vector<cv::Point2f>& getPosition();
	int getId();
	bool isTracked();
	void lost();
	virtual ~Marker() = default;
};

#endif /* MARKER_H_ */
