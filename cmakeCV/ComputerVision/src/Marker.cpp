/*
 * Marker.cpp
 *
 *  Created on: 2016. aug. 12.
 *      Author: Máté
 */

#include "Marker.h"
#include <iostream>

void Marker::setPosition(tbb::concurrent_vector<cv::Point2f> position){
	screenPosition = position;
	tracked = true;
}

tbb::concurrent_vector<cv::Point2f>& Marker::getPosition(){
	return screenPosition;
}

int Marker::getId(){
	return id;
}

bool Marker::isTracked(){
	return tracked;
}

void Marker::lost(){
	tracked = false;
}
