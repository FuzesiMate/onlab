/*
 * ObjectDataProvider.cpp
 *
 *  Created on: 2016. aug. 24.
 *      Author: Máté
 */

#include "DataProvider.h"


#include "windows.h"

tbb::flow::continue_msg DataProvider::process(MarkerPosition position){
	dataBuffer[position.objectName].push_back(position);
}

bool DataProvider::provide(ImageProcessingResult& output){

	if(providing){

		while(!readyToSend){

			readyToSend = true;

			for(auto& element : dataBuffer){

				if((element.second.begin()->frameIndex!= nextFrameIndex || dataBuffer.size()<numberOfObjects )){
					readyToSend = false;
				}
			}

			Sleep(10);
			//std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}

		nextFrameIndex++;

				for(auto& object : dataBuffer){
					for(auto& marker : object.second.front().position){
						output[object.first][marker.first] = marker.second;
					}
				}

			for(auto& object : dataBuffer){
				auto pos = object.second;
				object.second.clear();
				for(size_t i = 1 ; i<pos.size() ; i++){
					object.second.push_back(pos[i]);
				}
			}

			readyToSend = false;

			return true;
	}else{
		return false;
	}
}
