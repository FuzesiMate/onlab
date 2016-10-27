/*
 * Visualizer.cpp
 *
 *  Created on: 2016. aug. 19.
 *      Author: M�t�
 */

#include "SimpleVisualizer.h"
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
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

	if (delay > 0) {
		auto time = std::chrono::steady_clock::now();
		auto currentTimestamp = std::chrono::duration_cast<std::chrono::milliseconds>(time.time_since_epoch()).count();

		auto diff = currentTimestamp - frameBuffer.begin()->timestamp;

		if ((delay - diff) > 0) {
			std::this_thread::sleep_for(std::chrono::milliseconds(delay - diff));
		}
	}

	size_t i = 0;
	for (auto& image : frameBuffer.begin()->images) {

		auto& modelData = *dataBuffer.begin();

		for (auto& objectData : modelData.objectData) {

			for (auto& markerData : objectData.markerData) {
				if (markerData.tracked[i]) {
					cv::putText(image, markerData.name, cv::Point(markerData.screenPosition[i].x, markerData.screenPosition[i].y), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2);
				}
			}
		}

		cv::resize(image, image, cv::Size(800, 600));
		std::stringstream winname;
		winname << windowName << i;
		cv::imshow(winname.str(), image);
		cv::waitKey(5);
		i++;
	}


	frameBuffer.erase(frameBuffer.begin());
	dataBuffer.erase(dataBuffer.begin());

	tbb::flow::continue_msg msg;
	return msg;
}
