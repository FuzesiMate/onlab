/*
 * IImageProcessor.h
 *
 *  Created on: 2016. máj. 13.
 *      Author: Máté
 */

#ifndef IIMAGEPROCESSOR_H_
#define IIMAGEPROCESSOR_H_

#include <opencv2/core.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>


class IImageProcessor {
public:
	IImageProcessor(){};
	//Process the current frame and return the found contours
	virtual std::vector< std::vector<cv::Point> > processImage(cv::Mat frame)=0;
	//set a window to show the processed image
	virtual void setWindow(std::string winname)=0;
	//set the processing specific filter values
	virtual void setFilterValues(boost::property_tree::ptree propertyTree)=0;
	//get the processing specific additional information to identify contours
	virtual std::vector<int> getMarkerIdentifiers()=0;
	//set marker identifiers
	virtual void setMarkerIdentifiers(std::vector<int> identifiers)=0;
	//virtual destructor
	virtual ~IImageProcessor(){};
};

#endif /* IIMAGEPROCESSOR_H_ */
