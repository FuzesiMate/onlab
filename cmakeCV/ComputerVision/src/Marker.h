/*
 * Marker.h
 *
 *  Created on: 2016. aug. 12.
 *      Author: M�t�
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

public:
	Marker(std::string name , int id ):name(name),id(id){};
	int getId();
	virtual ~Marker() = default;
};

#endif /* MARKER_H_ */
