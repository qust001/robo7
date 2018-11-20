#include <vector>
#include <cmath>
#include "ros/ros.h"
#include "std_msgs/Bool.h"
#include "geometry_msgs/Point.h"
#include "geometry_msgs/Twist.h"
#include "robo7_msgs/XY_coordinates.h"
#include "robo7_msgs/classifiedObj.h"
#include "robo7_msgs/aObject.h"
#include "robo7_msgs/allObjects.h"
#include "robo7_srvs/objectToRobot.h"


class ObjectFilter
{
  public:
	ros::NodeHandle n;
	ros::Subscriber obj_sub;
  ros::Subscriber robo_pos_sub;
  ros::ServiceClient obj_to_robo_srv;

  ros::Publisher all_obj_pub;


	ObjectFilter()
	{
		// Parameters
		n.param<float>("/object_filter/distance_rad_lim", distance_rad_lim, 0.05);
    n.param<int>("/object_filter/num_classes", num_classes, 14);

		obj_sub = n.subscribe("/vision/results", 1, &ObjectFilter::ObjCallback, this);
    robo_pos_sub = n.subscribe("/localization/kalman_filter/position", 1, &ObjectFilter::roboPosCallback, this);
    obj_to_robo_srv = n.serviceClient<robo7_srvs::objectToRobot>("/localization/object_to_robot");

		all_obj_pub = n.advertise<robo7_msgs::allObjects>("/vision/all_objects", 1);

	}


  void roboPosCallback(const geometry_msgs::Twist::ConstPtr &msg)
  {
    robo_pos = *msg;
    position_updated = true;
  }


	void ObjCallback(const robo7_msgs::classifiedObj::ConstPtr &msg)
	{
    if (!position_updated){
      ROS_WARN("Object filter: Unable to filer object, no robot position recieved");
      return;
    }

    robo7_srvs::objectToRobot::Request srv_req;
    robo7_srvs::objectToRobot::Response srv_resp;

    //geometry_msgs::Point obj_cam_coords = msg->pos;
    srv_req.camera_position = msg->pos;         // geometry_msgs/Point
    srv_req.robot_position = robo_pos;          // geometry_msgs/Twist

    obj_to_robo_srv.call(srv_req, srv_resp);

    if (!srv_resp.success){
      ROS_WARN("Object filter: Unable to filer object, objectToRobot service call failed");
      return;
    }

    ROS_INFO("pos x: %f", srv_resp.object_in_map_frame.x);
    ROS_INFO("pos y: %f", srv_resp.object_in_map_frame.y);

    std::vector<int> init_weigh(num_classes, 0);

    robo7_msgs::aObject new_obj;
    new_obj.obj_class = msg->objClass;
    new_obj.pos = srv_resp.object_in_map_frame;
    new_obj.total_votes = 1;
    new_obj.weights = init_weigh;
    new_obj.weights[msg->objClass] = 1;

    saveObj(new_obj);
	}


  void publishObjects(){
    if(filtered_objs.size() > 0){
      robo7_msgs::allObjects pub_obj;
      pub_obj.objects = filtered_objs;
      all_obj_pub.publish(pub_obj);
    }
  }


  float distance(robo7_msgs::aObject point_a, robo7_msgs::aObject point_b){
    // returns the distance between two aobjects
    return sqrt(pow(point_a.pos.x - point_b.pos.x, 2) + pow(point_a.pos.y - point_b.pos.y, 2));
  }


  int donminantClass(robo7_msgs::aObject a_obj){
    int size = a_obj.weights.size();
    int dom_class = 0;
    int dom_votes = 0;

    if (size > 0){
      for (int m = 0 ; m < size ; m++){
        if (a_obj.weights[m] > dom_votes){
          dom_votes = a_obj.weights[m];
          dom_class = m;
        }
      }
      return dom_class;
    } else {
      ROS_WARN("Object filter: Something went wrong, comparing classes");
      return -1;
    }
  }


  void saveObj(robo7_msgs::aObject new_obj){
    // Do we have any objects in the vector already?
    if (filtered_objs.size() > 0){

      // Check all the current save object to see if the new_obj is the same
      for(std::vector<robo7_msgs::aObject>::iterator a_obj = filtered_objs.begin(); a_obj != filtered_objs.end(); ++a_obj) {

        // if its within the distance limmit it is considered the same object!
        float dist = distance(new_obj, *a_obj);
        ROS_INFO("disten: %f", dist);
        if (dist <= distance_rad_lim){
          ROS_INFO("Object filter: comparing objects");

          // avrage the positon
          (*a_obj).pos.x = (((*a_obj).pos.x * (*a_obj).total_votes) + new_obj.pos.x) / (*a_obj).total_votes;
          (*a_obj).pos.y = (((*a_obj).pos.y * (*a_obj).total_votes) + new_obj.pos.y) / (*a_obj).total_votes;
          (*a_obj).total_votes += 1;
          (*a_obj).weights[new_obj.obj_class] += 1;

          // set class to dominant class
          int dom_class = donminantClass(*a_obj);
          if (dom_class != -1){
            (*a_obj).obj_class = dom_class;
          } else {
            ROS_WARN("Object filter: Error saving object");
          }
          return;
        }
      }

      ROS_INFO("Object filter: new object not matching any in list, adding it");
      filtered_objs.push_back(new_obj);


    } else {
      ROS_INFO("Object filter: adding first object to list");
      filtered_objs.push_back(new_obj);
    }

  }


  // TODO: Return all objects in some way

  private:
    int num_classes;
    float distance_rad_lim;
    geometry_msgs::Twist robo_pos;
    bool position_updated;
    std::vector<robo7_msgs::aObject> filtered_objs;

};

int main(int argc, char **argv)
{
	ros::init(argc, argv, "object_filter");

	ObjectFilter object_filter;

	ros::Rate loop_rate(10);

	ROS_INFO("Object filter running");

	while (ros::ok()){
		ros::spinOnce();
		loop_rate.sleep();
    object_filter.publishObjects();

	}

	//ros::spin();

	return 0;
}
