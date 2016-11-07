/*
 * ObjectDataProvider.h
 *
 *  Created on: 2016. aug. 24.
 *      Author: M�t�
 */

#ifndef OBJECTDATACOLLECTOR_H_
#define OBJECTDATACOLLECTOR_H_

#include <mutex>
#include <map>
#include <condition_variable>
#include "Processor.h"
#include "Provider.h"
#include "DataTypes.h"
#include <tbb/flow_graph.h>

typedef tbb::flow::multifunction_node<ObjectData, tbb::flow::tuple<ModelData, tbb::flow::continue_msg>, tbb::flow::queueing > CollectorNode;

class ObjectDataCollector {
private:

	//Multifunction node
	CollectorNode collectorNode;

	//buffer to store object data
	std::map<std::string , std::vector<ObjectData> > dataBuffer;
	std::atomic<uint64_t> nextFrameIndex;
	size_t numberOfObjects;
public:

	void ObjectDataCollector::process(ObjectData objectData, CollectorNode::output_ports_type& output);

	//bool provide(ModelData& output);

	ObjectDataCollector(int numberOfObjects, tbb::flow::graph& g)
		:collectorNode(g , 1 , std::bind(&ObjectDataCollector::process, this , std::placeholders::_1 , std::placeholders::_2)),nextFrameIndex(0),numberOfObjects(numberOfObjects){};

	CollectorNode& getCollectorNode() {
		return collectorNode;
	}

	virtual ~ObjectDataCollector() = default;
};

#endif /* OBJECTDATACOLLECTOR_H_ */
