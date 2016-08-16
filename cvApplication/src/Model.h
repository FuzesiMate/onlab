/*
 * Model.h
 *
 *  Created on: 2016. aug. 11.
 *      Author: Máté
 */

#ifndef MODEL_H_
#define MODEL_H_

#include <tbb/concurrent_unordered_map.h>
#include <boost/property_tree/ptree.hpp>
#include "ImageProcessor.h"
#include "Object.h"

class Model {
private:
	tbb::concurrent_unordered_map<std::string , std::shared_ptr<Object> > objects;
public:
	Model() = default;
	bool build(boost::property_tree::ptree config , tbb::flow::graph& g);
	tbb::concurrent_vector<cv::Point2f> const & getPosition(std::string objectName , std::string markerName);
	std::vector<std::string> const getObjectNames();
	std::vector<std::string> const getMarkerNames(std::string objectName);
	std::shared_ptr<Object> const getObject(std::string objectName);
	int getCallCounter(std::string objectName){
		return objects[objectName]->getCallCounter();
	}
	virtual ~Model() = default;
};

#endif /* MODEL_H_ */
