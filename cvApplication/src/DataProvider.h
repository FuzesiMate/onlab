/*
 * ObjectDataProvider.h
 *
 *  Created on: 2016. aug. 24.
 *      Author: M�t�
 */

#ifndef DATAPROVIDER_H_
#define DATAPROVIDER_H_

#include <mutex>
#include <map>
#include <condition_variable>
#include "Processor.h"
#include "Provider.h"
#include "DataTypes.h"
#include "TemplateConfiguration.h"

class DataProvider: public Processor<ObjectData , tbb::flow::continue_msg , tbb::flow::queueing> ,public Provider<ModelData> {
private:
	//wait-notify variables
	std::mutex lock;
	std::condition_variable new_data;

	std::map<std::string , std::vector<ObjectData> > dataBuffer;
	std::atomic<int64_t> nextFrameIndex;
	bool readyToSend;
	int numberOfObjects;
public:
	tbb::flow::continue_msg process(ObjectData position);

	bool provide(ModelData& output);

	DataProvider(int numberOfObjects, tbb::flow::graph& g):Processor<ObjectData , tbb::flow::continue_msg , tbb::flow::queueing>(g ,1),
			Provider<ModelData>(g),nextFrameIndex(0),readyToSend(false),numberOfObjects(numberOfObjects){};
	virtual ~DataProvider() = default;
};

#endif /* DATAPROVIDER_H_ */
