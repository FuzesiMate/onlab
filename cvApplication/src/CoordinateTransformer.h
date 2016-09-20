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

class CoordinateTransformer: public Processor<ImageProcessingResult, tbb::concurrent_unordered_map<std::string , tbb::concurrent_unordered_map<std::string , tbb::concurrent_vector<cv::Point3f> > > ,tbb::flow::queueing> {
private:
	Matrices matrices;
	bool canTransform;
public:
	CoordinateTransformer(tbb::flow::graph& g):Processor<ImageProcessingResult,
	tbb::concurrent_unordered_map<std::string , tbb::concurrent_unordered_map<std::string , tbb::concurrent_vector<cv::Point3f> > >,
	tbb::flow::queueing>(g , tbb::flow::unlimited){};

	bool loadMatrices(std::string path);

	tbb::concurrent_unordered_map<std::string , tbb::concurrent_unordered_map<std::string , tbb::concurrent_vector<cv::Point3f> > >
		process(ImageProcessingResult ipData);

	virtual ~CoordinateTransformer();
};

#endif /* COORDINATETRANSFORMER_H_ */
