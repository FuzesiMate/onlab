#ifndef FRAMEPROVIDERFACTORY_H_
#define FRAMEPROVIDERFACTORY_H_

#include "Camera.h"
#include "VideoSource.h"
#include "FrameProvider.h"
#include "InitFailedException.h"
#include <boost/property_tree/ptree.hpp>

#define TYPE 					"type"

enum FrameProviderType {
	XIMEA = CV_CAP_XIAPI,
	DEFAULT = 0,
	VIDEO_SOURCE = 1
};

std::map<std::string, FrameProviderType> res_FrameProviderType = { {"ximea",FrameProviderType::XIMEA} ,{"default" , FrameProviderType::DEFAULT} ,{"video",FrameProviderType::VIDEO_SOURCE} };

class FrameProviderFactory {
public:
	FrameProviderFactory()=delete;

	static std::unique_ptr<FrameProvider> createFrameProvider(boost::property_tree::ptree parameters, tbb::flow::graph& g) {
		
		std::unique_ptr<FrameProvider> frameProvider;
		try {
			auto providerType = res_FrameProviderType[parameters.get<std::string>(TYPE)];

			//Same constructor and init method for ximea camera and default camera (webcam)
			if (providerType == FrameProviderType::XIMEA || providerType == FrameProviderType::DEFAULT) {

				int numberOfCameras = parameters.get<int>(NUMBEROFCAMERAS);
				int fps = parameters.get<int>(FPS);
				int expo = parameters.get<int>(EXPOSURE);
				float gain = parameters.get<float>(GAIN);

				auto cam = std::unique_ptr<Camera>( new Camera(fps, expo, gain, numberOfCameras, g));
				if (!cam->initialize(providerType)) {
					throw std::exception("Camera initialization failed");
				}
				frameProvider = std::move(cam);
			}
			else if (providerType == FrameProviderType::VIDEO_SOURCE) {
				
				auto source_config = parameters.get_child(SOURCES);
				auto fps = parameters.get<int>(FPS);
				std::vector<std::string> sources;
				for (auto& source_item : source_config) {
					sources.push_back(source_item.second.get<std::string>(""));
				}

				auto vid = std::unique_ptr<VideoSource>(new VideoSource(fps, g));
				if (!vid->initialize(sources)) {
					throw std::exception("Video player initialization failed");
				}
				frameProvider = std::move(vid);
			}
		}
		catch (std::exception& e) {
			throw e;
		}

		return frameProvider;
	}

	virtual ~FrameProviderFactory()=default;
};

#endif /* FRAMEPROVIDERFACTORY_H_ */

