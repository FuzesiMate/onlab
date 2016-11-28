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
	int getFrameIteration(uint64_t timestamp, int setupTime);

	virtual ~LedController();
};

#endif /* LEDCONTROLLER_H_ */
