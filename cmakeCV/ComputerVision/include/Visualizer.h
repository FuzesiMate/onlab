#pragma once

#include <tbb/flow_graph.h>
#include "Processor.h"
#include "DataTypes.h"

class Visualizer :public Processor<tbb::flow::tuple<Frame, ModelData>, tbb::flow::continue_msg, tbb::flow::queueing> 
{
public:
	Visualizer(tbb::flow::graph& g):Processor<tbb::flow::tuple<Frame, ModelData>, tbb::flow::continue_msg, tbb::flow::queueing>(g,1) {};
	
	~Visualizer()=default;
};

