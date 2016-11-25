/*
 * LedController.cpp
 *
 *  Created on: 2016. aug. 1.
 *      Author: M�t�
 */

#include <memory>
#include <chrono>
#include <string>

#include "LedController.h"


LedController::LedController() {
	serial = std::unique_ptr<SerialComm>(new SerialComm());

	if(serial->IsConnected()){
		std::cout<<"connected to arduino"<<std::endl;
	}
	iteration = 0;
	lastTimestamp = 0;
	duration = 0 ;
}

void LedController::flashNext(uint64_t timestamp , uint64_t nextDuration){

		history.push_back(std::make_tuple(lastTimestamp.load() , duration.load() , iteration.load()));

		auto currentPattern = pattern[iteration];

		int i = 1;
		for(auto v : currentPattern){
			std::stringstream str;
			str<<i<<" "<<v<<"\n";
			serial->writeData(str.str().c_str() , str.str().length());
			i++;
		}

		auto time = std::chrono::steady_clock::now();
		auto ts = std::chrono::duration_cast<std::chrono::milliseconds>(time.time_since_epoch()).count();

		lastTimestamp = ts;
		duration = nextDuration;

		if(iteration == pattern.size()-1){
			iteration = 0;
		}else{
			iteration++;
		}
}


void LedController::addPattern(std::vector<int> p){
	pattern.push_back(p);
}

uint64_t LedController::getLastTimestamp(){
	return lastTimestamp;
}

uint64_t LedController::getDuration(){
	return duration;
}

int LedController::getIteration(){
	return iteration;
}

LedController::~LedController() {
	// TODO Auto-generated destructor stub
}

