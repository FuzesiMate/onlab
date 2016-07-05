//============================================================================
// Name        : CvDataSubscriber.cpp
// Author      : Fuzesi Mate
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "Subscriber.h"
#include "MQTTAsync.h"
using namespace std;

#define BROKER_URL 		"127.0.0.1"
#define CLIENT_ID 		"cv_sub"
#define DATA_TOPIC		"cv/data"


int messageArrived(void *context, char *topicName, int topicLen, MQTTAsync_message *message){

	std::string payload((char*)message->payload);

	stringstream ss ;
	ss<< payload;

	try{
	boost::property_tree::ptree pt;
	boost::property_tree::read_json(ss, pt);

	auto objects = pt.get_child("objects");

	for(auto o : objects){
		cout<<o.second.get<std::string>("name")<<endl;
	}
	}catch(exception &e){
		cout<<"exception"<<endl;
	}

	MQTTAsync_freeMessage(&message);
	MQTTAsync_free(topicName);
	return 1;
}

void connlost(void *context, char *cause){
	cout<<"data connection lost!!"<<endl;
}


int main() {
	MQTTSubscriber subscriber(BROKER_URL , CLIENT_ID);

	subscriber.subscribeTopic(DATA_TOPIC , 0 , (MQTTClient_messageArrived*)&messageArrived , (MQTTClient_connectionLost*)&connlost , NULL);

	cout<<"connected to broker!"<<endl;
	while(1){

	}

	return 0;
}
