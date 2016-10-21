/*
 * Model.cpp
 *
 *  Created on: 2016. aug. 11.
 *      Author: M�t�
 */

#include "ModelDataStore.h"

#include <tbb/parallel_for_each.h>
#include <string>
#include "TemplateConfiguration.h"

/*
 * Awful solution caused by data type refactor TODO make better
 */


tbb::flow::continue_msg ModelDataStore::process(ModelData data){

	//thread safety
	std::unique_lock<std::mutex> l(lock);

	//If it is empty, initialize the data structure
	if(modelData.objectData.empty()){
		modelData = data;
	}else{

	/*
	 * If the marker is untracked in an image, we store the last known position of the marker
	 * with the tracked flag set to false, so the user knows that it is risky information
	 */

		size_t objIndex = 0;
		for(auto& objectData : data.objectData){
			size_t markIndex = 0;
			for(auto& markerData : objectData.markerData){
				bool allTracked = true;

				size_t posIndex = 0;
				for(auto& position : markerData.screenPosition){
					if(markerData.tracked[posIndex]){
						modelData.objectData[objIndex].markerData[markIndex].screenPosition[posIndex]=position;
						modelData.objectData[objIndex].markerData[markIndex].tracked[posIndex]=true;
					}else{
						modelData.objectData[objIndex].markerData[markIndex].tracked[posIndex]=false;
						allTracked = false;
					}
					posIndex++;
				}
				if(allTracked){
					modelData.objectData[objIndex].markerData[markIndex].realPosition = markerData.realPosition;
				}
				markIndex++;
			}
			objIndex++;
		}

		modelData.frameIndex = data.frameIndex;
		modelData.timestamp = data.timestamp;
	}

	new_data.notify_one();

	tbb::flow::continue_msg msg;
	return msg;
}


ModelData ModelDataStore::getData(){

	/*
	 * if the position data was provided for a frameindex, it is not provided again
	 * this function blocks, until a new frame data arrives
	 * so the user wont get the already known data again
	 *
	 * TODO maybe i should make it configurable
	 */

	std::unique_lock<std::mutex> l(lock);

	new_data.wait(l , [this](){
		return providedFrameIndex != modelData.frameIndex;
	});

	providedFrameIndex = modelData.frameIndex;
	return modelData;
}

ObjectData ModelDataStore::getObjectData(std::string object){
	for(auto& objectData : modelData.objectData){
		if(objectData.name == object){
			return objectData;
		}
	}
}

MarkerData ModelDataStore::getMarkerData(std::string object ,std::string marker){
	for(auto& objectData : modelData.objectData){
		if(objectData.name == object){
			for(auto& markerData : objectData.markerData){
				if(markerData.name == marker){
					return markerData;
				}
			}
		}
	}
}
