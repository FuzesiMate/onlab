/*
 * DataProvider.cpp
 *
 *  Created on: 2016. aug. 19.
 *      Author: Máté
 */

#include "DataProvider.h"

template<typename CONFIG>
bool DataProvider<CONFIG>::provideData(ImageProcessingResult& data) {
	auto objects = model->getObjectNames();

	//do not send the same data again, wait for the next frame
	//TODO dummy solution...
	//if it looks stupid and it works, it's aint stupid ;)

	while (model->getFrameIndex(objects[0]) == prevFrameIndex) {
		Sleep(20);
	}

	prevFrameIndex++;

	for (auto& o : objects) {
		auto markers = model->getMarkerNames(o);

		for (auto& m : markers) {
			data[o][m] = model->getPosition(o, m);
		}
	}

	if (providing) {
		return true;
	} else {
		return false;
	}
}

template <typename CONFIG>
void DataProvider<CONFIG>::stop(){
	providing = false;
}

template <typename CONFIG>
void DataProvider<CONFIG>::start(){
	providing = true;
	this->activate();
}

