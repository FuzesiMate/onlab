#include "Camera.h"
#include <opencv2/imgproc.hpp>
#include <fstream>
#include <cmath>

//#define LOG

bool Camera::provide(Frame &frame) {
	
	auto time = std::chrono::steady_clock::now();
	auto currentTimestamp =
			std::chrono::duration_cast<std::chrono::milliseconds>(
					time.time_since_epoch()).count();

	auto delay = ((1000/fps) - (currentTimestamp - lastTimestamp));

	if (delay > 0 && lastTimestamp!=0) {
		std::this_thread::sleep_for(std::chrono::milliseconds(delay));
	}
	time = std::chrono::steady_clock::now();
	currentTimestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
			time.time_since_epoch()).count();

	frame.images = tbb::concurrent_vector<cv::Mat>(numberOfCameras);

	tbb::parallel_for(size_t(0), cameras.size(), [&](size_t i ) {
		bool success = cameras[i].grab();
		if(!success){
			std::cout<<"grab failed"<<std::endl;
		}
	});
	tbb::parallel_for(size_t(0), cameras.size(), [&](size_t i ) {
		cameras[i].retrieve(frame.images[i]);
	});
	int64_t currentFps;
	if((currentTimestamp-lastTimestamp)>0){
		currentFps = (float)1000/(currentTimestamp-lastTimestamp);
	}
	std::stringstream fpsstring;
	fpsstring <<"Current fps: "<<currentFps;
	frame.fps = (int)currentFps;

#ifdef LOG
	ofs << fps << ";" << currentFps << std::endl;
#endif	

	for(auto& i : frame.images){
		cv::putText(i , fpsstring.str() , cv::Point(100,100) , cv::FONT_HERSHEY_SIMPLEX ,1.0 ,cv::Scalar(255,255,255) , 2);
	}
	frame.frameIndex = frameCounter;
	frame.timestamp = currentTimestamp;
	frameCounter++;
	lastTimestamp = currentTimestamp;
	
	if (!providing) {
		std::cout << "Stop recording..." << std::endl;
		for (auto c : cameras) {
			c.release();
		}
		#ifdef LOG
			ofs.close();
		#endif // LOG
	}
#ifdef LOG
	/*ONLY FOR BENCHMARKING*/
	if (frameCounter % 20 == 0) {
		fps++;
	}

	if (fps == 31) {
		ofs.close();
		return false;
	}

#endif
	return providing;
}

bool Camera::initialize(int cameraType) {

#ifdef LOG
	ofs.open("fps_ximea.csv");
	ofs << "Required fps;Measured fps" << std::endl;
#endif
	cameras = std::vector<cv::VideoCapture>(numberOfCameras);
	for(int i = 0 ; i<numberOfCameras ; i++){
		if(!cameras[i].open(cameraType+i)){
			return false;
		}
		cameras[i].set(cv::CAP_PROP_XI_AEAG , 0.0);
		cameras[i].set(cv::CAP_PROP_XI_EXPOSURE , exposure);
		cameras[i].set(cv::CAP_PROP_XI_GAIN , gain);
	}
	return true;
}

void Camera::reconfigure(boost::property_tree::ptree config) {
	int fps = config.get<int>(FPS);
	int exp = config.get<int>(EXPOSURE);
	float gain = config.get<float>(GAIN);

	this->fps = fps;
	for (auto& camera : cameras) {
		camera.set(cv::CAP_PROP_XI_EXPOSURE, exp);
		camera.set(cv::CAP_PROP_XI_GAIN, gain);
	}
}

Camera::~Camera() {
	for(auto c : cameras){
		c.release();
	}
}

