/*
 * DataSender.cpp
 *
 *  Created on: Oct 3, 2016
 *      Author: teqbox
 */

#include "ZeroMQDataSender.h"
#include <zmq_addon.hpp>

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
	std::stringstream output;

	for (auto& object : modelData.objectData) {
		if (std::find(objects.begin(), objects.end(), object.name) != objects.end()) {
			output << "name:" << object.name;
			for (auto& marker : object.markerData) {

				output << "screenposition:";
				for (auto i = 0; i < marker.screenPosition.size(); i++) {
					output << "X:" << marker.screenPosition[i].x << "y:" << marker.screenPosition[i].y;
					output << "tracked:" << marker.tracked[i] ? "true" : "false";
				}
			}
		}
	}

	zmq::message_t mesg(output.str().c_str() , output.str().length());
	publisher.send(mesg);

	tbb::flow::continue_msg msg;
	return msg;
}

void ZeroMQDataSender::addObject(std::string object) {
	objects.push_back(object);
}


ZeroMQDataSender::~ZeroMQDataSender(){
	try{
		publisher.close();
		context.close();
	}catch(std::exception& e){

	}

}
