/*
 * Object.cpp
 *
 *  Created on: 2016. aug. 11.
 *      Author: Máté
 */

#include "Object.h"

template<typename CONFIG>
int Object<CONFIG>::getCallCounter(){
	return callCounter;
}

template<typename CONFIG>
void Object<CONFIG>::update(ImageProcessingData<CONFIG> data){
	callCounter++;

	if(callCounter == limit){
			done = true;
	}

	for(auto m : markers){
		int id = m.second->getId();
		bool found = true;

		tbb::concurrent_vector<cv::Point2f> position;

		int i = 0 ;
		for(auto identifier : data.identifiers){

			auto index = std::find(identifier.begin() , identifier.end() , id);

			if(index==identifier.end()){
				found = false;
			}else{
				auto posIndex = std::distance(identifier.begin() , index);
				position.push_back(data.data[i][posIndex]);
			}
			i++;
		}

		if(found){
			m.second->setPosition(position);
		}else{
			m.second->lost();

		}
	}

	frameIndex = data.frameIndex;
}

template<typename CONFIG>
tbb::concurrent_vector<cv::Point2f>& Object<CONFIG>::getMarkerPosition(std::string name){
	return markers[name]->getPosition();
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
int64_t Object<CONFIG>::getFrameIndex(){
	return frameIndex;
}

template<typename CONFIG>
int64_t Object<CONFIG>::getTimestamp(){
	return timestamp;
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


