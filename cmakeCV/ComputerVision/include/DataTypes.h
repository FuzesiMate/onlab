
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <tbb/concurrent_vector.h>
#include <tbb/concurrent_unordered_map.h>
#include <opencv2/core/types.hpp>

#ifndef DATA_TYPES_H
#define DATA_TYPES_H

template<typename T, typename V>
using tbb_map = tbb::concurrent_unordered_map<T, V>;

template<typename T>
using tbb_vector = tbb::concurrent_vector<T>;

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

		size_t idIndex = 0;
		for (tbb::concurrent_vector<int> identifier : identifiers) {
			size_t posIndex = 0;
			for (auto& id : identifier) {
				markerPositions[id].push_back(data[idIndex][posIndex]);
			}
			idIndex++;
		}
		if (!markerPositions.empty()) {
			json << "{ \"Markers\":[";
			size_t markerIndex = 0;
			for (auto& marker : markerPositions) {
				json << "{";
				json << "\"id\":" << marker.first << ",";
				json << "\"positions\":[";
				size_t posIndex = 0;
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
	tbb::concurrent_unordered_map<std::string , MarkerData> markerData;
	uint64_t timestamp;
	uint64_t frameIndex;
	bool alive;
};

struct ModelData {
	tbb::concurrent_unordered_map<std::string , ObjectData> objectData;
	uint64_t timestamp;
	uint64_t frameIndex;

	std::string toJSON() {

		std::stringstream json;

		json << "{\"Objects\":[";

		size_t objectIndex = 0;
		for (auto& object : objectData) {
			json << "{\"name\":" << "\"" << object.second.name << "\"" << ",";
			json << "\"markers\":[";

			size_t markerIndex = 0;
			for (auto& marker : object.second.markerData) {
				json << "{";
				json << "\"name\":" << "\"" << marker.second.name << "\"" << ",";
				json << "\"realposition\":";
				json << "{" << "\"x\":" << marker.second.realPosition.x << "," << "\"y\":" << marker.second.realPosition.y << "," << "\"z\":" << marker.second.realPosition.z << "},";
				json << "\"screenpositions\":[";

				size_t positionIndex = 0;
				for (auto& screenPosition : marker.second.screenPosition) {
					json << "{";

					std::string tracked = marker.second.tracked[positionIndex] ? "true" : "false";

					json << "\"x\":" << screenPosition.x << "," << "\"y\":" << screenPosition.y << "," << "\"tracked\":" << "\"" << tracked << "\"";
					json << "}";

					if (positionIndex < marker.second.screenPosition.size() - 1) {
						json << ",";
					}

					positionIndex++;
				}

				json << "]}";

				if (markerIndex < object.second.markerData.size() - 1) {
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
