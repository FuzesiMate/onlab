/*
 * ObjectDataProvider.h
 *
 *  Created on: 2016. aug. 24.
 *      Author: Máté
 */

#ifndef DATAPROVIDER_H_
#define DATAPROVIDER_H_

#include "Processor.h"
#include "Provider.h"
#include "TemplateConfiguration.h"

class DataProvider: public Processor<MarkerPosition , tbb::flow::continue_msg , tbb::flow::queueing> ,public Provider<ImageProcessingResult> {
private:
	tbb::concurrent_unordered_map<std::string , tbb::concurrent_vector<MarkerPosition> > dataBuffer;
	std::atomic<int64_t> nextFrameIndex;
	std::atomic_bool readyToSend;
	int numberOfObjects;
public:
	tbb::flow::continue_msg process(MarkerPosition position);

	bool provide(ImageProcessingResult& output);

	DataProvider(int numberOfObjects, tbb::flow::graph& g):Processor<MarkerPosition , tbb::flow::continue_msg , tbb::flow::queueing>(g ,1),
			Provider<ImageProcessingResult>(g),nextFrameIndex(0),readyToSend(false),numberOfObjects(numberOfObjects){};
	virtual ~DataProvider() = default;
};

#endif /* DATAPROVIDER_H_ */
