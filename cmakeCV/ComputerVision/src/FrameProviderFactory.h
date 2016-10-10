/*
 * FrameProviderFactory.h
 *
 *  Created on: Oct 7, 2016
 *      Author: teqbox
 */

#ifndef FRAMEPROVIDERFACTORY_H_
#define FRAMEPROVIDERFACTORY_H_

#include "Camera.h"
#include "FrameProvider.h"
#include <boost/property_tree/ptree.hpp>

#define NUMBEROFCAMERAS 		"number"
#define EXPOSURE				"exposure"
#define GAIN					"gain"
#define FPS						"fps"
#define TYPE 					"type"

enum FrameProviderType{
	XIMEA = CV_CAP_XIAPI,
	DEFAULT = 0,
	VIDEO_SOURCE = 1
};

class FrameProviderFactory {
public:
	FrameProviderFactory();

	static std::shared_ptr<FrameProvider> createFrameProvider(boost::property_tree::ptree parameters , tbb::flow::graph& g);

	virtual ~FrameProviderFactory();
};

#endif /* FRAMEPROVIDERFACTORY_H_ */
