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

class ObjectDataCollector: public Processor<ObjectData , tbb::flow::continue_msg , tbb::flow::queueing> ,public Provider<ModelData> {
private:
	//wait-notify pattern variables
	std::mutex lock;
	std::condition_variable new_data;

	//buffer to store object data
	std::map<std::string , std::vector<ObjectData> > dataBuffer;
	std::atomic<uint64_t> nextFrameIndex;
	size_t numberOfObjects;
public:
	tbb::flow::continue_msg process(ObjectData position);

	bool provide(ModelData& output);

	ObjectDataCollector(int numberOfObjects, tbb::flow::graph& g):Processor<ObjectData , tbb::flow::continue_msg , tbb::flow::queueing>(g ,1),
			Provider<ModelData>(g),nextFrameIndex(0),numberOfObjects(numberOfObjects){};
	virtual ~ObjectDataCollector() = default;
};

#endif /* OBJECTDATACOLLECTOR_H_ */
