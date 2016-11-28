/*
 * Visualizer.cpp
 *
 *  Created on: 2016. aug. 19.
 *      Author: M�t�
 */

#include "SimpleVisualizer.h"
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <thread>
#include <chrono>
#include <iostream>


tbb::flow::continue_msg SimpleVisualizer::process(tbb::flow::tuple<Frame, ModelData> data) {
	auto frame = std::get<0>(data);

	tbb::concurrent_vector<cv::Mat> clonedImages;
	for (auto i : frame.images) {
		clonedImages.push_back(i.clone());
	}

	frame.images = clonedImages;

	frameBuffer.push_back(frame);
	dataBuffer.push_back(std::get<1>(data));
	
	if (delay > 0 ) {
		auto time = std::chrono::steady_clock::now();
		auto ts = std::chrono::duration_cast<std::chrono::milliseconds>(time.time_since_epoch()).count();
		auto diff = ts - frameBuffer.begin()->timestamp;

		if ((delay - diff) <  delay) {
			tbb::flow::continue_msg m;
			return m;
		}
	}

	size_t i = 0;
	for (auto& image : frameBuffer.begin()->images) {

		auto& modelData = *dataBuffer.begin();

		for (auto& objectData : modelData.objectData) {

			for (auto& markerData : objectData.second.markerData) {
				if (markerData.second.tracked[i]) {
					cv::putText(image, markerData.second.name, cv::Point(markerData.second.screenPosition[i].x, markerData.second.screenPosition[i].y), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2);
				}
			}
		}

		cv::resize(image, image, cv::Size(640, 480));
		std::stringstream winname;
		winname << windowName << i;
		cv::imshow(winname.str(), image);
		cv::waitKey(10);
		i++;
	}

	frameBuffer.erase(frameBuffer.begin());
	dataBuffer.erase(dataBuffer.begin());

	tbb::flow::continue_msg msg;
	return msg;
}

SimpleVisualizer::~SimpleVisualizer() {
	cv::destroyAllWindows();
}