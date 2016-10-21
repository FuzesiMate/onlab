/*
 * Model.h
 *
 *  Created on: 2016. aug. 11.
 *      Author: M�t�
 */

#ifndef MODELDATASTORE_H_
#define MODELDATASTORE_H_

#include <tbb/concurrent_unordered_map.h>
#include <boost/property_tree/ptree.hpp>
#include <mutex>
#include <condition_variable>
#include <tbb/flow_graph.h>
#include "Processor.h"
#include "DataTypes.h"
#include "TemplateConfiguration.h"

class ModelDataStore:public Processor<ModelData, tbb::flow::continue_msg , tbb::flow::queueing> {
private:

	//wait-notify pattern variables
	std::mutex lock;
	std::condition_variable new_data;

	ModelData modelData;
	uint64_t providedFrameIndex;

public:
	ModelDataStore(tbb::flow::graph& g):Processor<ModelData, tbb::flow::continue_msg , tbb::flow::queueing>(g ,1),providedFrameIndex(0){};

	tbb::flow::continue_msg process(ModelData data);

	//get actual collected position data
	ModelData getData();
	ObjectData getObjectData(std::string object);
	MarkerData getMarkerData(std::string object ,std::string marker);

	//default destructor
	virtual ~ModelDataStore() = default;
};

#endif /* MODELDATASTORE_H_ */
