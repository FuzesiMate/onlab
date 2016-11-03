

#include "ObjectDataCollector.h"

#include <thread>
#include <mutex>

void ObjectDataCollector::process(ObjectData objectData, CollectorNode::output_ports_type& output){

	//std::unique_lock<std::mutex> l(lock);

	dataBuffer[objectData.name].push_back(objectData);

	

	bool readyToSend = true;

	for (auto& object : dataBuffer) {
		
		 if(((object.second.front().frameIndex != nextFrameIndex) || dataBuffer.size() < numberOfObjects) && object.second.front().alive) {
			readyToSend = false;
		}
	}

	if (readyToSend) {
		nextFrameIndex++;

		//std::cout << "object data sent out" << std::endl;

		tbb::concurrent_vector<ObjectData> objectPosition;

		for (auto& object : dataBuffer) {
			objectPosition.push_back(object.second.front());
		}

		ModelData modelData;

		modelData.objectData = objectPosition;
		modelData.frameIndex = dataBuffer.begin()->second.front().frameIndex;
		modelData.timestamp = dataBuffer.begin()->second.front().timestamp;

		std::get<0>(output).try_put(modelData);

		tbb::flow::continue_msg msg;
		std::get<1>(output).try_put(msg);

		for (auto& object : dataBuffer) {

			if (!object.second.empty()) {
				object.second.erase(object.second.begin());
			}
		}
		
	}

	//std::cout << "object data arrived: " <<objectData.name<< std::endl;

	/*
	new_data.notify_all();

	tbb::flow::continue_msg msg;
	return msg;
	*/
}

/*
bool ObjectDataCollector::provide(ModelData& output) {

	std::unique_lock<std::mutex> l(lock);

	if (!providing) {
		std::cout << "Stop providing position data" << std::endl;
		return false;
	}

	//wait for the corresponding object data
	new_data.wait(l, [this]() {

		bool readyToSend = true;

		for (auto& object : dataBuffer) {
					if ((object.second.front().frameIndex != nextFrameIndex) || dataBuffer.size() < numberOfObjects || object.second.empty() || !providing) {
						readyToSend = false;
					}
		}

		return readyToSend;
	});

	nextFrameIndex++;

	//std::cout << "object data sent out" << std::endl;

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

		if (!object.second.front().alive) {
			dataBuffer.erase(object.second.front().name);
			numberOfObjects--;
		}
	}

	return true;
}
*/