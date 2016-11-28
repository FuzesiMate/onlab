/*
 * Model.cpp
 *
 *  Created on: 2016. aug. 11.
 *      Author: M�t�
 */

#include "ModelDataStore.h"

#include <string>
#include "TemplateConfiguration.h"


ModelData ModelDataStore::process(ModelData newData){

	//thread safety
	std::unique_lock<std::mutex> l(lock);

	//If it is empty, initialize the data structure
	if(modelData.objectData.empty()){
		modelData = newData;
	}else{

	/*
	 * if the marker is untracked in an image, it stores the last known position of the marker
	 * with the tracked flag set to false, so the user knows that this information is out of date
	 */

		for (auto& objectData : newData.objectData) {
			for (auto& markerData : objectData.markerData) {

				int positionIndex = 0;
				for (auto& position : markerData.screenPosition) {
					
					setPosition(objectData.name, markerData.name, position  , markerData.realPosition, positionIndex, markerData.tracked[positionIndex]);
					
					positionIndex++;
				}
			}
		}

		/*
		size_t objIndex = 0;
		for(auto& objectData : newData.objectData){
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
		*/

		modelData.frameIndex = newData.frameIndex;
		modelData.timestamp = newData.timestamp;
	}

	return modelData;
}


bool ModelDataStore::getData(ModelData& output) {
	std::unique_lock<std::mutex> l(lock);
	output = modelData;
	if (providedFrameIndex == modelData.frameIndex) {
		return false;
	}
	else {
		providedFrameIndex = modelData.frameIndex;
		return true;
	}
}

void ModelDataStore::setPosition(std::string objectName, std::string markerName, cv::Point2f position , cv::Point3f realPosition , int index , bool tracked) {
	for (auto& objectData : modelData.objectData) {
		if (objectData.name == objectName) {
			for (auto& markerData : objectData.markerData) {
				if (markerData.name == markerName) {
					if (tracked) {
						markerData.screenPosition[index] = position;
						markerData.realPosition = realPosition;
						markerData.tracked[index] = true;
					}
					else {
						markerData.tracked[index] = false;
					}
				}
			}
		}
	}
}
