

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

bool ObjectDataCollector::provide(ModelData& output) {

	std::unique_lock<std::mutex> l(lock);

	if (!providing) {
		std::cout << "Stop providing position data" << std::endl;
		return false;
	}

	//wait for the corresponding object data
	new_data.wait(l, [this]() {

		readyToSend = true;

		for (auto& object : dataBuffer) {
				if ((object.second.front().frameIndex != nextFrameIndex || dataBuffer.size() < numberOfObjects)) {
					readyToSend = false;
			}
		}

		return readyToSend;
	});

	nextFrameIndex++;

	tbb::concurrent_vector<ObjectData> objectPosition;

	for (auto& object : dataBuffer) {
		objectPosition.push_back(object.second.front());
	}

	output.objectData = objectPosition;
	output.frameIndex = dataBuffer.begin()->second.front().frameIndex;
	output.timestamp  = dataBuffer.begin()->second.front().timestamp;

	for(auto& object : dataBuffer){
		if (!object.second.empty()) {
			object.second.erase(object.second.begin());
		}
	}
			readyToSend = false;

	return true;
}
