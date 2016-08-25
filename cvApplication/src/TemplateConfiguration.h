/*
 * TemplateConfiguration.h
 *
 *  Created on: 2016. aug. 16.
 *      Author: Máté
 */

#ifndef TEMPLATECONFIGURATION_H_
#define TEMPLATECONFIGURATION_H_

#include <tbb/concurrent_unordered_map.h>
#include <tbb/concurrent_vector.h>
#include <opencv2/opencv.hpp>

enum MarkerType{
	ARUCO,
	CIRCLE
};

typedef tbb::concurrent_unordered_map<std::string , tbb::concurrent_unordered_map<std::string , tbb::concurrent_vector<cv::Point2f> > > ImageProcessingResult;

struct MarkerPosition{
	tbb::concurrent_unordered_map<std::string , tbb::concurrent_vector<cv::Point2f> > position;
	std::string objectName;
	int64_t frameIndex;
	bool tracked;
};


template<typename data , typename identifier>
struct TEMPLATE_CONFIG{
	using dataType = data;
	using identifierType = identifier;
};

#endif /* TEMPLATECONFIGURATION_H_ */
