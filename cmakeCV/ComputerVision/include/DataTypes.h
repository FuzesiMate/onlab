
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <tbb/concurrent_vector.h>
#include <opencv2/core/types.hpp>

#ifndef DATA_TYPES_H
#define DATA_TYPES_H

struct Frame {
	tbb::concurrent_vector<cv::Mat> images;
	uint64_t timestamp;
	uint64_t frameIndex;
	int fps;
};

template <typename CONFIG> struct ImageProcessingData {
	tbb::concurrent_vector<typename CONFIG::dataType> data;
	tbb::concurrent_vector<typename CONFIG::identifierType> identifiers;
	uint64_t timestamp;
	uint64_t frameIndex;

	std::string toJSON() {

		std::stringstream json;

		std::map<int, std::vector<cv::Point2f> > markerPositions;

		int idIndex = 0;
		for (tbb::concurrent_vector<int> identifier : identifiers) {
			int posIndex = 0;
			for (auto& id : identifier) {
				markerPositions[id].push_back(data[idIndex][posIndex]);
			}
			idIndex++;
		}
		if (!markerPositions.empty()) {
			json << "{ \"Markers\":[";
			int markerIndex = 0;
			for (auto& marker : markerPositions) {
				json << "{";
				json << "\"id\":" << marker.first << ",";
				json << "\"positions\":[";
				int posIndex = 0;
				for (auto position : marker.second) {
					json << "{";
					json << "\"x\":" << position.x << ",";
					json << "\"y\":" << position.y;
					json << "}";
					if (posIndex < marker.second.size() - 1) {
						json << ",";
					}
					posIndex++;
				}
				json << "]";

				json << "}";
				if (markerIndex < markerPositions.size() - 1) {
					json << ",";
				}
				markerIndex++;
			}

			json << "],";
			json << "\"timestamp\":" << timestamp << ",";
			json << "\"frameindex\":" << frameIndex;
			json << "}";
		}
		return json.str();
	}
};

struct MarkerData {
	std::string name;
	tbb::concurrent_vector<bool> tracked;
	tbb::concurrent_vector<cv::Point2f> screenPosition;
	cv::Point3f realPosition;
};

struct ObjectData {
	std::string name;
	tbb::concurrent_vector<MarkerData> markerData;
	uint64_t timestamp;
	uint64_t frameIndex;
	bool alive;
};

struct ModelData {
	tbb::concurrent_vector<ObjectData> objectData;
	uint64_t timestamp;
	uint64_t frameIndex;

	std::string toJSON() {

		std::stringstream json;

		json << "{\"Objects\":[";

		int objectIndex = 0;
		for (auto& object : objectData) {
			json << "{\"name\":" << "\"" << object.name << "\"" << ",";
			json << "\"markers\":[";

			int markerIndex = 0;
			for (auto& marker : object.markerData) {
				json << "{";
				json << "\"name\":" << "\"" << marker.name << "\"" << ",";
				json << "\"realposition\":";
				json << "{" << "\"x\":" << marker.realPosition.x << "," << "\"y\":" << marker.realPosition.y << "," << "\"z\":" << marker.realPosition.z << "},";
				json << "\"screenpositions\":[";

				int positionIndex = 0;
				for (auto& screenPosition : marker.screenPosition) {
					json << "{";

					std::string tracked = marker.tracked[positionIndex] ? "true" : "false";

					json << "\"x\":" << screenPosition.x << "," << "\"y\":" << screenPosition.y << "," << "\"tracked\":" << "\"" << tracked << "\"";
					json << "}";

					if (positionIndex < marker.screenPosition.size() - 1) {
						json << ",";
					}

					positionIndex++;
				}

				json << "]}";

				if (markerIndex < object.markerData.size() - 1) {
					json << ",";
				}

				markerIndex++;
			}

			json << "]}";

			if (objectIndex < objectData.size() - 1) {
				json << ",";
			}
			objectIndex++;
		}

		json << "],";
		json << "\"timestamp\":" << timestamp << ",";
		json << "\"frameindex\":" << frameIndex;
		json << "}";
		return json.str();
	}
};

#endif /*DATA_TYPES_H*/
