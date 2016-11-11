/*
 * CircleDetector.h
 *
 *  Created on: 2016. aug. 22.
 *      Author: Máté
 */

#ifndef CIRCLEDETECTOR_H_
#define CIRCLEDETECTOR_H_

#include "ImageProcessor.h"

template <typename CONFIG>
class CircleDetector: public ImageProcessor<CONFIG> {
public:
	ImageProcessingData<CONFIG> process(Frame frame);
	CircleDetector(tbb::flow::graph& g):ImageProcessor<CONFIG>(g){};
	virtual void reconfigure(boost::property_tree::ptree config);
	virtual ~CircleDetector() = default;
};

#endif /* CIRCLEDETECTOR_H_ */
