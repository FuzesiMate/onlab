/*
 * ArucoImageProcessor.cpp
 *
 *  Created on: 2016. aug. 9.
 *      Author: Máté
 */

#include "ArucoImageProcessor.h"
#include "tbb/parallel_for.h"
#include "tbb/concurrent_vector.h"
#include <opencv2/aruco.hpp>
#include <chrono>

template <typename CONFIG>
ImageProcessingData< CONFIG >ArucoImageProcessor<CONFIG>::process(Frame frame){

	ImageProcessingData<CONFIG> foundMarkers;

	foundMarkers.data = tbb::concurrent_vector<tbb::concurrent_vector<cv::Point2f> >(frame.images.size());
	foundMarkers.identifiers = tbb::concurrent_vector<tbb::concurrent_vector<int> >(frame.images.size());

	tbb::parallel_for(size_t(0) , frame.images.size() , [&](size_t i){

		std::vector< std::vector< cv::Point2f > > corners, rejected;
		std::vector<int> identifiers;
		cv::aruco::detectMarkers(frame.images[i] , dictionary , corners , identifiers , detectorParams , rejected) ;

		cv::Point2f center;
		float r;

		tbb::concurrent_vector<cv::Point2f> markerPosition(corners.size());
		tbb::concurrent_vector<int> 		markerIdentifier(identifiers.size());

		int j = 0 ;
		for(auto& corner : corners){
			cv::minEnclosingCircle(corner , center , r);
			markerPosition[j] = center;
			markerIdentifier[j]=identifiers[j];
			j++;
		}

		foundMarkers.data[i]=(markerPosition);
		foundMarkers.identifiers[i]=(markerIdentifier);
	});

	foundMarkers.timestamp = frame.timestamp;
	foundMarkers.frameIndex = frame.frameIndex;
	prevFrameIdx = frame.frameIndex;

	return foundMarkers;
}

template <typename CONFIG>
void ArucoImageProcessor<CONFIG>::setProcessingSpecificValues(boost::property_tree::ptree config){
	std::cout<<"set aruco values"<<std::endl;
}
