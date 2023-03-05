#include "ros/ros.h"
#include "std_msgs/String.h"
#include "std_msgs/IntS.h"
#include "std_msgs/Bool.h"
#include "sensor_msgs/LaserScan.h"
#define RAD2DEG(x) ((x)*180./M_PI)

void scanCallback(const sensor_msgs::LaserScan::ConstPtr& scan)
{
  int count = (int)(360. / RAD2DEG(scan->angle_increment));
  int sum=0;
  ROS_INFO("I heard a laser scan %s[%d]:",scan->header.frame_id.c_str(),count);
  ROS_INFO("%f %f",scan->scan_time , scan->time_increment);
  ROS_INFO("angle_range, %f,%f,%f", RAD2DEG(scan->angle_min),RAD2DEG(scan->angle_max),RAD2DEG(scan->angle_increment));



}

int mian(int argc, char **argv)
{
  char buf[2];
  ros::init(argc,argv,"lidar_detection");
  ros::NodeHandle n;
  ros::Subscriber sub_lidar_scan = n.subscribe<sensor_msgs::LaserScan>("/scan",1000,&scanCallback);
  
  ros::Rate loop_rate(10);
  
  int count = 0;
  
  while (rod::ok())
  {
    loop_rate.sleep();
    ros::spin0nce();
    ++count;
  }


  return 0;
}