/*
 * CoordinateTransformer.h
 *
 *  Created on: Sep 19, 2016
 *      Author: teqbox
 */

#ifndef COORDINATETRANSFORMER_H_
#define COORDINATETRANSFORMER_H_

#include "Processor.h"
#include "TemplateConfiguration.h"
#include "Camera.h"
#include <tbb/concurrent_unordered_map.h>
#include <tbb/flow_graph.h>

struct Matrices{
	tbb::concurrent_vector<cv::Mat> cameraMatrix;
	tbb::concurrent_vector<cv::Mat> distCoeffs;
	tbb::concurrent_vector<cv::Mat> projectionMatrix;
	tbb::concurrent_vector<cv::Mat> rectificationMatrix;
};

class CoordinateTransformer: public Processor<ModelData, ModelData ,tbb::flow::queueing> {
private:
	Matrices matrices;
	bool canTransform;
public:
	CoordinateTransformer(tbb::flow::graph& g):Processor<ModelData,ModelData,tbb::flow::queueing>(g , tbb::flow::unlimited),canTransform(false){};

	bool loadMatrices(std::string path);

	ModelData process(ModelData ipData);

	virtual ~CoordinateTransformer()=default;
};

#endif /* COORDINATETRANSFORMER_H_ */
