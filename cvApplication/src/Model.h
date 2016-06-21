/*
 * Model.h
 *
 *  Created on: 2016. máj. 13.
 *      Author: Máté
 */

#ifndef MODEL_H_
#define MODEL_H_

#include "IImageProcessor.h"
#include "Object.h"
#include "IRImageProcessor.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <map>
#include "Marker.h"

class Model {
private:
	std::map<std::string,Object> objects;
	std::vector<std::string> listOfObjectIds;
	bool showGrid;
public:
	Model();
	bool buildModel(boost::property_tree::ptree propertyTree);
	void updateModel(PointSet points);
	cv::Point3f getPosition(std::string objectId , std::string markerId);
	std::vector<std::string> getObjectIds();
	std::vector<std::string> getMarkerIds(std::string objectId);
	void draw(Frame frame);
	void setShowGrid(bool show);
	bool isTracked(std::string objectId);
	virtual ~Model();
};

#endif /* MODEL_H_ */
