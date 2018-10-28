#include <algorithm>
#include <vector>
#include "ros/ros.h"
#include "std_msgs/Bool.h"
#include "robo7_srvs/IsGridOccupied.h"
#include "robo7_msgs/XY_coordinates.h"
#include <cv_bridge/cv_bridge.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui_c.h>

typedef std::vector<double> Array;
typedef std::vector<Array> Matrix;

class OccupancyGridServer
{
public:
	ros::NodeHandle n;
	ros::Subscriber map_sub;
	ros::ServiceServer is_occupied_service;
	Matrix grid;


	OccupancyGridServer()
	{
		// Parameters
		n.param<double>("/occupancy_grid_server/grid_square_size", grid_square_size, 0.02);
		n.param<double>("/occupancy_grid_server/min_distance", min_distance, 0.13);
		n.param<int>("/occupancy_grid_server/smoothing_kernel_size", smoothing_kernel_size, 15);
		n.param<int>("/occupancy_grid_server/smoothing_kernel_sd", smoothing_kernel_sd, 3);

		map_sub = n.subscribe("/own_map/wall_coordinates", 1, &OccupancyGridServer::mapCallback, this);
		is_occupied_service = n.advertiseService("/occupancy_grid/is_occupied", &OccupancyGridServer::gridRequest, this);

		num_min_distance_squares = ceil(min_distance/grid_square_size);

		if ( smoothing_kernel_size % 2 == 0){
			smoothing_kernel_size += 1;
			ROS_WARN("Entered kernel size is even number, changing to: %d", smoothing_kernel_size);
	  }

	}

	void mapCallback(const robo7_msgs::XY_coordinates::ConstPtr &msg)
	{
		X_wall_coordinates = msg->X_coordinates;
		Y_wall_coordinates = msg->Y_coordinates;
	}

	bool gridRequest(robo7_srvs::IsGridOccupied::Request &req,
				 robo7_srvs::IsGridOccupied::Response &res)
	{
		ROS_DEBUG("New grid occupancy request recieved");

		//updateGrid();

		int req_x_grid = floor(req.x/grid_square_size);
		int req_y_grid = floor(req.y/grid_square_size);

		if (req_x_grid >= num_grid_squares_x || req_y_grid >= num_grid_squares_y){
			res.occupancy = 1.0;
		}
		else{
			//res.occupancy = gauss_grid.at<float>(req_y_grid, req_x_grid);
			//res.occupancy = grid[req_x_grid][req_y_grid];
			float value = gauss_grid.at<float>(req_x_grid, req_y_grid);

			if (value < 0.0001)
			{
				res.occupancy = 0.0;
			}
			else{
				res.occupancy = value;
			}
		}
		return true;
	}

	void updateGridSize()
	{
		ROS_DEBUG("Setting grid size");

		// Wait untill we get our first coordinates
		while (X_wall_coordinates.size() <= 0){
			ros::spinOnce();
		}

		// Get maximum coordinates in grid
		float x_max = *max_element(X_wall_coordinates.begin(), X_wall_coordinates.end());
		float y_max = *max_element(Y_wall_coordinates.begin(), Y_wall_coordinates.end());
		ROS_DEBUG("x_max: %f", x_max);
		ROS_DEBUG("y_max: %f", y_max);

		num_grid_squares_x = ceil(x_max/grid_square_size);
		num_grid_squares_y = ceil(y_max/grid_square_size);
		ROS_DEBUG("x squares: %d", num_grid_squares_x);
		ROS_DEBUG("y squares: %d", num_grid_squares_y);

		// Set up sizes
		grid.resize(num_grid_squares_x);
	  for (int i = 0; i < num_grid_squares_x; ++i)
		    grid[i].resize(num_grid_squares_y);

		updateGrid();
	}

	void updateGrid()
	{
		ROS_DEBUG("Updating grid");

		// Put every point in the grid
		for (int i = 0; i< X_wall_coordinates.size(); ++i){
			// Get the square that the point is located in
			int square_x = floor(X_wall_coordinates[i]/grid_square_size);
			int square_y = floor(Y_wall_coordinates[i]/grid_square_size);

			int x_low = square_x - num_min_distance_squares;
			int y_low = square_y - num_min_distance_squares;

			int x_high = square_x + num_min_distance_squares;
			int y_high = square_y + num_min_distance_squares;

			// Setting 1.0 for all the walls (enlarged by the min distance)
			for (int i = x_low; i <= x_high ; ++i){
	 		  for (int j = y_low; j <= y_high; ++j) {
					if (!(i < 0) && !(i >= num_grid_squares_x) && !(j < 0) && !(j >= num_grid_squares_y))
					{
						grid[i][j] = 1.0;
					}
				}
			}
		}

		// Setting the filtered grid
		gauss_grid = gauss_filter(grid, smoothing_kernel_size, smoothing_kernel_sd);

	}

	cv::Mat gauss_filter(Matrix unfiltered_grid, int kernel_size, int sigma)
	{
		cv::Mat grid_in(unfiltered_grid.size(), unfiltered_grid.at(0).size(), CV_64FC1);
		cv::Mat grid_filtered;

		for(int i=0; i<grid_in.rows; ++i){
			for(int j=0; j<grid_in.cols; ++j){
				grid_in.at<double>(i, j) = unfiltered_grid[i][j];
			}
		}

		// To display uncomment here and bellow
		/*
		namedWindow("Display window", cv::WINDOW_NORMAL );
		cv::resizeWindow("Display window", 600,600);
		imshow( "Display window", grid_in );
		cv::waitKey(0);
		*/

		GaussianBlur(grid_in, grid_filtered, cv::Size(kernel_size, kernel_size), sigma, 0 );

		// To display uncomment here as well
		/*
		imshow( "Display window", grid_filtered );
		cv::waitKey(0);
		cvDestroyWindow("Display window");
		*/

		cv::Mat normalized_grid;
		cv::normalize(grid_filtered, normalized_grid, 0, 1, cv::NORM_MINMAX, CV_32F);

		ROS_INFO("Gaussed grid ready");
		return normalized_grid;
	}


private:
	double min_distance;
	int num_min_distance_squares;
	double grid_square_size;
	int smoothing_kernel_size;
	int smoothing_kernel_sd;
	int num_grid_squares_x;
	int num_grid_squares_y;
	std::vector<float> X_wall_coordinates;
	std::vector<float> Y_wall_coordinates;
	cv::Mat gauss_grid;


};

int main(int argc, char **argv)
{
	ros::init(argc, argv, "occupancy_grid_server");

	OccupancyGridServer occupancy_grid_server;

	ros::Rate loop_rate(10);

	ROS_INFO("Occupancy grid server running");

	occupancy_grid_server.updateGridSize();
<<<<<<< HEAD
	occupancy_grid_server.printGrid();
=======
>>>>>>> d5bc5ef7de86da509474fc3d3fa038c2818618ba

	ros::spin();

	return 0;
}
