/*
 * IRTDImageProcessor.h
 *
 *  Created on: 2016. szept. 9.
 *      Author: M�t�
 */

#ifndef IRTDIMAGEPROCESSOR_H_
#define IRTDIMAGEPROCESSOR_H_

#include <atomic>
#include <opencv2/core.hpp>
#include "LedController.h"
#include "ImageProcessor.h"
#include "DataTypes.h"

#define THRESHOLD 	"threshold"
#define SETUP_TIME 	"setuptime"
#define DURATION 	"duration"

enum BodyPart{
	LEFT_HAND,
	RIGHT_HAND,
	LEFT_ANKLE,
	RIGHT_ANKLE
};

struct Contour{
	std::vector<cv::Point> points;
	int clusterId = -1;
	double area;

	bool operator == (const Contour c){
		return this->points == c.points;
	}
};

struct ContourPair{
	Contour c1;
	Contour c2;
	float distance;
};

struct Cluster{
	std::vector<Contour> contours;
	std::vector<cv::Point> allPoints;
	double area;
	cv::Rect boundingRect;
};

template <typename CONFIG>
class IRTDImageProcessor: public ImageProcessor<CONFIG> {
private:
	std::atomic<int> threshold;
	std::atomic<int> duration;
	std::atomic<int> setupTime;
	LedController ledController;

	std::vector<Cluster> clusterContours(std::vector<std::vector<cv::Point> > contours , int distanceThreshold);

public:
	void startLedController();

	IRTDImageProcessor(int threshold , int duration , int setupTime, tbb::flow::graph& g):ImageProcessor<CONFIG>(g),threshold(threshold),duration(duration),setupTime(setupTime){
		ledController.addPattern(std::vector<int>{0,255,0,50});
		ledController.addPattern(std::vector<int>{255,0,50,0});

		//tbb::tbb_thread ledControllerThread(std::bind(&IRTDImageProcessor::startLedController , this));
		//ledControllerThread.detach();
	};

	ImageProcessingData<CONFIG> process(Frame frame);
	void reconfigure(boost::property_tree::ptree config);

	virtual ~IRTDImageProcessor() = default;
};

#endif /* IRTDIMAGEPROCESSOR_H_ */
