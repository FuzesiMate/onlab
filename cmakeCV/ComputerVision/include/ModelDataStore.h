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

class ModelDataStore:public Processor<ModelData, ModelData , tbb::flow::queueing> {
private:

	//thread safety
	std::mutex lock;

	/*
	stores an up to date instance of ModelData
	*/
	ModelData modelData;

public:
	ModelDataStore(tbb::flow::graph& g):Processor<ModelData, ModelData , tbb::flow::queueing>(g ,1){};

	ModelData process(ModelData data);

	ModelData getData();
	ObjectData getObjectData(std::string object);
	MarkerData getMarkerData(std::string object ,std::string marker);

	virtual ~ModelDataStore() = default;
};

#endif /* MODELDATASTORE_H_ */
