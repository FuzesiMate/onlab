//============================================================================
// Name        : cvApplication.cpp
// Author      : Fuzesi MAte
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <opencv2/highgui.hpp>
#include "ComputerVision.h"

using namespace std;

#define BROKER_URL 		"127.0.0.1:1883"
#define CV_DATA_TOPIC	"cv/data"

int main(int argc , char **argv) {

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
		cvModule.sendData(CV_DATA_TOPIC);
	}

	return 0;
}
