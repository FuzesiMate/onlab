/*
 * ArucoImageProcessor.h
 *
 *  Created on: 2016. jún. 30.
 *      Author: Máté
 */

#ifndef ARUCOIMAGEPROCESSOR_H_
#define ARUCOIMAGEPROCESSOR_H_

#include <opencv2/core.hpp>
#include <iostream>
#include <opencv2/aruco.hpp>
#include "IImageProcessor.h"

class ArucoImageProcessor :public IImageProcessor{
private:
	std::vector<int> markerIdentifiers;
	std::vector<int> foundMarkerIdentifiers;
	std::string winname;
	cv::Ptr<cv::aruco::Dictionary> dictionary;
	cv::Ptr<cv::aruco::DetectorParameters> detectorParams;

public:
	ArucoImageProcessor();
	virtual std::vector< std::vector<cv::Point> > processImage(cv::Mat frame) override;
	//set a window to show the processed image
	void setWindow(std::string) override;
	//set the processing specific filter values
	virtual void setFilterValues(boost::property_tree::ptree propertyTree) override;
	//get additional parameters to identify contours
	virtual std::vector<int> getMarkerIdentifiers() override;
	//virtual destructor
	virtual ~ArucoImageProcessor();
};

#endif /* ARUCOIMAGEPROCESSOR_H_ */
