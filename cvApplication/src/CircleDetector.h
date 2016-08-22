/*
 * CircleDetector.h
 *
 *  Created on: 2016. aug. 22.
 *      Author: M�t�
 */

#ifndef CIRCLEDETECTOR_H_
#define CIRCLEDETECTOR_H_

#include "ImageProcessor.h"

template <typename CONFIG>
class CircleDetector: public ImageProcessor<CONFIG> {
public:
	virtual ImageProcessingData<CONFIG> ProcessNextFrame(Frame frame);
	CircleDetector(tbb::flow::graph& g):ImageProcessor<CONFIG>(g){};
	virtual ~CircleDetector() = default;
};

#endif /* CIRCLEDETECTOR_H_ */