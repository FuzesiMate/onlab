/*
 * LedController.h
 *
 *  Created on: 2016. aug. 1.
 *      Author: M�t�
 */

#ifndef LEDCONTROLLER_H_
#define LEDCONTROLLER_H_


#include <iostream>
#include <memory>
#include <sstream>
#include <atomic>
#include <tbb/concurrent_vector.h>

#include "SerialComm.h"

#include "Pattern.h"

class LedController {
private:
	std::unique_ptr<SerialComm> serial;
	std::atomic_int iteration;
	std::atomic<uint64_t> lastTimestamp;
	std::atomic<uint64_t> duration;
	std::vector<std::vector<int> > pattern;

	tbb::concurrent_vector<std::tuple<uint64_t , uint64_t , int> >  history;
public:
	LedController();
	void flashNext(uint64_t timestamp , uint64_t duration);
	void addPattern(std::vector<int> p);
	uint64_t getLastTimestamp();
	uint64_t getDuration();
	int getIteration();

	int getFrameIteration(uint64_t timestamp , int setupTime){

		if(timestamp > lastTimestamp+setupTime && timestamp < lastTimestamp+duration){

			//std::cout<<"begin: "<<lastTimestamp+setupTime<<std::endl;
			//std::cout<<"timestamp: "<<timestamp<<std::endl;


			//std::cout<<"end: "<<lastTimestamp+duration<<std::endl;

			return iteration;
		}

		for(auto& h : history){
			auto ts = std::get<0>(h);
			auto durat = std::get<1>(h);
			auto iter = std::get<2>(h);

			if(timestamp > ts+setupTime && timestamp < ts+durat){
				return iter;
			}
		}

		return -1;
	}

	virtual ~LedController();
};

#endif /* LEDCONTROLLER_H_ */
