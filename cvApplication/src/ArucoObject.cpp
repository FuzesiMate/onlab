/*
 * ArucoObject.cpp
 *
 *  Created on: 2016. jún. 30.
 *      Author: Máté
 */

#include "ArucoObject.h"

ArucoObject::ArucoObject() {
}

std::pair<std::vector<int> ,std::vector<int>> ArucoObject::detect(PointSet points,std::pair<std::vector< std::vector<cv::Point> > ,std::vector< std::vector<cv::Point> > > contourSet , std::pair<std::vector<int> , std::vector<int> > identifiers){
	std::cout<<"identifiers"<<std::endl;
	for(auto &i : identifiers.first){
		std::cout<<i<<std::endl;
	}
	std::cout<<"identifiers end"<<std::endl;
}

void ArucoObject::track(Frame frame , Frame prevFrame){
	std::cout<<"nothinghappensrightnow"<<std::endl;
}

ArucoObject::~ArucoObject() {
	// TODO Auto-generated destructor stub
}

