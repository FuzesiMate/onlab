/*
 * ImageProcessor.h
 *
 *  Created on: 2016. ápr. 13.
 *      Author: Máté
 */

#include <opencv2/core.hpp>
#include <iostream>
#include "IImageProcessor.h"

#ifndef IRIMAGEPROCESSOR_H_
#define IRIMAGEPROCESSOR_H_

#define THRESH_VALUE		"threshold"

class IRImageProcessor :public IImageProcessor{
private:
	int thresholdValue;
	std::string windowName;
public:
	IRImageProcessor();
	void setWindow(std::string winname);
	virtual std::vector<cv::Point2f> processImage(cv::Mat frame) override;
	virtual void setFilterValues(boost::property_tree::ptree propertyTree) override;
	~IRImageProcessor();
};



#endif /* IRIMAGEPROCESSOR_H_ */
