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
	IImageProcessor(){}
	virtual std::vector< std::vector<cv::Point> > processImage(cv::Mat frame)=0;
	virtual void setWindow(std::string)=0;
	virtual void setFilterValues(boost::property_tree::ptree propertyTree)=0;
	virtual ~IImageProcessor(){}
};

#endif /* IIMAGEPROCESSOR_H_ */
