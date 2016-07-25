/*
 * IRObject.cpp
 *
 *  Created on: 2016. jún. 30.
 *      Author: Máté
 */

#include "IRObject.h"

IRObject::IRObject() {
}

std::pair<bool, std::vector<cv::Point2f> > IRObject::findMatch(std::vector<cv::Point2f> points) {

	std::vector<cv::Point2f> matchPoints;
	std::vector<cv::Point2f> actualPoints;


		std::sort(points.begin(), points.end(),
				[](const cv::Point2f &a , const cv::Point2f &b)-> bool {return a.x<b.x;});

		for (auto i = 0; i < points.size() - 1; i++) {
			if (fabs(points[i].x - points[i + 1].x) < 10) {
				if (points[i].y < points[i + 1].y) {
					auto temp = points[i];
					points[i] = points[i + 1];
					points[i + 1] = temp;
				}
			}
		}

		for(cv::Point2f p : points){

		}

		for (auto i = 0; i < points.size(); i++) {
			cv::Point2f reference = points[i];
			actualPoints.clear();

			int matchNo = 1;

			cv::Point2f lastMatchPoint = points[i];

			for (auto j = i + 1; j < points.size(); j++) {

				bool match = false;
				auto refPos = markers[markerIds[matchNo]].getReferences();

				switch (refPos.first) {
				case UPPER:
					match = (reference.y - points[j].y) > 15;
					break;
				case LOWER:
					match = (points[j].y - reference.y) > 15;
					break;
				case IN_ROW:
					match = fabs(points[j].y - reference.y) < 15;
					break;
				}
				switch (refPos.second) {
				case UPPER:
					match = match && (points[j - 1].y - points[j].y) > 15;
					break;
				case LOWER:
					match = match && (points[j].y - points[j - 1].y) > 15;
					break;
				case IN_ROW:
					match = match && fabs(points[j].y - lastMatchPoint.y) < 15;
					break;
				}
				if (match) {
					lastMatchPoint = points[j];
					actualPoints.push_back(points[j]);
					matchNo++;
					if (matchNo == numberOfParts) {
						actualPoints.insert(actualPoints.begin(), reference);
						return std::pair<bool, std::vector<cv::Point2f> >(true, actualPoints);
					}
				}
			}
		}

	return std::pair<bool, std::vector<cv::Point2f> >(false, actualPoints);
}


void IRObject::track(Frame frame, Frame prevFrame) {

	for (auto i = 0; i < markerIds.size(); i++) {

		markers[markerIds[i]].refreshPosition(frame, prevFrame);

		if (markers[markerIds[i]].isLost()) {
			tracked = false;
		}

		for (auto i = 0; i < markerIds.size() - 1; i++) {
			if (markers[markerIds[i]].getPosition().left.x
					> markers[markerIds[i + 1]].getPosition().left.x) {
				auto tmp = markerIds[i];
				markerIds[i] = markerIds[i + 1];
				markerIds[i + 1] = tmp;
			}
		}

		for (auto i = 0; i < markerIds.size() - 1; i++) {
			if (fabs(
					markers[markerIds[i]].getPosition().left.x
							- markers[markerIds[i + 1]].getPosition().left.x)
					< 10) {
				if (markers[markerIds[i]].getPosition().left.y
						< markers[markerIds[i + 1]].getPosition().left.y) {
					auto tmp = markerIds[i];
					markerIds[i] = markerIds[i + 1];
					markerIds[i + 1] = tmp;
				}
			}

		}

		auto reference = markers[markerIds[0]].getPosition();

		auto matrices = camera.getCameraMatrices();

		auto refReal = markers[markerIds[0]].getRealPosition(matrices.leftCamMatrix,
				matrices.rightCamMatrix, matrices.r1, matrices.r2, matrices.p1, matrices.p2, matrices.leftDistCoeffs,
				matrices.rightDistCoeffs);

		for (auto i = 1; i < markerIds.size(); i++) {
			if (tracked) {

				Marker m = markers[markerIds[i]];
				ReferencePosition fromPrev;
				ReferencePosition fromRef;

				cv::Point3f mReal = m.getRealPosition(matrices.leftCamMatrix, matrices.rightCamMatrix,
						matrices.r1, matrices.r2, matrices.p1, matrices.p2, matrices.leftDistCoeffs, matrices.rightDistCoeffs);



				auto referenceDistance = sqrtf(
						powf(mReal.x - refReal.x, 2.0)
								+ powf(mReal.y - refReal.y, 2.0)
								+ powf(mReal.z - refReal.z, 2.0));

				float verticalDistance = reference.left.y
						- m.getPosition().left.y;

				if (verticalDistance > 15) {
					fromRef = UPPER;
				} else if (verticalDistance < -15) {
					fromRef = LOWER;
				} else {
					fromRef = IN_ROW;
				}

				verticalDistance =
						markers[markerIds[i - 1]].getPosition().left.y
								- m.getPosition().left.y;
				if (verticalDistance > 15) {
					fromPrev = UPPER;
				} else if (verticalDistance < -15) {
					fromPrev = LOWER;
				} else {
					fromPrev = IN_ROW;
				}

				markers[markerIds[i]].updateReference(fromPrev, fromRef,
						referenceDistance);
			}

		}
	}

}

std::pair<std::vector<int>, std::vector<int>> IRObject::detect(PointSet points,std::pair<std::vector< std::vector<cv::Point> > ,std::vector< std::vector<cv::Point> > > contourSet,std::pair<std::vector<int> , std::vector<int> > identifiers) {

	std::vector<int> left;
	std::vector<int> right;
	std::pair<std::vector<int>, std::vector<int>> MatchPointIdx;
	MatchPointIdx.first = left;
	MatchPointIdx.second = right;

	if(points.left.size()==0 || points.right.size()==0){
		tracked = false;
		return MatchPointIdx;
	}

	auto matrices = camera.getCameraMatrices();

	int c=0;
	while (!tracked) {
		c++;

		MatchPointIdx.first.clear();
		MatchPointIdx.second.clear();

		if(c==20)break;

		auto leftMatchPoints = findMatch(points.left);
		auto rightMatchPoints = findMatch(points.right);

		if (leftMatchPoints.first && rightMatchPoints.first) {

			tracked = true;

			std::cout<<"foundObject"<<std::endl;

			StereoPoint newPosition;
			newPosition.left = leftMatchPoints.second[0];
			newPosition.right = rightMatchPoints.second[0];

			PointSet pointset;

			for(size_t i = 0 ; i<contourSet.first.size() ; i++){
				if(cv::pointPolygonTest(contourSet.first[i] , newPosition.left , false )>0){
					for(size_t k = 0 ; k<contourSet.first[i].size() ; k++){
						pointset.left.push_back(cv::Point2f(contourSet.first[i][k].x,contourSet.first[i][k].y));
					}
					break;
				}
			}

			for(size_t i = 0 ; i<contourSet.second.size() ; i++){
				if(cv::pointPolygonTest(contourSet.second[i] , newPosition.right , false )>0){
					for(size_t k = 0 ; k<contourSet.second[i].size() ; k++){
						pointset.right.push_back(cv::Point2f(contourSet.second[i][k].x,contourSet.second[i][k].y));
					}
					break;
				}
			}

			markers[markerIds[0]].setPosition(newPosition);

			markers[markerIds[0]].setPosition(pointset);



			auto refPosition = markers[markerIds[0]].getRealPosition(
					matrices.leftCamMatrix, matrices.rightCamMatrix, matrices.r1, matrices.r2, matrices.p1, matrices.p2,
					matrices.leftDistCoeffs, matrices.rightDistCoeffs);

			MatchPointIdx.second.push_back(
					findIndex(points.right, rightMatchPoints.second[0]));
			MatchPointIdx.first.push_back(
					findIndex(points.left, leftMatchPoints.second[0]));

			for (auto i = 1; i < markerIds.size(); i++) {
				StereoPoint newPosition;
				newPosition.left = leftMatchPoints.second[i];
				newPosition.right = rightMatchPoints.second[i];

				PointSet pointset;

				for(size_t j = 0 ; j<contourSet.first.size() ; j++){
					if(cv::pointPolygonTest(contourSet.first[j] , newPosition.left , false )>0){
						for(size_t k = 0 ; k<contourSet.first[j].size() ; k++){
							pointset.left.push_back(cv::Point2f(contourSet.first[j][k].x,contourSet.first[j][k].y));
						}
						break;
					}
				}

				for(size_t j = 0 ; j<contourSet.second.size() ; j++){
					if(cv::pointPolygonTest(contourSet.second[j] , newPosition.right , false )>0){
						for(size_t k = 0 ; k<contourSet.second[j].size() ; k++){
							pointset.right.push_back(cv::Point2f(contourSet.second[j][k].x,contourSet.second[j][k].y));
						}
						break;
					}
				}

				markers[markerIds[i]].setPosition(newPosition);

				markers[markerIds[i]].setPosition(pointset);

				auto position = markers[markerIds[i]].getRealPosition(
						matrices.leftCamMatrix, matrices.rightCamMatrix, matrices.r1, matrices.r2, matrices.p1, matrices.p2,
						matrices.leftDistCoeffs, matrices.rightDistCoeffs);
				auto distanceFromRef = sqrtf(
						powf(position.x - refPosition.x, 2.0)
								+ powf(position.y - refPosition.y, 2.0)
								+ powf(position.z - refPosition.z, 2.0));

				if (fabs(
						distanceFromRef
								- markers[markerIds[i]].getReferenceDistance())
						> 3.0f) {

					std::cout<<distanceFromRef<<std::endl;

					auto leftIdx = findIndex(points.left,
							markers[markerIds[i]].getPosition().left);
					auto rightIdx = findIndex(points.right,
							markers[markerIds[i]].getPosition().right);

						if(points.left.size()>leftIdx){
							points.left.erase(points.left.begin() + leftIdx);
						}

						if(points.right.size()>rightIdx){
							points.right.erase(points.right.begin() + rightIdx);
						}


					tracked = false;

					if (points.left.size() == 0 || points.right.size() == 0) {
						break;
					}


				} else {
					MatchPointIdx.second.push_back(
							findIndex(points.right,
									rightMatchPoints.second[i]));
					MatchPointIdx.first.push_back(
							findIndex(points.left, leftMatchPoints.second[i]));
				}
			}
			}else{
				tracked = false;
			}
		}

		if(!tracked){
			MatchPointIdx.first.clear();
			MatchPointIdx.second.clear();
		}

		return MatchPointIdx;

	}


IRObject::~IRObject() {
	// TODO Auto-generated destructor stub
}

