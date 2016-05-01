/*
 * stereoCalibrate.cpp
 *
 *  Created on: Jan 27, 2016
 *      Author: teqbox
 */

#include "opencv2/calib3d.hpp"
#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <opencv2/videoio.hpp>
#include <iostream>

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

int main(int argc , char *argv[]){
	string left_path;
	string right_path;
	Rect left_ROI , right_ROI;

	if(argc<3){
		std::cout<<"too few arguments"<<std::endl;
		return -1;
	}else{
		left_path = argv[1];
		right_path = argv[2];
	}

	char calib;
	calib = 'x';

	VideoCapture left_player;
	VideoCapture right_player;

	Mat r1,r2,p1,p2,q;
	Mat left_camMatrix;
	Mat left_distCoeffs;
	Mat right_camMatrix;
	Mat right_distCoeffs;
	Mat left,right;

	bool found_left;
	bool found_right;

	vector< vector<Point3f> > obj_points;

	vector< vector<Point2f> > left_img_points;
	vector< vector<Point2f> > right_img_points;

	FileStorage file("left_camMatrix.yml" , FileStorage::READ);

	file["camMatrix"]>>left_camMatrix;
	file["distCoeffs"]>>left_distCoeffs;

	file.release();

	FileStorage afile("right_camMatrix.yml" , FileStorage::READ);

	afile["camMatrix"]>>right_camMatrix;
	afile["distCoeffs"]>>right_distCoeffs;


	left_player.open(left_path.c_str());
	right_player.open(right_path.c_str());

	afile.release() ;


	Mat grayleft, grayright;

	vector <Point2f> left_corners;
	vector <Point2f> right_corners;
	if(calib=='c'){
	char comm;
	bool start = false;

	while(left_player.read(left) && right_player.read(right)){

		cvtColor(left ,grayleft , CV_RGB2GRAY);
		cvtColor(right ,grayright , CV_RGB2GRAY);

		if(start){
			found_left =findChessboard(grayleft , left_corners , Size(6,3));
			found_right=findChessboard(grayright , right_corners , Size(6,3));

			if(found_left && found_right){
				drawChessboardCorners(left , Size(6,3) , left_corners , found_left);
				drawChessboardCorners(right , Size(6,3) , right_corners , found_right);

				vector<Point3f>obj;
				for(int i = 0 ; i<3 ; i++){
					for(int j = 0 ; j<6 ; j++)
					obj.push_back(Point3f(i*2.36 , j*2.36 , 0.0f));
				}

				obj_points.push_back(obj);
				left_img_points.push_back(left_corners);
				right_img_points.push_back(right_corners);
			}
		}

		resize(left ,left , Size(640,480));
		resize(right ,right , Size(640,480));

		imshow("left" , left);
		imshow("right" , right);



		comm=waitKey(5);
		if(comm=='q' && start == false){
			start = true;
		}else if(comm=='q' && start == true){
			start = false;
		}else if(comm=='x'){
			left_player.release();
			right_player.release();
			break;
		}
	}


		Mat r,t,e,f;


	stereoCalibrate(obj_points , left_img_points , right_img_points , left_camMatrix , left_distCoeffs ,
			right_camMatrix , right_distCoeffs , Size(1280,1024) , r,t,e,f , CALIB_FIX_INTRINSIC|CALIB_SAME_FOCAL_LENGTH | CALIB_RATIONAL_MODEL);


	Mat h1 , h2;

	vector<Point2f> all_left;
	vector<Point2f> all_right;

	for(size_t i = 0 ; i<left_img_points.size(); i++){
		for(size_t j = 0 ; j<left_img_points[i].size() ; j++){
			all_left.push_back(left_img_points[i][j]);
		}
	}

	for(size_t i = 0 ; i<right_img_points.size(); i++){
			for(size_t j = 0 ; j<right_img_points[i].size() ; j++){
				all_right.push_back(right_img_points[i][j]);
			}
		}


//	stereoRectifyUncalibrated(all_left ,all_right , f , Size(1280,1024) , h1 , h2);


	stereoRectify(left_camMatrix , left_distCoeffs , right_camMatrix , right_distCoeffs , Size(1280,1024) , r,t,r1,r2,p1,p2,q ,0,
			-1 , Size() , &left_ROI  , &right_ROI);


	}

	left_player = VideoCapture();
	right_player = VideoCapture();

	left_player.open(left_path.c_str());
	right_player.open(right_path.c_str());

if(calib!='c'){
	FileStorage input("matrices.yml" , FileStorage::READ);

	input["r1"]>>r1;
	input["p1"]>>p1;
	input["r2"]>>r2;
	input["p2"]>>p2;
	input["left_camMatrix"]>>left_camMatrix;
	input["right_camMatrix"]>>right_camMatrix;
	input["left_distCoeffs"]>>left_distCoeffs;
	input["right_distCoeffs"]>>right_distCoeffs;
	input["q"]>>q;
	input.release();
}

	   Mat map11, map12, map21, map22;
	   initUndistortRectifyMap(left_camMatrix, left_distCoeffs, r1, p1, Size(1280,1024), CV_16SC2, map11, map12);
	   initUndistortRectifyMap(right_camMatrix, right_distCoeffs, r2, p2, Size(1280,1024), CV_16SC2, map21, map22);

	   Mat cord;

	Mat uleft,uright;

	char save;

	while(left_player.read(left)&&
	right_player.read(right)){



		remap(left,uleft , map11 ,map12 , INTER_LINEAR);
		remap(right,uright, map21,map22 , INTER_LINEAR);
	//	undistort(left , uleft , left_camMatrix , left_distCoeffs);
	//undistort(right,uright,right_camMatrix , right_distCoeffs);
if(save=='q'){
		cvtColor(left ,grayleft , CV_RGB2GRAY);
		cvtColor(right ,grayright , CV_RGB2GRAY);

		found_left =findChessboard(grayleft , left_corners , Size(6,3));
		found_right=findChessboard(grayright , right_corners , Size(6,3));

		if(found_left && found_right){

		drawChessboardCorners(uleft , Size(6,3) , left_corners , found_left);
		drawChessboardCorners(uright , Size(6,3) , right_corners , found_right);

		undistortPoints(left_corners, left_corners , left_camMatrix , left_distCoeffs , r1 , p1);
		undistortPoints(right_corners , right_corners , right_camMatrix , right_distCoeffs , r2 , p2 );

/*

		float px = 283/fabs(left_corners[0].x-left_corners[5].x);
		cout<<px<<endl;
		float disloc = fabs(left_corners[0].x-right_corners[0].x);
		float z = (380*3.7)/(disloc*(px/2));
		cout<<z<<endl;
*/

		triangulatePoints(p1,p2,left_corners , right_corners , cord);


			vector<Point3f> image;
			vector<Point3f> real;
/*
			for(size_t i= 0 ; i<left_corners.size() ; i++){
				float disloc;
				disloc = fabs(left_corners[i].x-right_corners[i].x);
				image.push_back(Point3f(left_corners[i].x , left_corners[i].y , disloc));
			}

			perspectiveTransform(image , real , q);
			cout<<"found points:"<<endl;
			for(size_t i = 0 ; i<real.size(); i++){
				cout<<"x: "<<real[i].x<<" y: "<<real[i].y<<" z: "<<real[i].z<<endl;
			}
			cout<<"points end"<<endl<<endl;
*/

			for(int i = 0 ; i<cord.cols ; i++){

				  float w = cord.at<float>(3,i);
				  float x = cord.at<float>(0,i)/w;
				  float y = cord.at<float>(1,i)/w;
				  float z = cord.at<float>(2,i)/(w);
				  cout<<"x: "<< x <<" y: "<<y<<" z: "<<z<<endl;
			}


		}


}
		rectangle(uleft , left_ROI , Scalar(255,0,0));
		rectangle(uright , right_ROI , Scalar(255,0,0));
		imshow("left" , uleft);
		imshow("right" ,uright);

		save = waitKey(10);
		if(save=='s'){
			FileStorage save("matrices.yml" , FileStorage::WRITE);
			save<<"left_camMatrix"<<left_camMatrix<<"left_distCoeffs"<<left_distCoeffs;
			save<<"right_camMatrix"<<right_camMatrix<<"right_distCoeffs"<<right_distCoeffs;
			save<<"r1"<<r1<<"r2"<<r2<<"p1"<<p1<<"p2"<<p2<<"q"<<q;
		}
	}

	waitKey(0);

	left_player.release();
	right_player.release();

	return 0;
}


