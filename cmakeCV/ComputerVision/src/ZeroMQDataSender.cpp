/*
 * DataSender.cpp
 *
 *  Created on: Oct 3, 2016
 *      Author: teqbox
 */

#include "ZeroMQDataSender.h"

#include <iostream>

void ZeroMQDataSender::bindAddress(std::string address){
	publisher.bind(address);
}

tbb::flow::continue_msg ZeroMQDataSender::process(ModelData modelData){

	/*
	 * Publish the topic as the first part of a multi-part message
	 * The subscriber has to subscribe for this topic in order to receive the message
	 */
	zmq::message_t topic_message(topic.c_str() , topic.length());
	publisher.send(topic_message , ZMQ_SNDMORE);

	/*
	 * Publish the actual data
	 */
	zmq::message_t mesg("uzenet" , 6);
	publisher.send(mesg);

	tbb::flow::continue_msg msg;
	return msg;
}


ZeroMQDataSender::~ZeroMQDataSender(){
	try{
		publisher.close();
		context.close();
	}catch(std::exception& e){

	}

}
