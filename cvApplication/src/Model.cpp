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

template<typename CONFIG>
bool Model<CONFIG>::build(boost::property_tree::ptree cfg , tbb::flow::graph& g){
	try{
		auto objectList = cfg.get_child("objects");

			for(auto o : objectList){
				auto name = o.second.get<std::string>("name");
				auto numberOfMarkers = o.second.get<int>("numberofparts");
				auto markerType = o.second.get<std::string>("markertype");

				try{
					objects[name] = std::make_shared< Object<CONFIG> >(name , numberOfMarkers , g);
					std::cout<<"instant done"<<std::endl;

				}catch(std::exception& e){
					std::cout<<" tegeg"<<std::endl;
				}

				for(auto m : o.second.get_child("markers")){
					auto markername = m.second.get<std::string>("name");
					auto id = m.second.get<int>("id");

					objects.at(name)->addMarker(markername, id);
				}
			}

	}catch(std::exception& e){
		std::cout<<"JSON parsing error! message: "<<e.what()<<std::endl;
		return false;
	}

	return true;
}

template<typename CONFIG>
 tbb::concurrent_vector<cv::Point2f> const& Model<CONFIG>::getPosition(std::string objectName , std::string markerName){
	return objects.at(objectName)->getMarkerPosition(markerName);
}

template<typename CONFIG>
std::vector<std::string> const Model<CONFIG>::getObjectNames(){
	std::vector<std::string> names;
	for(auto o : objects){
		names.push_back(o.first);
	}
	return names;
}

template<typename CONFIG>
std::vector<std::string> const Model<CONFIG>::getMarkerNames(std::string objectName){
	return objects.at(objectName)->getMarkerNames();
}

template<typename CONFIG>
std::shared_ptr<Object<CONFIG> > const Model<CONFIG>::getObject(std::string objectName){
	return objects.at(objectName);
}


