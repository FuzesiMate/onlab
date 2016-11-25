/*
 * CircleDetector.cpp
 *
 *  Created on: 2016. aug. 22.
 *      Author: Máté
 */

#include "CircleDetector.h"
#include <thread>
#include <tbb/parallel_for.h>

template <typename CONFIG>
ImageProcessingData<CONFIG> CircleDetector<CONFIG>::process(Frame frame){
	//TODO implement Balint's circle detection without magic constant
	ImageProcessingData<CONFIG> ipData;

	ipData.data = tbb::concurrent_vector<tbb::concurrent_vector<cv::Point2f> >(frame.images.size());
	ipData.identifiers = tbb::concurrent_vector<tbb::concurrent_vector<int> >(frame.images.size());
	ipData.frameIndex = frame.frameIndex;
	ipData.timestamp = frame.timestamp;

	std::vector<cv::Point3f> circles;

	tbb::parallel_for(size_t(0), frame.images.size(), [&](size_t i) {
		/*cv::Mat gray;
		cv::cvtColor(frame.images[i] , gray , CV_RGB2GRAY);*/
		cv::HoughCircles(frame.images[i], circles, CV_HOUGH_GRADIENT, 1, 20, 100, 80, 0, 0);
	});

	return ipData;
}

template <typename CONFIG>
void CircleDetector<CONFIG>::reconfigure(boost::property_tree::ptree config){

}

