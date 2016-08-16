/*
 * Object.cpp
 *
 *  Created on: 2016. aug. 11.
 *      Author: Máté
 */

#include "Object.h"

int Object::getCallCounter(){
	return callCounter;
}

void Object::update(ImageProcessingData<tbb::concurrent_vector<cv::Point2f> , tbb::concurrent_vector<int> > data){
	callCounter++;

	bool allTracked = true;

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
			allTracked = false;
		}
	}

	//std::cout<<"update object "<<name<<std::endl;
	frameIndex = data.frameIndex;
}

tbb::concurrent_vector<cv::Point2f>& Object::getMarkerPosition(std::string name){
	return markers[name]->getPosition();
}

std::vector<std::string> Object::getMarkerNames(){
	std::vector<std::string> names;
	for(auto m : markers){
		names.push_back(m.first);
	}
	return names;
}

int64_t Object::getFrameIndex(){
	return frameIndex;
}

int64_t Object::getTimestamp(){
	return timestamp;
}

void Object::addMarker(std::string name , int id){
	markers[name] = std::shared_ptr<Marker>(new Marker(name, id));
}


