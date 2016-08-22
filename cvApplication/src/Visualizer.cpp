/*
 * Visualizer.cpp
 *
 *  Created on: 2016. aug. 19.
 *      Author: Máté
 */

#include "Visualizer.h"
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <chrono>
#include <iostream>

void Visualizer::draw(tbb::flow::tuple<Frame, ImageProcessingResult> data){
	frameBuffer.push_back(std::get<0>(data));
	dataBuffer.push_back(std::get<1>(data));

	auto time = std::chrono::steady_clock::now();
	auto currentTimestamp = std::chrono::duration_cast<std::chrono::milliseconds>(time.time_since_epoch()).count();

	auto diff = currentTimestamp-frameBuffer.begin()->timestamp;

	if((500-diff)>0){
		Sleep(500-(diff));
	}

	size_t i = 0 ;
	for(auto& image : frameBuffer.begin()->images){

		for(auto& o : *dataBuffer.begin()){
				for(auto& m : o.second){
					if(i<m.second.size()){
						cv::putText(image , m.first , cv::Point(m.second[i].x,m.second[i].y) ,cv::FONT_HERSHEY_SIMPLEX ,  1.0 , cv::Scalar(255,255,255) , 2.0);
					}
				}
			}

		std::stringstream winname;
		winname<<"cam "<<i;
		cv::imshow(winname.str() , image);
		i++;
	}

	frameBuffer.erase(frameBuffer.begin());
	dataBuffer.erase(dataBuffer.begin());

	cv::waitKey(5);
}
