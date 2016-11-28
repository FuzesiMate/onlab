//============================================================================
// Name        : cvApplication.cpp
// Author      : Fuzesi MAte
// Version     :
// Copyright   : Your copyright notice
// Description : Computer Vision framework usage example in C++, Ansi-style
//============================================================================

#include <iostream>
#include "ComputerVision.h"
#include <iostream>
#include <fstream>
#include <tbb/compat/thread>
#include <tbb/tbb.h>

using namespace std;

int main(int argc , char *argv[]) {

	if (argc < 2) {
		std::cout << "No command line argument!" << std::endl;
		return -1;
	}

	ComputerVision cvModule;

	std::ofstream out;

	char c='a';

	while(c!='q'){
		cin>>c;
		switch(c){
		case 's':
		{
			std::cout << "Initializing ComputerVision..."<<std::endl;

			if (!cvModule.initialize(argv[1])) {
				std::cout << "Failed to init!" << std::endl;
			}
			else {
				std::cout << "Init successful!" << std::endl;

				cout << "start processing thread..." << endl;

				tbb::tbb_thread processingThread(std::bind(&ComputerVision::startProcessing, &cvModule));
				processingThread.detach();
			}
			break;
		}
		case 'x':
			cvModule.stopProcessing();
			break;
		case 'r':
			cvModule.reconfigure(argv[1]);
			break;
		case 'g':
		{
				ModelData posdata;
				if (cvModule.getData(posdata)) {
					out.open("ex.json");
					out<<posdata.toJSON();
					out.close();
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
