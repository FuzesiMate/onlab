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

tbb::flow::continue_msg Visualizer::process(tbb::flow::tuple<Frame, ImageProcessingResult> data){
	frameBuffer.push_back(std::get<0>(data));
	dataBuffer.push_back(std::get<1>(data));

	auto time = std::chrono::steady_clock::now();
	auto currentTimestamp = std::chrono::duration_cast<std::chrono::milliseconds>(time.time_since_epoch()).count();

	auto diff = currentTimestamp-frameBuffer.begin()->timestamp;

	if((500-diff)>0){
		//Sleep(500-(diff));
		//std::this_thread::sleep_for(std::chrono::milliseconds(200-diff));
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

		/*
		 *
		 * Just for screencast

		auto objects = *dataBuffer.begin();
		auto human = objects["human"];

		auto lefthand = human["la"];
		auto righthand = human["ra"];
		auto leftleg = human["ll"];
		auto rightleg = human["rl"];

		cv::Mat drawing = cv::Mat::zeros(image.size() , CV_8UC3);

		if(lefthand.size()>i && righthand.size()>i){

			cv::Point center((lefthand[i].x+righthand[i].x)/2 , (lefthand[i].y+righthand[i].y)/2);

			cv::line(drawing , center , cv::Point(righthand[i].x , righthand[i].y) , cv::Scalar(0,0,255) , 2.0);
			cv::line(drawing , cv::Point(lefthand[i].x , lefthand[i].y) , center , cv::Scalar(255,0,0) , 2.0);

			cv::line(drawing , cv::Point(center.x , center.y+50) , center , cv::Scalar(255,255,0) , 2.0);

			if(leftleg.size()>i){
				cv::line(drawing , cv::Point(center.x , center.y+50) , cv::Point(leftleg[i].x , leftleg[i].y) , cv::Scalar(0,255,0) , 2.0);
			}
			if(rightleg.size()>i){
				cv::line(drawing , cv::Point(center.x , center.y+50) , cv::Point(rightleg[i].x , rightleg[i].y) , cv::Scalar(0,255,255) , 2.0);
			}
		}


		cv::resize(drawing,drawing,cv::Size(800,600));
		std::stringstream dname;
		dname<<"drawing "<<i;
		cv::imshow(dname.str() , drawing);

*/

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
