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
#include "IRObject.h"
#include "ArucoObject.h"
#include "IRImageProcessor.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <map>
#include "Marker.h"

class Model {
private:
	std::map<std::string,std::shared_ptr<Object> > objects;
	std::vector<std::string> objectIds;
	bool showGrid;
public:
	Model();
	bool buildModel(boost::property_tree::ptree propertyTree);
	void updateModel(std::pair<std::vector< std::vector<cv::Point> > ,std::vector< std::vector<cv::Point> > > contourSet ,std::pair<std::vector<int> , std::vector<int> > identifiers, Frame frame , Frame prevFrame );
	cv::Point3f getPosition(std::string objectId , std::string markerId);
	std::vector<std::string> getObjectNames();
	std::vector<std::string> getMarkerNames(std::string objectId);
	std::vector<int> getMarkerIds(std::string objectId);
	void draw(Frame frame);
	void setShowGrid(bool show);
	bool isTracked(std::string objectId);
	virtual ~Model();
};

#endif /* MODEL_H_ */
