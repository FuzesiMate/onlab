

#include "ObjectDataCollector.h"

#include <thread>
#include <mutex>

tbb::flow::continue_msg ObjectDataCollector::process(ObjectData objectData){

	std::unique_lock<std::mutex> l(lock);

	dataBuffer[objectData.name].push_back(objectData);

	new_data.notify_one();

	tbb::flow::continue_msg msg;
	return msg;
}

bool ObjectDataCollector::provide(ModelData& output){

	std::unique_lock<std::mutex> l(lock);

	if(!providing){
		std::cout<<"Stop providing position data"<<std::endl;
		return false;
	}

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

				output.objectData.clear();
				for(auto& object : dataBuffer){
					output.objectData.push_back(object.second.front());
				}

				output.frameIndex = dataBuffer.begin()->second.front().frameIndex;
				output.timestamp = dataBuffer.begin()->second.front().frameIndex;

				for(auto& object : dataBuffer){
					object.second.erase(object.second.begin());
				}

			readyToSend = false;

	return true;
}
