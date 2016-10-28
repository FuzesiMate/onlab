/*
 * Visualizer.h
 *
 *  Created on: 2016. aug. 19.
 *      Author: M�t�
 */

#ifndef SIMPLEVISUALIZER_H_
#define SIMPLEVISUALIZER_H_

#include <tbb/flow_graph.h>
#include "DataTypes.h"
#include "Visualizer.h"

class SimpleVisualizer: public Visualizer{
private:
	std::vector<ModelData> dataBuffer;
	std::vector<Frame> frameBuffer;
	std::string windowName;
	int64_t delay;
public:
	
	SimpleVisualizer(std::string windowName, int64_t delay ,tbb::flow::graph& g):Visualizer(g),windowName(windowName),delay(delay) {};

	tbb::flow::continue_msg process(tbb::flow::tuple<Frame, ModelData> data);

	SimpleVisualizer(SimpleVisualizer& vis) = delete;

	virtual ~SimpleVisualizer() = default;
};

#endif /* VISUALIZER_H_ */
