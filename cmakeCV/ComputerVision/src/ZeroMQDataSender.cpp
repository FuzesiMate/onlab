/*
 * DataSender.cpp
 *
 *  Created on: Oct 3, 2016
 *      Author: teqbox
 */

#include "ZeroMQDataSender.h"
#include <zmq_addon.hpp>

#include <iostream>

template<typename INPUT>
void ZeroMQDataSender<INPUT>::bindAddress(std::string address){
	publisher.bind(address);
}

template<typename INPUT>
tbb::flow::continue_msg ZeroMQDataSender<INPUT>::process(INPUT data) {

	/*
	 * Publish the topic as the first part of a multi-part message
	 * The subscriber has to subscribe for this topic in order to receive the message
	 */
	
	/*
	 *The template type must implement the toJSON method which returns a string
	 *containing the JSON object that will be published
	 */
	std::string output = data.toJSON();

	if (!output.empty()){
		zmq::message_t topic_message(topic.c_str(), topic.length());
		publisher.send(topic_message, ZMQ_SNDMORE);
		zmq::message_t mesg(output.c_str(), output.length());
		publisher.send(mesg);
	}

	tbb::flow::continue_msg msg;
	return msg;
}

template<typename INPUT>
ZeroMQDataSender<INPUT>::~ZeroMQDataSender(){
	try{
		publisher.close();
		context.close();
	}catch(std::exception& e){
		std::cout << "Cannot close ZeroMQ context! Error message: " << e.what() << std::endl;
	}

}
