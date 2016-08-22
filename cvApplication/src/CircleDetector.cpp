/*
 * CircleDetector.cpp
 *
 *  Created on: 2016. aug. 22.
 *      Author: Máté
 */

#include "CircleDetector.h"

template <typename CONFIG>
ImageProcessingData<CONFIG> CircleDetector<CONFIG>::ProcessNextFrame(Frame frame){
	//TODO implement Balint's circle detection without magic constant

	std::cout<<"call"<<std::endl;
	ImageProcessingData<CONFIG> ipData;

	ipData.data = tbb::concurrent_vector<tbb::concurrent_vector<cv::Point2f> >(frame.images.size());
	ipData.identifiers = tbb::concurrent_vector<tbb::concurrent_vector<int> >(frame.images.size());

	for(auto f : frame.images){
		std::vector<cv::Point3f> circles;
		cvtColor(f , f , CV_RGB2GRAY);
		cv::HoughCircles(f , circles , CV_HOUGH_GRADIENT , 1 , 20 , 100 , 80 , 0 , 0 );
		for(auto& p : circles){
			std::cout<<p<<std::endl;
		}
	}

	return ipData;
}

