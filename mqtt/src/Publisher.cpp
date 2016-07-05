/*
 * publisher.cpp
 *
 *  Created on: Aug 30, 2015
 *      Author: pappi
 */

#include <string.h>
#include <iostream>
#include <stdlib.h>
#include "Publisher.h"

using namespace std;

MQTTPublisher::MQTTPublisher(const std::string& brokerUrl, const std::string& clientId) {
	MQTTClient_create(&client, brokerUrl.c_str(), clientId.c_str(), MQTTCLIENT_PERSISTENCE_NONE, NULL);
}

void MQTTPublisher::connect(MQTTClient_deliveryComplete* dC, MQTTClient_connectionLost* cL, void* context) {
	conn_options.keepAliveInterval = 20;
	MQTTClient_setCallbacks(client, context, cL, NULL, dC);
	MQTTClient_connect(client, &conn_options);
}

void MQTTPublisher::publishMessage(const std::string& topic, int qos, const void* payload) {
	MQTTClient_deliveryToken token;
	message.payload = const_cast<void*>(payload);
	message.payloadlen = strlen(static_cast<const char*>(payload));
    message.qos = qos;
    MQTTClient_publishMessage(client, topic.c_str(), &message, &token);
}

void MQTTPublisher::disconnect() {
	MQTTClient_disconnect(client, 1000L);
	MQTTClient_destroy(&client);
}
