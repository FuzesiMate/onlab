
#include "FrameProviderFactory.h"

std::map<std::string, FrameProviderType> res_FPType = {
														{ "ximea",FrameProviderType::XIMEA },
														{ "default", FrameProviderType::DEFAULT },
														{"video", FrameProviderType::VIDEO_SOURCE }
													  };

std::shared_ptr<FrameProvider> FrameProviderFactory::createFrameProvider(boost::property_tree::ptree parameters , tbb::flow::graph& g){

	auto type = parameters.get<std::string>(TYPE);
	auto sourceType = res_FPType[type];

	if(sourceType == FrameProviderType::XIMEA || sourceType == FrameProviderType::DEFAULT){
		auto camera = std::make_shared<Camera>(parameters.get<int>(FPS),
							parameters.get<int>(EXPOSURE), parameters.get<float>(GAIN),
							parameters.get<int>(NUMBEROFCAMERAS), g);
		camera->init(sourceType);

		return camera;
	}

	if(sourceType == FrameProviderType::VIDEO_SOURCE){

	}

}
