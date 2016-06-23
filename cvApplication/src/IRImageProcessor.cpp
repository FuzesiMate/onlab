#include "IRImageProcessor.h"
#include<opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/video.hpp>
#include <iostream>
#include "IImageProcessor.h"

using namespace std;
using namespace cv;

IRImageProcessor::IRImageProcessor(){

}

void IRImageProcessor::setWindow(std::string winname){
	this->windowName = winname;
	namedWindow(windowName);
	thresholdValue=0;
	createTrackbar("threshold" , windowName , &thresholdValue , 255);
}

std::vector< std::vector<cv::Point> > IRImageProcessor::processImage(Mat frame){
		vector<Point2f> points;

		vector<Vec4i> hierarchy;
		Mat processed;

		std::vector< std::vector<cv::Point> > contours;

		threshold(frame , processed  , thresholdValue, 255 , CV_THRESH_BINARY);

		blur(processed,processed, Size(5,5));

		Mat element = getStructuringElement(MORPH_RECT , Size(5,5));
		morphologyEx(processed,processed,MORPH_CLOSE,element);
		morphologyEx(processed,processed,MORPH_OPEN,element);

		Mat cont = Mat::zeros( processed.size(), CV_8UC3 );
		findContours(processed,contours,hierarchy,CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

		for(size_t i = 0 ; i<contours.size() ; i++){
			drawContours(cont , contours ,  i , Scalar(0,0,255) , 2);
		}

		resize(cont,cont,Size(640,480));
		imshow(windowName , cont);

	return contours;
}

void IRImageProcessor::setFilterValues(boost::property_tree::ptree propertyTree){
	thresholdValue = propertyTree.get<int>(THRESH_VALUE);
}

IRImageProcessor::~IRImageProcessor(){}
