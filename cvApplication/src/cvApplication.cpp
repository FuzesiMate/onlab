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
#include <tbb/flow_graph.h>
#include "tbb/compat/thread"
#include <tbb/tbb.h>
#include <opencv2/videoio.hpp>

using namespace std;

#define BROKER_URL 		"127.0.0.1:1883"
#define CV_DATA_TOPIC	"cv/data"

using namespace tbb;

int main(int argc , char *argv[]) {

	ComputerVision cvModule;

	if(!cvModule.initialize(argv[1])){
		cout<<"failed to init"<<endl;
		return -1;
	}

	cout<<"successful init"<<endl;

	char c='a';

	while(c!='q'){
		cin>>c;

		switch(c){
		case 's':
			if(!cvModule.isProcessing()){
				cout<<"start processing thread..."<<endl;
				tbb_thread processingThread(std::bind(&ComputerVision::startProcessing, &cvModule));

			}
			break;
		case 'x':
			cvModule.stopProcessing();
			break;
		default:
			cout<<"invalid"<<endl;
			break;
		}
	}

	//cvModule.startProcessing();

/*
	if(argc==2){
		if(!cvModule.initialize(argv[1])){
			cout<<"Problem occured while initializing"<<endl;
			return -1;
		}
	}else{
		cout<<"Command line argument is missing!"<<endl;
		return -1;
	}

	//Set up MQTT publisher

	cout<<"Setting up MQTT..."<<endl;
	cvModule.setupDataSender(BROKER_URL);

	cout<<"Start capturing and processing"<<endl;

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
	*/

	return 0;
}
