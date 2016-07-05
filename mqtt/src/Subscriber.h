/*
 * subscriber.hpp
 *
 *  Created on: Sep 29, 2015
 *      Author: pappi
 */

#ifndef SUBSCRIBER_H_
#define SUBSCRIBER_H_

#include <string>

#include "MQTTClient.h"
#include "MQTTClientPersistence.h"

class MQTTSubscriber {
private:
	MQTTClient client;
	MQTTClient_connectOptions conn_options = MQTTClient_connectOptions_initializer;
	MQTTClient_message message = MQTTClient_message_initializer;
public:
	MQTTSubscriber(const std::string& brokerUrl, const std::string& clientId);
	void subscribeTopic(const std::string& topic, int qos, MQTTClient_messageArrived* mA, MQTTClient_connectionLost* cL, void* context = nullptr);
	void disconnect();
};

#endif /* SUBSCRIBER_H_ */
