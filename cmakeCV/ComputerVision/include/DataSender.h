/*
 * DataSender.h
 *
 *  Created on: Oct 3, 2016
 *      Author: teqbox
 */

#ifndef DATASENDER_H_
#define DATASENDER_H_

#include "Processor.h"

template <typename INPUT>
class DataSender: public Processor<INPUT , tbb::flow::continue_msg , tbb::flow::queueing> {
public:
	DataSender(tbb::flow::graph& g , int concurrency):Processor<INPUT, tbb::flow::continue_msg , tbb::flow::queueing>(g , concurrency){};

	virtual tbb::flow::continue_msg process(INPUT modelData) = 0;

	//virtual void addReference(std::string reference) = 0;

	virtual ~DataSender() = default;
};

#endif /* DATASENDER_H_ */
