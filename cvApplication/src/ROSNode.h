#include "ros/ros.h"
//ROS communication messages
#include <geometry_msgs/Point.h>
#include <hu_bme_mit_cpp_localsearch_robot/RobotPartsPose.h>
#include <hu_bme_mit_cpp_localsearch_robot/TrainPose.h>

namespace ROSNode {

  class Node{
    ros::Publisher trainStream_pub;
    ros::Publisher robotStream_pub;
    ros::NodeHandle* nh;

    Node();
    ~Node();

    void cleanUp();
    void initializeNode();
    void sendDataOverROS();
  }
}
