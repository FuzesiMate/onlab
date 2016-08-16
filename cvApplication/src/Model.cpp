/*
 * Model.cpp
 *
 *  Created on: 2016. aug. 11.
 *      Author: Máté
 */

#include "Model.h"
#include <tbb/parallel_for_each.h>

/*
void Model::update(ImageProcessingData< defaultData , defaultIdentifier > data){

	for(auto element : objects){
		element.second.update(data);
	}
}
*/

bool Model::build(boost::property_tree::ptree config , tbb::flow::graph& g){

	auto objectList = config.get_child("objects");

	for(auto o : objectList){
		auto name = o.second.get<std::string>("name");
		auto numberOfMarkers = o.second.get<int>("numberofparts");
		auto markerType = o.second.get<std::string>("markertype");

		objects[name] = std::shared_ptr<Object>( new Object(name, numberOfMarkers , g )) ;

		for(auto m : o.second.get_child("markers")){
			auto markername = m.second.get<std::string>("name");
			auto id = m.second.get<int>("id");

			objects[name]->addMarker(markername, id);
		}
	}

	return true;
}

 tbb::concurrent_vector<cv::Point2f> const& Model::getPosition(std::string objectName , std::string markerName){
	return objects[objectName]->getMarkerPosition(markerName);
}

std::vector<std::string> const Model::getObjectNames(){
	std::vector<std::string> names;
	for(auto o : objects){
		names.push_back(o.first);
	}
	return names;
}

std::vector<std::string> const Model::getMarkerNames(std::string objectName){
	return objects[objectName]->getMarkerNames();
}

std::shared_ptr<Object> const Model::getObject(std::string objectName){
	return objects[objectName];
}


