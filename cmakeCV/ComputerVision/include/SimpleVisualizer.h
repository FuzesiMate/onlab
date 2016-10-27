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
#include "Visualizer.h"

class SimpleVisualizer: public Visualizer{
private:
	std::vector<ModelData> dataBuffer;
	std::vector<Frame> frameBuffer;
	std::string windowName;
	int64_t delay;
public:
	tbb::flow::continue_msg process(tbb::flow::tuple<Frame, ModelData> data);
	SimpleVisualizer(std::string windowName, int64_t delay ,tbb::flow::graph& g):Visualizer(g),windowName(windowName),delay(delay) {};
	virtual ~SimpleVisualizer() = default;
};

#endif /* VISUALIZER_H_ */
