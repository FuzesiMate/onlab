/*
 * Model.h
 *
 *  Created on: 2016. aug. 11.
 *      Author: M�t�
 */

#ifndef MODEL_H_
#define MODEL_H_

#include <tbb/concurrent_unordered_map.h>
#include <boost/property_tree/ptree.hpp>
#include <mutex>
#include <condition_variable>
#include "ImageProcessor.h"
#include "Object.h"
#include "DataTypes.h"
#include "TemplateConfiguration.h"

template<typename CONFIG>

class Model:public Processor<ModelData, tbb::flow::continue_msg , tbb::flow::queueing> {
private:

	//wait-notify pattern variables
	std::mutex lock;
	std::condition_variable new_data;

	tbb::concurrent_unordered_map<std::string , std::shared_ptr<Object<CONFIG> > > objects;

	ModelData modelData;
	uint64_t provided;

public:
	Model(tbb::flow::graph& g):Processor<ModelData, tbb::flow::continue_msg , tbb::flow::queueing>(g ,1),provided(0){};

	tbb::flow::continue_msg process(ModelData data);

	//instantiate objects and set properties provided by user in config file
	bool build(boost::property_tree::ptree config , tbb::flow::graph& g);

	//get information about currently processed objects and markers
	std::vector<std::string> const getObjectNames();
	std::vector<std::string> const getMarkerNames(std::string objectName);
	std::shared_ptr<Object<CONFIG> > const getObject(std::string objectName);
	MarkerType getMarkerType(std::string objectName);
	bool isDone(std::string objectName);

	//get actual collected position data
	ModelData getData();
	ObjectData getObjectData(std::string object);
	MarkerData getMarkerData(std::string object ,std::string marker);

	//remove object from processing workflow
	void remove(std::string objectName);
	bool isRemoved(std::string objectName);

	//default destructor
	virtual ~Model() = default;
};

#endif /* MODEL_H_ */
