/*
 * ArucoImageProcessor.h
 *
 *  Created on: 2016. aug. 9.
 *      Author: Máté
 */

#ifndef ARUCOIMAGEPROCESSOR_H_
#define ARUCOIMAGEPROCESSOR_H_

#include "ImageProcessor.h"
#include <opencv2/aruco.hpp>
#include <tbb/flow_graph.h>
#include <fstream>
#include <mutex>

template<typename CONFIG>
class ArucoImageProcessor : public ImageProcessor<CONFIG> {
private:
	cv::Ptr<cv::aruco::Dictionary> dictionary;
	cv::Ptr<cv::aruco::DetectorParameters> detectorParams;
	int64_t prevFrameIdx;

	std::mutex lock;

public:
	ArucoImageProcessor(tbb::flow::graph& g) :ImageProcessor<CONFIG>(g), dictionary(cv::aruco::getPredefinedDictionary(cv::aruco::PREDEFINED_DICTIONARY_NAME(0))), detectorParams(cv::aruco::DetectorParameters::create()), prevFrameIdx(0) {};
	ImageProcessingData<CONFIG> process(Frame frame);

	virtual void reconfigure(boost::property_tree::ptree config);

	virtual ~ArucoImageProcessor() = default;
};

#endif /* ARUCOIMAGEPROCESSOR_H_ */
