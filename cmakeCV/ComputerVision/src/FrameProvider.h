/*
 * FrameProvider.h
 *
 *  Created on: Oct 7, 2016
 *      Author: teqbox
 */

#ifndef FRAMEPROVIDER_H_
#define FRAMEPROVIDER_H_

#include "DataTypes.h"
#include "Provider.h"

class FrameProvider: public Provider<Frame> {
public:
	FrameProvider(tbb::flow::graph& g): Provider<Frame>(g){}

	virtual bool provide(Frame& images) = 0;
	virtual bool init(int cameraType){return false;};
	virtual bool init(std::string path){return false;};

	virtual void setFPS(int fps) = 0;
	virtual void setExposure(int exposure){};
	virtual void setGain(float gain){};


	virtual ~FrameProvider() = default;
};

#endif /* FRAMEPROVIDER_H_ */
