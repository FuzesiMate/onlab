/*
 * Object.cpp
 *
 *  Created on: 2016. aug. 11.
 *      Author: M�t�
 */

#include "Object.h"
#include <iostream>

template<typename CONFIG>
int Object<CONFIG>::getCallCounter(){
	return callCounter;
}

template<typename CONFIG>
ObjectData Object<CONFIG>::process(ImageProcessingData<CONFIG> ipData){

	ObjectData objectData;
	objectData.name = name;

	if(callCounter == limit){
				done = true;
	}else{

		for(auto m : markers){
			int id = m.second->getId();

			MarkerData markerData;
			markerData.name = m.first;
			markerData.screenPosition = tbb::concurrent_vector<cv::Point2f>(ipData.data.size());

			int i = 0;
			for(auto identifier : ipData.identifiers){
				auto index = std::find(identifier.begin() , identifier.end() , id);

				if(index!=identifier.end()){
					auto posIndex = std::distance(identifier.begin() , index);
					markerData.screenPosition[i]=(ipData.data[i][posIndex]);
					markerData.tracked.push_back(true);
				}else{
					markerData.tracked.push_back(false);
				}

				i++;
			}

			objectData.markerData.push_back(markerData);
		}

		callCounter++;
	}

	objectData.frameIndex = ipData.frameIndex;
	objectData.timestamp = ipData.timestamp;

	return objectData;
}


template<typename CONFIG>
std::vector<std::string> Object<CONFIG>::getMarkerNames(){
	std::vector<std::string> names;
	for(auto m : markers){
		names.push_back(m.first);
	}
	return names;
}

template<typename CONFIG>
void Object<CONFIG>::addMarker(std::string name , int id){
	markers[name] = std::shared_ptr<Marker>(new Marker(name, id));
}

template <typename CONFIG>
MarkerType Object<CONFIG>::getMarkerType(){
	return markerType;
}

template <typename CONFIG>
bool Object<CONFIG>::isDone(){
	return done;
}

template <typename CONFIG>
void Object<CONFIG>::remove(){
	removed = true;
}

template<typename CONFIG>
bool Object<CONFIG>::isRemoved(){
	return removed;
}


