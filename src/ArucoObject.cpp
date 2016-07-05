/*
 * ArucoObject.cpp
 *
 *  Created on: 2016. jún. 30.
 *      Author: Máté
 */

#include "ArucoObject.h"

using namespace std;

ArucoObject::ArucoObject() {
}

std::pair<int,int> ArucoObject::findMatch(std::string markerId , std::pair<std::vector<int> , std::vector<int> > identifiers){
	int leftIdentifier= -1;
	int rightIdentifier= -1;

	for(size_t i = 0 ; i<identifiers.first.size() ; i++){
		if(identifiers.first[i] == markers[markerId].getId()){
			leftIdentifier = i;
		}
	}

	for(size_t i = 0 ; i<identifiers.second.size() ; i++){
		if(identifiers.second[i] == markers[markerId].getId()){
			rightIdentifier = i;
		}
	}

	return std::make_pair(leftIdentifier , rightIdentifier);
}

std::pair<std::vector<int> ,std::vector<int>> ArucoObject::detect(PointSet points,std::pair<std::vector< std::vector<cv::Point> > ,std::vector< std::vector<cv::Point> > > contourSet , std::pair<std::vector<int> , std::vector<int> > identifiers){

	for(auto m : markerIds){
			cout<<"findMatches"<<endl;
					auto indices = findMatch(m , identifiers);
					cout<<"finadmatches end"<<endl;

					cout<<"set positions"<<endl;
					if(indices.first>=0 && indices.second>=0){
						StereoPoint pos;
						pos.left = points.left[indices.first];
						pos.right = points.right[indices.second];
						markers[m].setPosition(pos);
					}else{
						markers[m].setLost(true);
						tracked = false;
					}
					if(!markers[m].isLost()){
						tracked = true;
					}
	}

	std::vector<int> a;
	std::vector<int> b;

	a.push_back(1);
	b.push_back(2);

	return std::make_pair(a,b);
}

void ArucoObject::track(Frame frame , Frame prevFrame){
	for(auto m : markerIds){
		markers[m].refreshPosition(frame , prevFrame);
	}
}

ArucoObject::~ArucoObject() {
	// TODO Auto-generated destructor stub
}

