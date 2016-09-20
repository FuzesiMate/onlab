

#include "DataProvider.h"
#include <thread>
#include <mutex>

tbb::flow::continue_msg DataProvider::process(MarkerPosition position){

	std::unique_lock<std::mutex> l(lock);

	dataBuffer[position.objectName].push_back(position);

	new_data.notify_one();
}

bool DataProvider::provide(ImageProcessingResult& output){

	std::unique_lock<std::mutex> l(lock);

	//wait for the corresponding object data
	new_data.wait(l, [this](){

				readyToSend = true;

				for(auto& object : dataBuffer) {

					if((object.second.begin()->frameIndex!= nextFrameIndex || dataBuffer.size()<numberOfObjects )) {
						readyToSend = false;
					}
				}

				return readyToSend;
	});

		nextFrameIndex++;

				for(auto& object : dataBuffer){

					for(auto& marker : object.second.front().position){
						output[object.first][marker.first] = marker.second;
					}
				}

				for(auto& object : dataBuffer){
					object.second.erase(object.second.begin());
				}

			readyToSend = false;

	if(providing){
		return true;
	}else{
		return false;
	}
}
