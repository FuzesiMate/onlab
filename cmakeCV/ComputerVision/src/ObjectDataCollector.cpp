

#include "ObjectDataCollector.h"

#include <thread>
#include <mutex>

void ObjectDataCollector::process(ObjectData objectData, CollectorNode::output_ports_type& output){

	//std::unique_lock<std::mutex> l(lock);

	dataBuffer[objectData.name].push_back(objectData);

	

	bool readyToSend = true;

	for (auto& object : dataBuffer) {
		if (object.second.empty()) {
			readyToSend = false;
		}
		else {
			if (((object.second[0].frameIndex != nextFrameIndex) || dataBuffer.size() < numberOfObjects) && object.second[0].alive) {
				readyToSend = false;
			}
		}
		
	}

	if (readyToSend) {
		nextFrameIndex++;

		tbb::concurrent_vector<ObjectData> objectPosition;

		for (auto& object : dataBuffer) {
			objectPosition.push_back(object.second[0]);
		}

		ModelData modelData;

		modelData.objectData = objectPosition;
		modelData.frameIndex = dataBuffer.begin()->second[0].frameIndex;
		modelData.timestamp = dataBuffer.begin()->second[0].timestamp;

		std::get<0>(output).try_put(modelData);

		tbb::flow::continue_msg msg;
		std::get<1>(output).try_put(msg);

		for (auto& object : dataBuffer) {

			if (!object.second.empty()) {
				object.second.erase(object.second.begin());
			}
		}
		
	}
}
