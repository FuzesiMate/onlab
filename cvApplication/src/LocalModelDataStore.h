/*
 * LocalModelDataStore.h
 *
 *  Created on: Sep 27, 2016
 *      Author: teqbox
 */

#ifndef LOCALMODELDATASTORE_H_
#define LOCALMODELDATASTORE_H_

#include "Processor.h"
#include "DataTypes.h"
#include <atomic>


class LocalModelDataStore: public Processor<ModelData , tbb::flow::continue_msg , tbb::flow::queueing> {
	ModelData data;
public:
	LocalModelDataStore(tbb::flow::graph& g):Processor<ModelData , tbb::flow::continue_msg , tbb::flow::queueing>(g , 1){};

	ModelData getData();

	ObjectData getObjectData(std::string object);

	MarkerData getMarkerData(std::string object ,std::string marker);

	virtual ~LocalModelDataStore()=default;
};

#endif /* LOCALMODELDATASTORE_H_ */
