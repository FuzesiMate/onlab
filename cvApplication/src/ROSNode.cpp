#include "ROSNode.h"

ROSNode::Node(){
  ros::init(argc, argv, "cameraInfoPublisher");
}

void ROSNode::initializeNode(){
  nh = new ros::NodeHandle();
  trainStream_pub = nh->advertise<hu_bme_mit_cpp_localsearch_robot::TrainPose>("trainStream", 1000);
  robotStream_pub = nh->advertise<hu_bme_mit_cpp_localsearch_robot::RobotPartsPose>("robotStream", 1000);
}

void ROSNode::~Node(){
  ros::shutdown();
  delete nh;
}

void ROSNode::sendDataOverROS(std::string topic){
	hu_bme_mit_cpp_localsearch_robot::RobotPartsPose robotPartsPoseMsg;
	hu_bme_mit_cpp_localsearch_robot::TrainPose trainPosMsg;

	auto objectIds = model.getObjectIds();
	for(auto i = 0; i < objectIds.size(); i++){
		auto markerIds = model.getMarkerIds(objectIds[i]);
		auto position = this->getObjectPosition(objectIds[i]);
		for(auto j = 0; j < markerIds.size(); j++){
			auto markerpos = position[markerIds[j]];
			//Filter to Robot positions
			if(objectIds[i].compare("robot_arm") && markerIds[j].compare("")){//TODO MarkerID
				::geometry_msgs::Point p1;
			  p1.x = markerpos.x; p1.y = markerpos.x; p1.z = markerpos.x;
				robotPartsPoseMsg.p1=p1;
			}
			if(objectIds[i].compare("robot_arm") && markerIds[j].compare("")){//TODO MarkerID
				::geometry_msgs::Point p2;
			  p2.x = markerpos.x; p2.y = markerpos.x; p2.z = markerpos.x;
				robotPartsPoseMsg.p2=p2;
			}
			if(objectIds[i].compare("robot_arm") && markerIds[j].compare("")){//TODO MarkerID
				::geometry_msgs::Point p3;
			  p3.x = markerpos.x; p3.y = markerpos.x; p3.z = markerpos.x;
				robotPartsPoseMsg.p3=p3;
			}
			if(objectIds[i].compare("train")){
				::geometry_msgs::Point p; p.x=markerpos.x;p.y=markerpos.y;p.z=markerpos.z;
		    trainPosMsg.poses.push_back(p);
			}
		}
	}
	robotStream_pub.publish(robotPartsPoseMsg);
	trainStream_pub.publish(trainPosMsg);
}
