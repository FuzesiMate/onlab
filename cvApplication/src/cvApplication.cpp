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

	if(!cvModule.initialize("input.json")){
		cout<<"can not initialize computer vision module";
		return -1;
	}
	//Set up MQTT publisher
	cvModule.setupDataSender(BROKER_URL);

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
