/*
 * ImageProcessor.h
 *
 *  Created on: 2016. aug. 9.
 *      Author: M�t�
 */

#ifndef IMAGEPROCESSOR_H_
#define IMAGEPROCESSOR_H_

#include <tbb/flow_graph.h>
#include <opencv2/opencv.hpp>
#include <tbb/concurrent_vector.h>
#include "Camera.h"
#include "Processor.h"

template <typename CONFIG> struct ImageProcessingData{
	tbb::concurrent_vector<typename CONFIG::dataType> data;
	tbb::concurrent_vector<typename CONFIG::identifierType> identifiers;
	int64_t timestamp;
	int64_t frameIndex;
};

template<typename CONFIG>

class ImageProcessor :public Processor<Frame , ImageProcessingData<CONFIG>  , tbb::flow::queueing>{
public:
	virtual ImageProcessingData<CONFIG> process(Frame frame)=0;

	ImageProcessor(tbb::flow::graph& g):Processor<Frame , ImageProcessingData<CONFIG>  , tbb::flow::queueing>(g , tbb::flow::unlimited){}

	virtual ~ImageProcessor() = default;
};

#endif /* IMAGEPROCESSOR_H_ */
