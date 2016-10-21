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
#include <atomic>
#include "DataTypes.h"
#include "ImageProcessor.h"
#include "Marker.h"
#include "TemplateConfiguration.h"
#include "Processor.h"


template<typename CONFIG>
class Object: public Processor<ImageProcessingData<CONFIG> , ObjectData , tbb::flow::queueing >{
private:

	//name of the object provided by user in config file
	std::string name;
	//number of markers on the object
	int numberOfMarkers;
	//counts how many times the object was tried to detect
	std::atomic<int> callCounter;
	//detection limit provided by user in config file
	std::atomic<int> limit;
	//true if the object reaches it's detection limit
	std::atomic<bool> done;
	//true if the object was removed from the graph
	std::atomic<bool> removed;
	//type of the marker on the object
	std::string markerType;

	tbb::concurrent_unordered_map<std::string, std::shared_ptr<Marker> > markers;

public:
	Object(std::string name, int numberOfMarkers, std::string type, int limit,
			tbb::flow::graph& g) :Processor<ImageProcessingData<CONFIG> , ObjectData , tbb::flow::queueing >(g  , tbb::flow::unlimited), name(
					name), numberOfMarkers(numberOfMarkers), callCounter(0), limit(limit), done(false), removed(
					false), markerType(type) {};

	//Body of intel tbb function node
	ObjectData process(ImageProcessingData<CONFIG> data);

	void addMarker(std::string name, int id);
	std::vector<std::string> getMarkerNames();
	int getCallCounter();
	std::string getMarkerType();bool isDone();
	void remove();bool isRemoved();
	std::string getName(){
		return name;
	}
	virtual ~Object() = default;
};

#endif /* OBJECT_H_ */
