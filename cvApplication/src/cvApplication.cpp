//============================================================================
// Name        : cvApplication.cpp
// Author      : Fuzesi MAte
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <opencv2/highgui.hpp>
#include <signal.h> //For proper close ofterminal console
#include "ros/ros.h"
#include "ComputerVision.h"

using namespace std;

#define BROKER_URL 		"127.0.0.1:1883"
#define CV_DATA_TOPIC	"cv/data"

ros::Rate* loop_rate = nullptr;

void quit(int sig)
{
	if(loop_rate != nullptr) delete loop_rate;
  exit(0);
}

int main(int argc , char **argv) {
	signal(SIGINT,quit);
	ComputerVision cvModule;

	cout<<"init"<<endl;
	if(!cvModule.initialize("doksiba.json")){
		cout<<"can not initialize computer vision module"<<endl;
	}

	cout<<"init done"<<endl;
	//Set up MQTT publisher

	cout<<"datasender"<<endl;
	cvModule.setupDataSender(BROKER_URL);

	cout<<"datasender set"<<endl;

	while(true){
		//Capture stereo frame

		cvModule.captureFrame();

		//Process the captured frames

		cvModule.processCurrentFrame();

		//Show video stream

		cvModule.showImage();

		//send data over MQTT
		//cvModule.sendData(CV_DATA_TOPIC);

		//send data over ROS		
		ROSNode::Node nodeObj;
		nodeObj.initializeNode();

		//ros::Rate creatable only after ros::init()
		if(loop_rate == nullptr) loop_rate = new ros::Rate(10);
		nodeObj.sendDataOverROS(cvModule);

		//Sending message with 10 H
		loop_rate.sleep();
	}

	return 0;
}
