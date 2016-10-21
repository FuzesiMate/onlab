#pragma once

#include <memory>
#include <string>
#include <boost/property_tree/ptree.hpp>
#include "ImageProcessor.h"
#include "ArucoImageProcessor.h"
#include "IRTDImageProcessor.h"
#include "CircleDetector.h"

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
	ImageProcessorFactory();

	template <typename CONFIG>
	static std::shared_ptr<ImageProcessor<CONFIG> >createImageProcessor(boost::property_tree::ptree parameters, tbb::flow::graph& g) {

		auto ipType = res_MarkerType[parameters.get<std::string>(TYPE)]

		std::shared_ptr < ImageProcessor<CONFIG> > imageprocessor;

		switch (ipType) {
		case ARUCO:
			imageprocessor = std::make_shared(ArucoImageProcessor<CONFIG> , *this);
			break;
		case IRTD:
			imageprocessor = std::make_shared(IRTDImageProcessor<CONFIG>, *this);
			break;
		case CIRCLE:
			imageprocessor = std::make_shared(CircleDetector<CONFIG> , *this);
			break;
		default:
			throw std::exception("Not supported image processing method!");
			break;
		}

		imageprocessor->setProcessingSpecificValues(parameters);
	}

	virtual ~ImageProcessorFactory();
};

