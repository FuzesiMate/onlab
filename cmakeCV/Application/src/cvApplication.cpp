//============================================================================
// Name        : cvApplication.cpp
// Author      : Fuzesi MAte
// Version     :
// Copyright   : Your copyright notice
// Description : Computer Vision framework usage example in C++, Ansi-style
//============================================================================

#include <iostream>
#include "ComputerVision.h"
#include <tbb/compat/thread>
#include <tbb/tbb.h>

using namespace std;

int main(int argc , char *argv[]) {

	if (argc < 2) {
		std::cout << "No command line argument provided!" << std::endl;
		return -1;
	}

	ComputerVision cvModule;

	bool in = false;

	char c='a';

	while(c!='q'){
		cin>>c;

		switch(c){
		case 'i':
			in = cvModule.initialize(argv[1]);
			if (!in) {
				std::cout << "failed to init" << std::endl;
			}
			else {
				std::cout << "init successful" << std::endl;
			}
			break;
		case 's':
		{
			cout << "start processing thread..." << endl;

			tbb::tbb_thread processingThread(std::bind(&ComputerVision::startProcessing, &cvModule));
			processingThread.detach();
			break;
		}
		case 'x':
			cvModule.stopProcessing();
			in = false;
			break;
		case 'r':
			cvModule.reconfigure(argv[1]);
			break;
		case 'g':
		{
				ModelData posdata;
				if (cvModule.getData(posdata)) {
					for (auto& obj : posdata.objectData) {
						for (auto& mar : obj.markerData) {
							std::cout << mar.name << std::endl;
							for (auto& pos : mar.screenPosition) {
								std::cout << pos << std::endl;
							}
							std::cout << "real position: " << mar.realPosition << std::endl;

							std::cout << std::endl << std::endl;
						}
					}
				}
			break;
		}
		default:
			cout<<"invalid"<<endl;
			break;
		}
	}

	return 0;
}
