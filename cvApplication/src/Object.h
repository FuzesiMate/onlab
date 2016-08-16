/*
 * Object.h
 *
 *  Created on: 2016. aug. 11.
 *      Author: Máté
 */

#ifndef OBJECT_H_
#define OBJECT_H_

#include <tbb/flow_graph.h>
#include <tbb/concurrent_vector.h>
#include <tbb/concurrent_unordered_map.h>
#include "ImageProcessor.h"
#include "Marker.h"

class Object :public tbb::flow::function_node<ImageProcessingData<tbb::concurrent_vector<cv::Point2f> , tbb::concurrent_vector<int> > , tbb::flow::continue_msg , tbb::flow::queueing> {
private:
	std::string name;
	bool tracked;
	int numberOfMarkers;
	int64_t frameIndex;
	int64_t timestamp;
	int callCounter;
	tbb::concurrent_unordered_map<std::string, std::shared_ptr<Marker> > markers;

public:
	//Object(tbb::flow::graph& g):tbb::flow::function_node<ImageProcessingData<tbb::concurrent_vector<cv::Point2f> , tbb::concurrent_vector<int> >(g , 1 , std::bind(&Object::update , this , std::placeholders::_1)){};
	Object(std::string name , int numberOfMarkers , tbb::flow::graph& g):tbb::flow::function_node<ImageProcessingData<tbb::concurrent_vector<cv::Point2f> , tbb::concurrent_vector<int> > , tbb::flow::continue_msg , tbb::flow::queueing>(g , tbb::flow::serial , std::bind(&Object::update , this , std::placeholders::_1)),name(name),tracked(false),numberOfMarkers(numberOfMarkers){callCounter=0;timestamp =0 ; frameIndex=0;};
	void update(ImageProcessingData<tbb::concurrent_vector<cv::Point2f> , tbb::concurrent_vector<int> > data);
	void addMarker(std::string name , int id );
	tbb::concurrent_vector<cv::Point2f>& getMarkerPosition(std::string name);
	std::vector<std::string> getMarkerNames();
	int64_t getFrameIndex();
	int getCallCounter();
	int64_t getTimestamp();
	virtual ~Object() = default;
};

#endif /* OBJECT_H_ */
