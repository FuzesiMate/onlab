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
#include <fstream>
#include <condition_variable>
#include "Processor.h"
#include "Provider.h"
#include "DataTypes.h"
#include <tbb/flow_graph.h>

using collector_node =  tbb::flow::multifunction_node<ObjectData, tbb::flow::tuple<ModelData, tbb::flow::continue_msg>, tbb::flow::queueing >;

//#define LOG

class ObjectDataCollector {
private:
	//Multifunction node
	collector_node node;
	//buffer to store object data
	std::map<std::string , std::vector<ObjectData> > dataBuffer;
	std::atomic<uint64_t> nextFrameIndex;
	size_t numberOfObjects;
	int64_t lastTimestamp;
#ifdef LOG
	
	std::ofstream ofs;
#endif // LOG

public:
	void ObjectDataCollector::process(ObjectData objectData, collector_node::output_ports_type& output);
	ObjectDataCollector(int numberOfObjects, tbb::flow::graph& g)
		:node(g , 1 , std::bind(&ObjectDataCollector::process, this , std::placeholders::_1 , std::placeholders::_2)),
		nextFrameIndex(0),
		numberOfObjects(numberOfObjects), 
		lastTimestamp(0){
#ifdef LOG
		ofs.open("process.csv");
		ofs << "delay;current fps" << std::endl;
#endif
	};

	collector_node& getCollectorNode() {
		return node;
	}
	virtual ~ObjectDataCollector() = default;
};

#endif /* OBJECTDATACOLLECTOR_H_ */
