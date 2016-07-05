/*
 * publisher.hpp
 *
 *  Created on: Aug 31, 2015
 *      Author: pappi
 */

#ifndef PUBLISHER_H_
#define PUBLISHER_H_

#include <string>

#include "MQTTClient.h"
#include "MQTTClientPersistence.h"

using namespace std;

class MQTTPublisher {
private:
	MQTTClient client;
	MQTTClient_connectOptions conn_options = MQTTClient_connectOptions_initializer;
	MQTTClient_message message = MQTTClient_message_initializer;

public:
	MQTTPublisher(const std::string& brokerUrl, const std::string& clientId);
	void connect(MQTTClient_deliveryComplete* dC, MQTTClient_connectionLost* cL, void* context = nullptr);
	void publishMessage(const std::string& topic, int qos, const void* payload);
	void disconnect();
};

#endif /* PUBLISHER_H_ */
