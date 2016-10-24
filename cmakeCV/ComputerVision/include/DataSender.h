/*
 * DataSender.h
 *
 *  Created on: Oct 3, 2016
 *      Author: teqbox
 */

#ifndef DATASENDER_H_
#define DATASENDER_H_

#include "Processor.h"

class DataSender: public Processor<ModelData , tbb::flow::continue_msg , tbb::flow::queueing> {
public:
	DataSender(tbb::flow::graph& g , int concurrency):Processor<ModelData, tbb::flow::continue_msg , tbb::flow::queueing>(g , concurrency){};

	virtual tbb::flow::continue_msg process(ModelData modelData) = 0;

	virtual void addObject(std::string object) = 0;

	virtual ~DataSender() = default;
};

#endif /* DATASENDER_H_ */
