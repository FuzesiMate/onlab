/*
 * CoordinateTransformer.cpp
 *
 *  Created on: Sep 19, 2016
 *      Author: teqbox
 */

#include "CoordinateTransformer.h"
#include <opencv2/calib3d.hpp>
#include <opencv2/imgproc.hpp>

bool CoordinateTransformer::loadMatrices(std::string path){
	cv::FileStorage file;
	if(!file.open(path , cv::FileStorage::READ)){
		return false;
	}

	cv::Mat temp;
	file["m1"]>>temp;
	matrices.cameraMatrix.push_back(temp.clone());
	file["m2"]>>temp;
	matrices.cameraMatrix.push_back(temp.clone());
	file["d1"]>>temp;
	matrices.distCoeffs.push_back(temp.clone());
	file["d2"]>>temp;
	matrices.distCoeffs.push_back(temp.clone());
	file["p1"]>>temp;
	matrices.projectionMatrix.push_back(temp.clone());
	file["p2"]>>temp;
	matrices.projectionMatrix.push_back(temp.clone());
	file["r1"]>>temp;
	matrices.rectificationMatrix.push_back(temp.clone());
	file["r2"]>>temp;
	matrices.rectificationMatrix.push_back(temp.clone());

	file.release();

	canTransform = true;
	return true;
}

ModelData CoordinateTransformer::process(ModelData modelData){
	tbb::concurrent_unordered_map<std::string , tbb::concurrent_unordered_map<std::string , tbb::concurrent_vector<cv::Point3f> > > transformed;

	for(auto& objectData : modelData.objectData){
		for(auto& markerData : objectData.markerData){
			if(markerData.screenPosition.size()==2 && markerData.tracked[0] && markerData.tracked[1]){

				std::vector<cv::Point2f> p1{markerData.screenPosition[0]};
				std::vector<cv::Point2f> p2{markerData.screenPosition[1]};

				cv::undistortPoints(p1, p1 , matrices.cameraMatrix[0] , matrices.distCoeffs[0] , matrices.rectificationMatrix[0] , matrices.projectionMatrix[0]);
				cv::undistortPoints(p2, p2 , matrices.cameraMatrix[1] , matrices.distCoeffs[1] , matrices.rectificationMatrix[1] , matrices.projectionMatrix[1]);

				cv::Mat cord;
				cv::triangulatePoints(matrices.projectionMatrix[0] , matrices.projectionMatrix[1] , p1 , p2 , cord);

				float x,y,z;

				for(int i = 0 ; i<cord.cols ; i++){

					float w = cord.at<float>(3,i);
					x = cord.at<float>(0,i)/w;
					y = cord.at<float>(1,i)/w;
					z = cord.at<float>(2,i)/w;
				}
				markerData.realPosition = cv::Point3f(x,y,z);
			}
		}
	}

	return modelData;
}
