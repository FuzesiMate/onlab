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
#include "InitFailedException.h"
#include <boost/property_tree/ptree.hpp>

#define NUMBEROFCAMERAS 		"number"
#define EXPOSURE				"exposure"
#define GAIN					"gain"
#define FPS						"fps"
#define TYPE 					"type"

enum FrameProviderType {
	XIMEA = CV_CAP_XIAPI,
	DEFAULT = 0,
	VIDEO_SOURCE = 1
};

std::map<std::string, FrameProviderType> res_FrameProviderType = { {"ximea",FrameProviderType::XIMEA} ,{"default" , FrameProviderType::DEFAULT} ,{"video",FrameProviderType::VIDEO_SOURCE} };

class FrameProviderFactory {
public:
	FrameProviderFactory();

	static std::shared_ptr<FrameProvider> createFrameProvider(boost::property_tree::ptree parameters, tbb::flow::graph& g) {

		try{
			auto providerType = res_FrameProviderType[parameters.get<std::string>(TYPE)];

			//Same constructor and init method for ximea camera and default camera (webcam)
			if (providerType == FrameProviderType::XIMEA || providerType == FrameProviderType::DEFAULT) {

				int numberOfCameras = parameters.get<int>(NUMBEROFCAMERAS);
				int fps = parameters.get<int>(FPS);
				int expo = parameters.get<int>(EXPOSURE);
				float gain = parameters.get<float>(GAIN);

				auto cam = std::make_shared<Camera>(fps, expo, gain, numberOfCameras, g);
				if (!cam->init(providerType)) {
					throw std::exception("Camera initialization failed");
				}
				return cam;
			}
			else if (providerType == FrameProviderType::VIDEO_SOURCE) {
				throw std::exception("This function is currently not available!");
			}
		}
		catch (std::exception& e) {
			throw;
		}
	}

	virtual ~FrameProviderFactory();
};

#endif /* FRAMEPROVIDERFACTORY_H_ */
