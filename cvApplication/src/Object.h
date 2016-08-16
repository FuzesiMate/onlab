/*
 * Object.h
 *
 *  Created on: 2016. aug. 11.
 *      Author: M�t�
 */

#ifndef OBJECT_H_
#define OBJECT_H_

#include <tbb/flow_graph.h>
#include <tbb/concurrent_vector.h>
#include <tbb/concurrent_unordered_map.h>
#include "ImageProcessor.h"
#include "Marker.h"

template <typename CONFIG>
class Object :public tbb::flow::function_node<ImageProcessingData<typename CONFIG::dataType ,typename CONFIG::identifierType > , tbb::flow::continue_msg , tbb::flow::queueing> {
private:
	std::string name;
	std::atomic_bool tracked;
	int numberOfMarkers;
	std::atomic<int64_t> frameIndex;
	std::atomic<int64_t> timestamp;
	std::atomic_int callCounter;
	tbb::concurrent_unordered_map<std::string, std::shared_ptr<Marker> > markers;

public:
	Object(std::string name , int numberOfMarkers , tbb::flow::graph& g):tbb::flow::function_node<ImageProcessingData<typename CONFIG::dataType , typename CONFIG::identifierType > , tbb::flow::continue_msg , tbb::flow::queueing>(g , tbb::flow::serial , std::bind(&Object::update , this , std::placeholders::_1)),
			name(name),tracked(false),numberOfMarkers(numberOfMarkers),frameIndex(0),timestamp(0),callCounter(0){};

	void update(ImageProcessingData<typename CONFIG::dataType ,typename CONFIG::identifierType > data);
	void addMarker(std::string name , int id );
	tbb::concurrent_vector<cv::Point2f>& getMarkerPosition(std::string name);
	std::vector<std::string> getMarkerNames();
	int64_t getFrameIndex();
	int getCallCounter();
	int64_t getTimestamp();
	virtual ~Object() = default;
};

#endif /* OBJECT_H_ */
