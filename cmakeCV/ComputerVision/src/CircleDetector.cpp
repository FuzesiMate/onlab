/*
 * CircleDetector.cpp
 *
 *  Created on: 2016. aug. 22.
 *      Author: M�t�
 */

#include "CircleDetector.h"

template <typename CONFIG>
ImageProcessingData<CONFIG> CircleDetector<CONFIG>::process(Frame frame){
	//TODO implement Balint's circle detection without magic constant
	ImageProcessingData<CONFIG> ipData;

	ipData.data = tbb::concurrent_vector<tbb::concurrent_vector<cv::Point2f> >(frame.images.size());
	ipData.identifiers = tbb::concurrent_vector<tbb::concurrent_vector<int> >(frame.images.size());
	ipData.frameIndex = frame.frameIndex;
	ipData.timestamp = frame.timestamp;

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

