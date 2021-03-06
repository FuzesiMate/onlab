/*
 * Visualizer.cpp
 *
 *  Created on: 2016. aug. 19.
 *      Author: M�t�
 */

#include "Visualizer.h"
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <chrono>
#include <iostream>

//#define WITH_DELAY

tbb::flow::continue_msg Visualizer::process(tbb::flow::tuple<Frame, ModelData> data){
	frameBuffer.push_back(std::get<0>(data));
	dataBuffer.push_back(std::get<1>(data));

#ifdef WITH_DELAY

	auto time = std::chrono::steady_clock::now();
	auto currentTimestamp = std::chrono::duration_cast<std::chrono::milliseconds>(time.time_since_epoch()).count();

	auto diff = currentTimestamp-frameBuffer.begin()->timestamp;

	if((500-diff)>0){
		//Sleep(500-(diff));
		std::this_thread::sleep_for(std::chrono::milliseconds(500-diff));
	}

#endif

	size_t i = 0 ;
	for(auto& image : frameBuffer.begin()->images){

		auto& modelData = *dataBuffer.begin();

			for(auto& objectData : modelData.objectData){

				for(auto& markerData : objectData.markerData){
					if(i<markerData.screenPosition.size() && markerData.tracked[i]){
						cv::putText(image , markerData.name , cv::Point(markerData.screenPosition[i].x,markerData.screenPosition[i].y) ,cv::FONT_HERSHEY_SIMPLEX ,  1.0 , cv::Scalar(255,255,255) , 2.0);
					}
				}
			}

		cv::resize(image,image,cv::Size(800,600));
		std::stringstream winname;
		winname<<"cam "<<i;
		cv::imshow(winname.str() , image);
		cv::waitKey(10);
		i++;
	}


	frameBuffer.erase(frameBuffer.begin());
	dataBuffer.erase(dataBuffer.begin());

}
