/*
 * ImageProcessor.h
 *
 *  Created on: 2016. aug. 9.
 *      Author: Máté
 */

#ifndef IMAGEPROCESSOR_H_
#define IMAGEPROCESSOR_H_

#include <tbb/flow_graph.h>
#include <opencv2/opencv.hpp>
#include <tbb/concurrent_vector.h>
#include "Camera.h"

template <typename CONFIG> struct ImageProcessingData{
	tbb::concurrent_vector<typename CONFIG::dataType> data;
	tbb::concurrent_vector<typename CONFIG::identifierType> identifiers;
	int64_t timestamp;
	int64_t frameIndex;
};

template<typename CONFIG>

class ImageProcessor: public tbb::flow::function_node<Frame, ImageProcessingData<CONFIG>,tbb::flow::queueing> {
public:
	virtual ImageProcessingData<CONFIG> ProcessNextFrame(Frame frame)=0;

	ImageProcessor(tbb::flow::graph& g) :tbb::flow::function_node<Frame,ImageProcessingData<CONFIG>, tbb::flow::queueing>
					(g, tbb::flow::unlimited ,std::bind(&ImageProcessor::ProcessNextFrame, this,std::placeholders::_1)) {};

	virtual ~ImageProcessor() = default;
};

#endif /* IMAGEPROCESSOR_H_ */
