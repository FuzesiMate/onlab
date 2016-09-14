/*
 * Model.cpp
 *
 *  Created on: 2016. aug. 11.
 *      Author: Máté
 */

#include "Model.h"
#include <tbb/parallel_for_each.h>
#include <string>
#include "TemplateConfiguration.h"

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
				auto limit = o.second.get<int>("limit");

				MarkerType type;

				if(markerType == "aruco"){
					type = MarkerType::ARUCO;
				}else if (markerType == "circle"){
					type = MarkerType::CIRCLE;
				}else if(markerType == "irtd"){
					type = MarkerType::IRTD;
				}

				try{
					objects[name] = std::make_shared< Object<CONFIG> >(name , numberOfMarkers, type , limit , g);
				}catch(std::exception& e){
					std::cout<<"error: "<<e.what()<<std::endl;
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

template<typename CONFIG>
bool Model<CONFIG>::isDone(std::string objectName){
	return objects[objectName]->isDone();
}

template<typename CONFIG>
bool Model<CONFIG>::isRemoved(std::string objectName){
	return objects[objectName]->isRemoved();
}

template<typename CONFIG>
int64_t Model<CONFIG>::getFrameIndex(std::string objectName){
	return objects[objectName]->getFrameIndex();
}

template<typename CONFIG>
MarkerType Model<CONFIG>::getMarkerType(std::string objectName){
	return objects[objectName]->getMarkerType();
}

template <typename CONFIG>
void Model<CONFIG>::remove(std::string objectName){
	objects[objectName]->remove();
}


