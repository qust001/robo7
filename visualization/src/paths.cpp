
#include <ros/ros.h>
#include <visualization_msgs/Marker.h>
#include <visualization_msgs/MarkerArray.h>
#include "std_msgs/Float32MultiArray.h"
#include <robo7_msgs/paths.h>
#include <cmath>
#include <vector>
#include <geometry_msgs/Twist.h>

std::vector<std::vector<float> > paths_x, paths_y;
std::vector<float> path_x_msg, path_y_msg;
std::vector<float> start_goal_x(2), start_goal_y(2);
double control_frequency = 10.0;
int number_paths = 0;

class Paths
{
public:
  ros::Subscriber robot_position, paths_sub, start_goal_sub;
  ros::Publisher marker_array_pub;

  //Initialisation
  float x0, y0, theta0;
  bool paths_received, start_goal_received;

  Paths(ros::NodeHandle nh, ros::Publisher marker_array_pub)
  {
    this->paths_sub = nh.subscribe("/pathplanning/paths_vector", 1000, &Paths::pathsCallback, this);
    this->start_goal_sub = nh.subscribe("/pathplanning/start_goal", 1000, &Paths::startGoalCallback, this);
    this->marker_array_pub = marker_array_pub;
    this->paths_received = false;
    this->start_goal_received = false;
  }

  void pathsCallback(const robo7_msgs::paths::ConstPtr &paths_msg)
  {
    paths_x.clear();
    paths_y.clear();

    for (int i = 0; i < paths_msg->paths.size(); i++)
    {
      paths_x.push_back(paths_msg->paths[i].path_x);
      paths_y.push_back(paths_msg->paths[i].path_y);
    }
    paths_received = true;
  }

  void startGoalCallback(const robo7_msgs::path::ConstPtr &start_goal_msg)
  {
    //ROS_INFO("start: x: %f  y: %f", start_goal_msg->path_x[0], start_goal_msg->path_y[0]);
    //ROS_INFO("end: x: %f  y: %f", start_goal_msg->path_x[1], start_goal_msg->path_y[1]);
    start_goal_x[0] = start_goal_msg->path_x[0];
    start_goal_x[1] = start_goal_msg->path_x[1];
    start_goal_y[0] = start_goal_msg->path_y[0];
    start_goal_y[1] = start_goal_msg->path_y[1];

    start_goal_received = true;
  }

  void updatePaths()
  {

    if (this->paths_received)
    {
      visualization_msgs::MarkerArray curve_array_msg, points_array_msg, start_goal_msg;

      geometry_msgs::Point p;

      curve_array_msg.markers.resize(paths_x.size());
      points_array_msg.markers.resize(paths_x.size());
      start_goal_msg.markers.resize(2);

      for (int i = 0; i < paths_x.size(); i++)
      {

        curve_array_msg.markers[i].header.frame_id = points_array_msg.markers[i].header.frame_id = "/map";
        curve_array_msg.markers[i].header.stamp = points_array_msg.markers[i].header.stamp = ros::Time::now();
        curve_array_msg.markers[i].action = points_array_msg.markers[i].action = visualization_msgs::Marker::ADD;
        curve_array_msg.markers[i].pose.orientation.w = points_array_msg.markers[i].pose.orientation.w = 1.0;
        curve_array_msg.markers[i].ns = "Curves";
        points_array_msg.markers[i].ns = "Nodes";

        curve_array_msg.markers[i].id = i;
        points_array_msg.markers[i].id = i + paths_x.size() + 2;

        curve_array_msg.markers[i].type = visualization_msgs::Marker::LINE_STRIP;
        points_array_msg.markers[i].type = visualization_msgs::Marker::SPHERE;

        /*
        points_array_msg.markers[i].pose.position.x = 1;
        points_array_msg.markers[i].pose.position.y = 1;
        points_array_msg.markers[i].pose.position.z = 0;
        points_array_msg.markers[i].pose.orientation.x = 0.0;
        points_array_msg.markers[i].pose.orientation.y = 0.0;
        points_array_msg.markers[i].pose.orientation.z = 0.0;
        */
        curve_array_msg.markers[i].scale.x = 0.01;
        curve_array_msg.markers[i].scale.y = 0.01;
        curve_array_msg.markers[i].scale.z = 0.01;

        curve_array_msg.markers[i].color.b = 1.0;
        curve_array_msg.markers[i].color.a = 1.0;

        points_array_msg.markers[i].scale.x = 0.03;
        points_array_msg.markers[i].scale.y = 0.03;
        points_array_msg.markers[i].scale.z = 0.03;

        points_array_msg.markers[i].color.r = 0.0;
        points_array_msg.markers[i].color.g = 0.0;
        points_array_msg.markers[i].color.b = 1.0;
        points_array_msg.markers[i].color.a = 1.0;

        //

        //points_array_msg.markers[i].points.push_back(p);
        /*
        */

        for (int j = 0; j < paths_x[i].size(); j++)
        {
          p.x = paths_x[i][j];
          p.y = paths_y[i][j];
          p.z = 0.0;
          curve_array_msg.markers[i].points.push_back(p);
        }
        points_array_msg.markers[i].pose.position.x = p.x;
        points_array_msg.markers[i].pose.position.y = p.y;
      }

      if (this->start_goal_received)
      {

        for (int i = 0; i < 2; i++)
        {
          start_goal_msg.markers[i].header.frame_id = "/map";
          start_goal_msg.markers[i].header.stamp = ros::Time::now();
          start_goal_msg.markers[i].action = visualization_msgs::Marker::ADD;
          start_goal_msg.markers[i].pose.orientation.w = 1.0;
          start_goal_msg.markers[i].id = i;

          if (i == 0)
          {
            start_goal_msg.markers[i].ns = "Start";
            start_goal_msg.markers[i].type = visualization_msgs::Marker::CYLINDER;
            start_goal_msg.markers[i].color.r = 1.0;
            start_goal_msg.markers[i].color.a = 1.0;
          }
          else
          {
            start_goal_msg.markers[i].ns = "Goal";
            start_goal_msg.markers[i].type = visualization_msgs::Marker::CUBE;
            start_goal_msg.markers[i].color.g = 1.0;
            start_goal_msg.markers[i].color.a = 1.0;
          }

          start_goal_msg.markers[i].scale.x = 0.1;
          start_goal_msg.markers[i].scale.y = 0.1;
          start_goal_msg.markers[i].scale.z = 0.1;

          start_goal_msg.markers[i].pose.position.x = start_goal_x[i];
          start_goal_msg.markers[i].pose.position.y = start_goal_y[i];
          //ROS_INFO("i: %d x: %f  y: %f", i, start_goal_msg.markers[i].pose.position.x, start_goal_msg.markers[i].pose.position.y);
        }
      }

      //marker_array_pub.publish(points_array_msg);
      marker_array_pub.publish(curve_array_msg);
      marker_array_pub.publish(start_goal_msg);
    }
  }
};

int main(int argc, char **argv)
{
  ros::init(argc, argv, "paths");
  ros::NodeHandle nh;

  ros::Publisher marker_array_pub = nh.advertise<visualization_msgs::MarkerArray>("Paths", 10);
  ROS_INFO("Init paths");
  Paths paths = Paths(nh, marker_array_pub);

  ros::Rate loop_rate(control_frequency);

  while (ros::ok())
  {

    paths.updatePaths();

    ros::spinOnce();
    loop_rate.sleep();
  }
}
