/*
 * Model.cpp
 *
 *  Created on: 2016. aug. 11.
 *      Author: M�t�
 */

#include "Model.h"
#include <tbb/parallel_for_each.h>
#include <string>
#include "TemplateConfiguration.h"

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


/*
 * Awful solution caused by data type refactor TODO make better
 */

template<typename CONFIG>
tbb::flow::continue_msg Model<CONFIG>::process(ModelData data){

	//thread safety
	std::unique_lock<std::mutex> l(lock);

	//If it is empty, initialize the data structure
	if(modelData.objectData.empty()){
		modelData = data;
	}else{

	/*
	 * If the marker is untracked in an image, we store the last known position of the marker
	 * with the tracked flag set to false, so the user knows that it is risky information
	 */

		size_t objIndex = 0;
		for(auto& objectData : data.objectData){
			size_t markIndex = 0;
			for(auto& markerData : objectData.markerData){
				bool allTracked = true;

				size_t posIndex = 0;
				for(auto& position : markerData.screenPosition){
					if(markerData.tracked[posIndex]){
						modelData.objectData[objIndex].markerData[markIndex].screenPosition[posIndex]=position;
						modelData.objectData[objIndex].markerData[markIndex].tracked[posIndex]=true;
					}else{
						modelData.objectData[objIndex].markerData[markIndex].tracked[posIndex]=false;
						allTracked = false;
					}
					posIndex++;
				}
				if(allTracked){
					modelData.objectData[objIndex].markerData[markIndex].realPosition = markerData.realPosition;
				}
				markIndex++;
			}
			objIndex++;
		}

		modelData.frameIndex = data.frameIndex;
		modelData.timestamp = data.timestamp;
	}

	new_data.notify_one();
}

template<typename CONFIG>
ModelData Model<CONFIG>::getData(){

	/*
	 * if the position data was provided for a frameindex, it is not provided again
	 * this function blocks, until a new frame data arrives
	 * so the user wont get the already known data again
	 *
	 * TODO maybe i should make it configurable
	 */

	std::unique_lock<std::mutex> l(lock);

	new_data.wait(l , [this](){
		return provided != modelData.frameIndex;
	});

	provided = modelData.frameIndex;
	return modelData;
}

template<typename CONFIG>
ObjectData Model<CONFIG>::getObjectData(std::string object){
	for(auto& objectData : modelData.objectData){
		if(objectData.name == object){
			return objectData;
		}
	}
}

template<typename CONFIG>
MarkerData Model<CONFIG>::getMarkerData(std::string object ,std::string marker){
	for(auto& objectData : modelData.objectData){
		if(objectData.name == object){
			for(auto& markerData : objectData.markerData){
				if(markerData.name == marker){
					return markerData;
				}
			}
		}
	}
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
MarkerType Model<CONFIG>::getMarkerType(std::string objectName){
	return objects[objectName]->getMarkerType();
}

template <typename CONFIG>
void Model<CONFIG>::remove(std::string objectName){
	objects[objectName]->remove();
}


