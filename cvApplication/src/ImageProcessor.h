/*
 * ImageProcessor.h
 *
 *  Created on: 2016. ápr. 13.
 *      Author: Máté
 */

#include <opencv2/core.hpp>
#include <iostream>

#ifndef IMAGEPROCESSOR_H_
#define IMAGEPROCESSOR_H_

class ImageProcessor{
private:
	int thresholdValue;
	int cannythreshold;
	std::string windowName;
public:
	ImageProcessor(std::string windowName);
	std::vector<cv::Point2f> processImage(cv::Mat frame);
};



#endif /* IMAGEPROCESSOR_H_ */
