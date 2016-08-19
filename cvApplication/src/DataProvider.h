/*
 * DataProvider.h
 *
 *  Created on: 2016. aug. 19.
 *      Author: Máté
 */

#ifndef DATAPROVIDER_H_
#define DATAPROVIDER_H_

#include <tbb/flow_graph.h>
#include "Model.h"
#include "TemplateConfiguration.h"

template<typename CONFIG>
class DataProvider : public tbb::flow::source_node<ImageProcessingResult>{
private:
	std::atomic_bool providing;
	int64_t prevFrameIndex;
	std::shared_ptr<Model<CONFIG> > model;
public:
	bool provideData(ImageProcessingResult& data);
	DataProvider(std::shared_ptr<Model<CONFIG> > model,tbb::flow::graph& g):tbb::flow::source_node<ImageProcessingResult>(g , std::bind(&DataProvider::provideData , this , std::placeholders::_1) , false),providing(false),prevFrameIndex(0),model(model){};
	void stop();
	void start();
	virtual ~DataProvider()=default;
};

#endif /* DATAPROVIDER_H_ */
