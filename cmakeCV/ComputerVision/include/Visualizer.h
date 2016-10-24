/*
 * Visualizer.h
 *
 *  Created on: 2016. aug. 19.
 *      Author: M�t�
 */

#ifndef VISUALIZER_H_
#define VISUALIZER_H_

#include <tbb/flow_graph.h>
#include "TemplateConfiguration.h"
#include "Camera.h"
#include "Processor.h"

class Visualizer: public Processor<
		tbb::flow::tuple<Frame, ModelData>  , tbb::flow::continue_msg , tbb::flow::queueing> {
private:
	std::vector<ModelData> dataBuffer;
	std::vector<Frame> frameBuffer;
	std::string windowName;
	int64_t delay;
public:
	tbb::flow::continue_msg process(tbb::flow::tuple<Frame, ModelData> data);
	Visualizer(std::string windowName, int64_t delay ,tbb::flow::graph& g) :
			Processor<
					tbb::flow::tuple<Frame, ModelData>, tbb::flow::continue_msg , tbb::flow::queueing >(g ,1),windowName(windowName),delay(delay) {};
	virtual ~Visualizer() = default;
};

#endif /* VISUALIZER_H_ */
