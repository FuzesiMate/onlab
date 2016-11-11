#ifndef IMAGEPROCESSORFACTORY_H_
#define IMAGEPROCESSORFACTORY_H_

#include <memory>
#include <string>
#include <boost/property_tree/ptree.hpp>
#include "ImageProcessor.h"
#include "ArucoImageProcessor.h"
#include "IRTDImageProcessor.h"
#include "CircleDetector.h"
#include "ArucoImageProcessor.cpp"
#include "IRTDImageProcessor.cpp"
#include "CircleDetector.cpp"

#define TYPE			"type"
#define DICTIONARY		"dictionary"

enum MarkerType {
	ARUCO,
	CIRCLE,
	IRTD
};

std::map<std::string, MarkerType> res_MarkerType = { { "aruco",MarkerType::ARUCO },{ "irtd", MarkerType::IRTD },{ "circle",MarkerType::CIRCLE } };

class ImageProcessorFactory
{
public:
	ImageProcessorFactory()=delete;

	template <typename CONFIG>
	static std::shared_ptr<ImageProcessor<CONFIG> >createImageProcessor(boost::property_tree::ptree parameters, tbb::flow::graph& g) {

		std::shared_ptr < ImageProcessor<CONFIG> > imageprocessor;

		try {
			auto markerType = res_MarkerType[parameters.get<std::string>(TYPE)];

			switch (markerType) {
			case ARUCO:
				imageprocessor = std::make_shared<ArucoImageProcessor<CONFIG> >(g);
				break;
			case IRTD:
			{
				int threshold = parameters.get<int>(THRESHOLD);
				int duration = parameters.get<int>(DURATION);
				int setupTime = parameters.get<int>(SETUP_TIME);

				imageprocessor = std::make_shared<IRTDImageProcessor<CONFIG> >(threshold, duration, setupTime, g);
				break;
			}
			case CIRCLE:
				imageprocessor = std::make_shared<CircleDetector<CONFIG> >(g);
				break;
			default:
				throw std::exception("Not supported image processing method!");
				break;
			}

			imageprocessor->reconfigure(parameters);
		}
		catch (std::exception& e) {
			throw e;
		}
		
		return imageprocessor;
	}

	virtual ~ImageProcessorFactory()=delete;
};

#endif //IMAGEPROCESSORFACTORY_H_