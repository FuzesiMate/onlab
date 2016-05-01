//============================================================================
// Name        : chessboardDetector.cpp
// Author      :
// Version     :
// Copyright   :
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include "opencv2/highgui.hpp"
#include "opencv2/core.hpp"
#include "opencv2/calib3d.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/video.hpp"
#include "opencv2/videoio.hpp"
#include "math.h"

using namespace std;
using namespace cv;


bool findChessboard(Mat frame , vector<Point2f>& corners , Size boardsize){

	bool found = false;
	found =findChessboardCorners(frame , boardsize , corners , CALIB_CB_ADAPTIVE_THRESH + CALIB_CB_NORMALIZE_IMAGE);
	if(found){
		cornerSubPix(frame, corners, Size(11, 11), Size(-1, -1),
				TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));
	}
	return found;
}

void calibrate(Size imgsize ,vector<vector<Point3f> > object_points , vector<vector<Point2f> > img_points , Mat& camMatrix , Mat& distCoeffs , vector<Mat>& rvec , vector<Mat>& tvec){
	 camMatrix = Mat(3, 3, CV_64FC1);
	 //camMatrix.ptr<float>(0)[0] = 1;
	 //camMatrix.ptr<float>(1)[1] = 1;
	 calibrateCamera(object_points, img_points, imgsize, camMatrix, distCoeffs, rvec, tvec , CV_CALIB_RATIONAL_MODEL);
}

int main(int argc , char *argv[]) {

	VideoCapture video;
	video.open(argv[1]);

	vector<vector<Point2f> > img_points;
	vector<vector<Point3f> > obj_points;

	Mat camMatrix;
	Mat distCoeff;
	vector<Mat> rvec;
	vector<Mat> tvec;


	Mat frame;

	char start;
	bool found = false;
	bool search = false;

	Size imgsize;

	while(video.read(frame)){


		cvtColor(frame,frame , CV_BGR2GRAY);

		imgsize = frame.size();



		if(search){
			vector<Point2f> corners;
			found = findChessboard(frame , corners , Size(6,3));

			if(found){
				drawChessboardCorners(frame, Size(6,3), Mat(corners), found);

				vector<Point3f>obj;
				for(int i = 0 ; i<3 ; i++){
					for(int j = 0 ; j<6 ; j++){
					obj.push_back(Point3f(i*2.36 , j*2.36 , 0.0f));
					}
				}

				obj_points.push_back(obj);
				img_points.push_back(corners);

			}

		}

		resize(frame,frame, Size(1280/2 , 1024/2));
		//resize(right,right, Size(1280/2 , 1024/2));

		imshow("video" , frame);

		start=waitKey(50);
		if(start=='q' && search == false){
			search = true;
		}else if(start=='q' && search == true){
			search = false;
		}else if(start=='x'){
			video.release();
			break;
		}
	}

	calibrate(imgsize , obj_points , img_points , camMatrix , distCoeff , rvec , tvec);


	double fovx , fovy , focallength , aspect;
	Point2d principal;

	calibrationMatrixValues(camMatrix , Size(1280,1024) , 6.9 , 5.5 , fovx , fovy , focallength , principal , aspect );

	cout<<"fovx: "<<fovx<<" fovy: "<<fovy<<" focal: "<<focallength<<" aspect: "<<aspect<<" principal.x: "<<principal.x<<
	" principal.y: "<<principal.y<< endl;

	video = VideoCapture();
	video.open(argv[1]);

	Mat undistorted;
	char save;
	while(video.read(frame)){

		undistort(frame , undistorted , camMatrix , distCoeff);
		imshow("original" , frame);
		imshow("undistorted" , undistorted);
		save =waitKey(5);
		if(save =='s'){
			FileStorage file("right_camMatrix.yml" , FileStorage::WRITE);
			file<<"camMatrix"<<camMatrix<<"distCoeffs"<<distCoeff;
			file.release();
		}
	}

	return 0;
}



