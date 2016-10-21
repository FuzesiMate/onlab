

#include "ObjectDataCollector.h"

#include <thread>
#include <mutex>

tbb::flow::continue_msg ObjectDataCollector::process(ObjectData objectData){

	std::unique_lock<std::mutex> l(lock);

	dataBuffer[objectData.name].push_back(objectData);

	new_data.notify_all();

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
	new_data.wait_for(l, std::chrono::milliseconds(1000), [this](){

				readyToSend = true;

				for(auto& object : dataBuffer) {
					if (object.second.size() > 0) {
						if ((object.second[0].frameIndex != nextFrameIndex || dataBuffer.size() < numberOfObjects)) {
							readyToSend = false;
						}
					}
				}

				return readyToSend;
	});

		nextFrameIndex++;

				output.objectData.clear();
				for(auto& object : dataBuffer){
					if (object.second.size() > 0) {
						output.objectData.push_back(object.second[0]);
						output.frameIndex = object.second[0].frameIndex;
						output.timestamp = object.second[0].timestamp;
					}
				}

				

				for(auto& object : dataBuffer){
					if (object.second.size() > 0) {
						object.second.erase(object.second.begin());
					}
				}

			readyToSend = false;

	return true;
}
