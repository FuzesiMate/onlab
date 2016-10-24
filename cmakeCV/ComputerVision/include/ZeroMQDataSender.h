/*
 * DataSender.h
 *
 *  Created on: Oct 3, 2016
 *      Author: teqbox
 */

#ifndef ZEROMQDATASENDER_H_
#define ZEROMQDATASENDER_H_

#include "DataTypes.h"
#include "DataSender.h"
#include <zmq.hpp>

class ZeroMQDataSender: public DataSender {
private:
	zmq::context_t context;
	zmq::socket_t publisher;
	std::string topic;

	std::vector<std::string> objects;
public:
	ZeroMQDataSender(std::string topic , tbb::flow::graph& g):DataSender(g,1),context(zmq::context_t(1)),publisher(zmq::socket_t(context, ZMQ_PUB)),topic(topic){};

	void bindAddress(std::string address);

	void addObject(std::string object);

	tbb::flow::continue_msg process(ModelData modelData);

	virtual ~ZeroMQDataSender();
};

#endif /* ZEROMQDATASENDER_H_ */
