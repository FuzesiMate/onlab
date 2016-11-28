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

int LedController::getFrameIteration(uint64_t timestamp, int setupTime) {
	if (timestamp > lastTimestamp + setupTime && timestamp < lastTimestamp + duration) {
		return iteration;
	}

	for (auto& h : history) {
		auto ts = std::get<0>(h);
		auto durat = std::get<1>(h);
		auto iter = std::get<2>(h);

		if (timestamp > ts + setupTime && timestamp < ts + durat) {
			return iter;
		}
	}

	return -1;
}


void LedController::addPattern(std::vector<int> p){
	pattern.push_back(p);
}

LedController::~LedController() {
	// TODO Auto-generated destructor stub
}

