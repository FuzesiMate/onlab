#include "ImageProcessor.h"
#include<opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/video.hpp>
#include <iostream>

using namespace std;
using namespace cv;

ImageProcessor::ImageProcessor(string windowName){
	this->windowName = windowName;
	namedWindow(windowName);
	thresholdValue=200;
	createTrackbar("threshold" , windowName , &thresholdValue , 255);
}

vector<Point2f> ImageProcessor::processImage(Mat frame){
		vector<Point2f> points;
		vector< vector<Point> > contours;
		vector<Vec4i> hierarchy;
		Mat processed;

		threshold(frame , processed  , thresholdValue, 255 , CV_THRESH_BINARY);

		blur(frame,frame, Size(5,5));

		Mat element = getStructuringElement(MORPH_RECT , Size(5,5));
		morphologyEx(processed,processed,MORPH_CLOSE,element);
		morphologyEx(processed,processed,MORPH_OPEN,element);

		Mat cont = Mat::zeros( processed.size(), CV_8UC3 );
		findContours(processed,contours,hierarchy,CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

		Point2f center;
		float radius;
		for(auto i = 0 ; i<contours.size() ; i++){

			approxPolyDP(contours[i], contours[i] , 2 , false);

			if(contourArea(contours[i])<500){
				minEnclosingCircle(contours[i] , center , radius);
				if(radius>1 && radius<20){
					points.push_back(center);
					cv::drawContours(cont, contours, i , Scalar(0,0,255) , 2);
				}
			}
		}

		resize(cont,cont,Size(640,480));
		imshow(windowName , cont);

	return points;
}
