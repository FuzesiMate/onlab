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

	bool in = false;

	char c='a';

	while(c!='q'){
		cin>>c;

		switch(c){
		case 's':
			if(!in){
				cvModule.initialize("input.json");
				in = true;
				cout<<"successful init"<<endl;
			}else{
				cout<<"start processing thread..."<<endl;
				tbb_thread processingThread(std::bind(&ComputerVision::startProcessing, &cvModule));
				processingThread.detach();
			}

			break;
		case 'x':
			cvModule.stopProcessing();
			in = false;
			break;
		default:
			cout<<"invalid"<<endl;
			break;
		}
		Sleep(100);
	}

	return 0;
}
