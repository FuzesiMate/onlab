/*
 * Visualizer.h
 *
 *  Created on: 2016. aug. 19.
 *      Author: Máté
 */

#ifndef VISUALIZER_H_
#define VISUALIZER_H_

#include <tbb/flow_graph.h>
#include "TemplateConfiguration.h"
#include "Camera.h"

class Visualizer: public tbb::flow::function_node<
		tbb::flow::tuple<Frame, ImageProcessingResult>  , tbb::flow::continue_msg , tbb::flow::queueing> {
private:
	std::vector<ImageProcessingResult> dataBuffer;
	std::vector<Frame> frameBuffer;
public:
	void draw(tbb::flow::tuple<Frame, ImageProcessingResult> data);
	Visualizer(tbb::flow::graph& g) :
			tbb::flow::function_node<
					tbb::flow::tuple<Frame, ImageProcessingResult>, tbb::flow::continue_msg , tbb::flow::queueing >(g, 1,
					std::bind(&Visualizer::draw, this ,std::placeholders::_1)) {};
	virtual ~Visualizer() = default;
};

#endif /* VISUALIZER_H_ */
