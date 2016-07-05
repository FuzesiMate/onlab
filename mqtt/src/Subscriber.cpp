/*
 * subscriber.cpp
 *
 *  Created on: Sep 29, 2015
 *      Author: pappi
 */

#include "Subscriber.h"
#include <MQTTClient.h>
#include <MQTTClientPersistence.h>

MQTTSubscriber::MQTTSubscriber(const std::string& brokerUrl, const std::string& clientId) {
	MQTTClient_create(&client, brokerUrl.c_str(), clientId.c_str(), MQTTCLIENT_PERSISTENCE_NONE, NULL);
}

void MQTTSubscriber::subscribeTopic(const std::string& topic, int qos, MQTTClient_messageArrived* mA, MQTTClient_connectionLost* cL, void* context) {
	MQTTClient_setCallbacks(client , context, cL, mA, NULL);
	MQTTClient_connect(client, &conn_options);
	MQTTClient_subscribe(client, topic.c_str(), qos);
}

void MQTTSubscriber::disconnect() {
	MQTTClient_disconnect(client, 1000L);
	MQTTClient_destroy(&client);
}
