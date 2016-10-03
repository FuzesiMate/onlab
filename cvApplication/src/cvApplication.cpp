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
#include <tbb/compat/thread>
#include <tbb/tbb.h>
#include <opencv2/videoio.hpp>
#include <zmq.hpp>
#include <zmq_addon.hpp>

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
				in = cvModule.initialize(argv[1]);
				if(in){
					std::cout<<"init successful!"<<std::endl;
				}

			}else{
				cout<<"start processing thread..."<<endl;

				/*
				tbb_thread temp([&](){
					while(true){


					if(cvModule.isProcessing()){
							auto posdata = cvModule.getData();
							std::cout<<"frameIndex: "<<posdata.frameIndex<<std::endl;

							for(auto& obj : posdata.objectData){
															for(auto& mar : obj.markerData){
																std::cout<<mar.name<<std::endl;
																for(auto& pos : mar.screenPosition){
																	std::cout<<pos<<std::endl;
																}
																std::cout<<"real position: "<< mar.realPosition<<std::endl;

																std::cout<<std::endl<<std::endl;
															}
														}

							}

					}
				});


				temp.detach();
*/

				tbb_thread processingThread(std::bind(&ComputerVision::startProcessing, &cvModule));
				processingThread.detach();
			}

			break;
		case 'x':
			cvModule.stopProcessing();
			in = false;
			break;
		case 'r':
			cvModule.reconfigure(argv[1]);
			break;
		case 'g':
		{
			auto posdata = cvModule.getData();
			for(auto& obj : posdata.objectData){
				for(auto& mar : obj.markerData){
					std::cout<<mar.name<<std::endl;
					for(auto& pos : mar.screenPosition){
						std::cout<<pos<<std::endl;
					}
					std::cout<<"real position: "<< mar.realPosition<<std::endl;

					std::cout<<std::endl<<std::endl;
				}
			}
			break;
		}
		default:
			cout<<"invalid"<<endl;
			break;
		}
		//usleep(100000);
	}

	return 0;
}
