/*
 * ArucoObject.cpp
 *
 *  Created on: 2016. jún. 30.
 *      Author: Máté
 */

#include "ArucoObject.h"

ArucoObject::ArucoObject() {
	// TODO Auto-generated constructor stub

}

std::pair<std::vector<int> ,std::vector<int>> ArucoObject::detect(PointSet points,std::pair<std::vector< std::vector<cv::Point> > ,std::vector< std::vector<cv::Point> > > contourSet , std::pair<std::vector<int> , std::vector<int> > identifiers){
	for(auto &i : identifiers.first){
		std::cout<<i<<std::endl;
	}
}

void ArucoObject::track(Frame frame , Frame prevFrame){
	std::cout<<"nothinghappensrightnow2"<<std::endl;
}

ArucoObject::~ArucoObject() {
	// TODO Auto-generated destructor stub
}

