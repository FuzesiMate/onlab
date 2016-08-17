/*
 * ComputerVision.cpp
 *
 *  Created on: 2016. aug. 9.
 *      Author: Máté
 */

#include "ComputerVision.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <cstdio>
#include <exception>
#include "ArucoImageProcessor.h"
#include "ArucoImageProcessor.cpp"
#include "Object.cpp"
#include "Model.cpp"

bool ComputerVision::initialize(std::string configFilePath) {

	boost::property_tree::ptree config;
	try{
		boost::property_tree::read_json(configFilePath, config);
	}catch(std::exception& e){
		std::cout<<"JSON file is missing or invalid!"<<std::endl;
		initialized = false;
		return initialized;
	}

	camera = std::unique_ptr<Camera>(
			new Camera(config.get<int>(FPS) , config.get<int>(EXPOSURE) , config.get<int>(GAIN) , config.get<int>(NUMBEROFCAMERAS) , *this));

	auto cameraType = config.get<std::string>(CAMERATYPE);

	if(cameraType=="ximea"){
		initialized = camera->init(CV_CAP_XIAPI);
	}else if(cameraType=="default"){
		initialized = camera->init(0);
	}

	processing = false;
	this->config = config;

	return initialized;
}


void ComputerVision::startProcessing() {

	using templateConfiguration = TEMPLATE_CONFIG<tbb::concurrent_vector<cv::Point2f> , tbb::concurrent_vector<int > >;

	tbb::concurrent_unordered_map<MarkerType , std::shared_ptr<ImageProcessor<templateConfiguration> > > imageprocessors;

	tbb::concurrent_unordered_map<MarkerType , int> successorCounter;

	for(auto& m : config.get_child("markertypes")){
		auto type = m.second.get<std::string>("");

		if(type=="aruco"){
			successorCounter[MarkerType::ARUCO] = 0;
			imageprocessors[MarkerType::ARUCO] = std::make_shared<ArucoImageProcessor<templateConfiguration> >(*this);
		}if(type=="circle"){
			successorCounter[MarkerType::CIRCLE] = 0;
			imageprocessors[MarkerType::CIRCLE] = std::make_shared<ArucoImageProcessor<templateConfiguration> >(*this);
		}
	}

	std::shared_ptr<Model<templateConfiguration> > model = std::make_shared<Model<templateConfiguration> >();

	model->build(config , *this);

	if(initialized){

		processing = true;

		tbb::flow::function_node<Frame ,tbb::flow::continue_msg ,tbb::flow::queueing > drawer(*this , 1 , [&](Frame frame){

			auto time = std::chrono::steady_clock::now();
			auto currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(time.time_since_epoch()).count();

			auto diff = 500 -(currentTime-frame.timestamp);

			if(diff>0){
				Sleep(diff);
			}

			auto objects = model->getObjectNames();

			for(auto o : objects){
				auto markers = model->getMarkerNames(o);

				for(auto m : markers){
					auto position = model->getPosition(o , m);

					int i = 0 ;
					for(auto p : position){
						cv::putText(frame.images[i] , m , cv::Point(p.x,p.y) , cv::FONT_HERSHEY_SIMPLEX ,1.0 ,cv::Scalar(255,255,255) , 2.0);
						i++;
					}
				}
			}

			int i = 0;
			for(auto f : frame.images){
				std::stringstream frameId;
				frameId<<i;
				cv::Mat resized;
				cv::resize(f , resized , cv::Size(640,480));
				cv::imshow(frameId.str() , resized);
				i++;
			}
			cv::waitKey(5);

		});

		tbb::flow::function_node<int> sink(*this , 1 , [&](int i){

		});

		tbb::flow::join_node<tbb::flow::tuple<Frame , tbb::flow::continue_msg> , tbb::flow::queueing  > join(*this);

/*
		tbb::flow::function_node<tbb::flow::tuple<Frame , ImageProcessingData<defaultData , defaultIdentifier > > ,
		tbb::flow::continue_msg , tbb::flow::queueing> draw(*this , 1 , [&](tbb::flow::tuple<Frame , ImageProcessingData<defaultData , defaultIdentifier > > data){
			auto frame = std::get<0>(data);
			auto points = std::get<1>(data);

			 auto time = std::chrono::steady_clock::now();
			 auto currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(time.time_since_epoch()).count();

			 auto diff = 500 -(currentTime-frame.timestamp);

			 if(diff>0){
				 Sleep(diff);
			 }

			if(frame.frameIndex!=points.frameIndex){
				std::cout<<"Sequence fail!"<<std::endl;
			}

			int i = 0 ;
			for(auto f : frame.images){
				int j = 0;
				for(auto p : points.data[i]){
					cv::circle(f , cv::Point(p.x , p.y) , 5 , cv::Scalar(255,255,255) , 3.0);

					std::stringstream idstr;
					idstr<<points.identifiers[i][j];
					cv::putText(f , idstr.str() , cv::Point(p.x,p.y) , cv::FONT_HERSHEY_SIMPLEX , 1.0 , cv::Scalar(255,255,255) , 2.0);
					j++;
				}

				cv::resize(f , f , cv::Size(640,480));
				std::stringstream winname;
				winname<<"processed ";
				winname<<i;
				cv::imshow(winname.str() , f);
				i++;
			}

			cv::waitKey(5);

		});

*/

		tbb::flow::sequencer_node<Frame> frameSequencer(*this , [&](Frame f)->size_t{
			return f.frameIndex;
		});

		tbb::flow::continue_node<tbb::flow::continue_msg> cont(*this , [&](tbb::flow::continue_msg msg){
			/*
			auto objects = model->getObjectNames();

			auto prevO = objects[0];

			for(auto o : objects){
				auto index = model->getObject(o)->getFrameIndex();
				auto prevIndex = model->getObject(prevO)->getFrameIndex();

				if(index!=prevIndex){
					std::cout<<"not good: "<<std::endl;
					std::cout<<"current: "<<o<<" "<<index<<std::endl;
					std::cout<<"previous: "<<prevO<<" "<<prevIndex<<std::endl;
					std::cout<<std::endl<<std::endl;
				}
			}
			*/
		});

		tbb::flow::sequencer_node<ImageProcessingData<defaultData , defaultIdentifier > > dataSequencer(*this , [&](ImageProcessingData<defaultData , defaultIdentifier > d)->size_t{
			return d.frameIndex;
		});

		tbb::flow::broadcast_node<ImageProcessingData<defaultData , defaultIdentifier > > broadcaster(*this);

		tbb::concurrent_vector<std::shared_ptr<tbb::flow::sequencer_node<ImageProcessingData<defaultData, defaultIdentifier> > > > sequencers;

		tbb::concurrent_unordered_map<MarkerType , std::shared_ptr<tbb::flow::broadcast_node<ImageProcessingData<defaultData, defaultIdentifier> > > > broadcasters;

		for(auto& ip : imageprocessors){
			auto sequencer = std::make_shared<tbb::flow::sequencer_node<ImageProcessingData<defaultData, defaultIdentifier> > >
										(*this , [](ImageProcessingData<defaultData, defaultIdentifier> data)->size_t{
											return data.frameIndex;
										});

			broadcasters[ip.first] = std::make_shared<tbb::flow::broadcast_node<ImageProcessingData<defaultData, defaultIdentifier> > >(*this);

			sequencers.push_back(sequencer);

			make_edge(*camera , *ip.second);
			make_edge(*ip.second , *sequencers[sequencers.size()-1]);
			make_edge(*sequencers[sequencers.size()-1] , *broadcasters[ip.first]);
		}

		int idx = 0;
		tbb::flow::source_node<int> controller(*this,
				[&](int& timestamp)->bool {
					auto objects = model->getObjectNames();

					for(auto& o : objects) {
						if(model->isDone(o) && !model->getObject(o)->isRemoved()) {
							remove_edge(*broadcasters[model->getObject(o)->getMarkerType()] , *model->getObject(o));
							successorCounter[model->getObject(o)->getMarkerType()]--;
							model->getObject(o)->remove();
						}
						if(successorCounter[model->getObject(o)->getMarkerType()]==0){
							remove_edge(*camera , *imageprocessors[model->getObject(o)->getMarkerType()]);
						}

					}
					idx++;
					Sleep(20);
					return true;
				}, false);


		make_edge(*camera , drawer);
		make_edge(controller , sink);

		auto objects = model->getObjectNames();

		for(auto o : objects){
			successorCounter[model->getObject(o)->getMarkerType()]++;
			make_edge(*broadcasters[model->getObject(o)->getMarkerType()] , *model->getObject(o));
			make_edge(*model->getObject(o) , cont);
		}

		camera->startRecording();
		sender.activate();
		this->wait_for_all();
	}
}

void ComputerVision::stopProcessing() {
	processing = false;
	camera->stopRecording();
}

void ComputerVision::reconfigure(std::string configFilePath) {

}

bool ComputerVision::isProcessing(){
	return processing ;
}
