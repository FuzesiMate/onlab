/*
 * CircleDetector.cpp
 *
 *  Created on: 2016. aug. 22.
 *      Author: Máté
 */

#include "CircleDetector.h"
#include <thread>

template <typename CONFIG>
ImageProcessingData<CONFIG> CircleDetector<CONFIG>::process(Frame frame){
	//TODO implement Balint's circle detection without magic constant
	ImageProcessingData<CONFIG> ipData;

	ipData.data = tbb::concurrent_vector<tbb::concurrent_vector<cv::Point2f> >(frame.images.size());
	ipData.identifiers = tbb::concurrent_vector<tbb::concurrent_vector<int> >(frame.images.size());
	ipData.frameIndex = frame.frameIndex;
	ipData.timestamp = frame.timestamp;

	std::cout << "start " << std::endl;
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	std::cout << "stop" << std::endl;

	for(auto f : frame.images){
		std::vector<cv::Point3f> circles;
		//cvtColor(f , f , CV_RGB2GRAY);
		//cv::HoughCircles(f , circles , CV_HOUGH_GRADIENT , 1 , 20 , 100 , 80 , 0 , 0 );
	}

	return ipData;
}

template <typename CONFIG>
void CircleDetector<CONFIG>::setProcessingSpecificValues(boost::property_tree::ptree config){

}

