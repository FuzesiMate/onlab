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
			for (auto& markerData : objectData.second.markerData) {

				if (markerData.second.realPosition.x != 0 && markerData.second.realPosition.y != 0 && markerData.second.realPosition.z != 0) {
					modelData.objectData[objectData.first].markerData[markerData.first].realPosition = markerData.second.realPosition;
				}

				int positionIndex = 0;
				for (auto& position : markerData.second.screenPosition) {
					
					if (modelData.objectData[objectData.first].markerData[markerData.first].tracked[positionIndex]) {
						modelData.objectData[objectData.first].markerData[markerData.first].screenPosition[positionIndex] = position;
						modelData.objectData[objectData.first].markerData[markerData.first].tracked[positionIndex] = true;
					}	
					
					positionIndex++;
				}
			}
		}

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

/*
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
*/
