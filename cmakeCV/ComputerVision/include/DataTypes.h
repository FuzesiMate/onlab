#include <tbb/concurrent_vector.h>
#include <opencv2/core/types.hpp>
#include <string>

#ifndef DATA_TYPES_H
#define DATA_TYPES_H

struct Frame{
	tbb::concurrent_vector<cv::Mat> images;
	uint64_t timestamp;
	uint64_t frameIndex;
	int fps;
};

template <typename CONFIG> struct ImageProcessingData{
	tbb::concurrent_vector<typename CONFIG::dataType> data;
	tbb::concurrent_vector<typename CONFIG::identifierType> identifiers;
	uint64_t timestamp;
	uint64_t frameIndex;

	std::string toJSON() {
		return std::string("jsonified data");
	}
};

struct MarkerData{
	std::string name;
	tbb::concurrent_vector<bool> tracked;
	tbb::concurrent_vector<cv::Point2f> screenPosition;
	cv::Point3f realPosition;
};

struct ObjectData{
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
		return std::string("jsonified data");
	}
};

#endif /*DATA_TYPES_H*/
