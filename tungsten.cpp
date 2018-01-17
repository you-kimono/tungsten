#include <iostream>
#include <chrono>
#include <opencv2/opencv.hpp>
#include <ctime>

#include "ConfigFile.h"

inline bool ends_with(std::string const & value, std::string const & ending)
{
	if (ending.size() > value.size()) return false;
	return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

bool store_image_to_folder(cv::Mat &img, std::string path)
{
	std::stringstream filename;
	filename << path;
	if (!ends_with(path, "\\")) filename << "\\";

	std::chrono::milliseconds millis = 
		std::chrono::duration_cast<std::chrono::milliseconds>
		(std::chrono::system_clock::now().time_since_epoch()
	);
	filename << millis.count() << ".yml";

	cv::FileStorage fs(filename.str(), cv::FileStorage::WRITE);
	fs << "data" << img;
	fs.release();

	std::cout << "Saved: " << filename.str() << std::endl;
	
	return true;
}

cv::Mat preprocess_frame(cv::Mat &image, tungsten::ConfigFile cfg)
{
	bool separate_channels = cfg.getValueOfKey("separate_channels");
	int scale_factor = cfg.getValueOfKey<int>("scale_factor", 1);

	if (separate_channels)
	{
		cv::Mat bgr[3];
		cv::split(image, bgr);
		image = bgr[0];
		cv::cvtColor(image, image, cv::COLOR_GRAY2BGR);
	}
	if (scale_factor > 1)
	{
		int resized_width = image.cols / scale_factor;
		int resized_height = image.rows / scale_factor;
		cv::resize(image, image, cv::Size(resized_width, resized_height));
	}
	return image;
}

int main(int argc, char *argv[])
{
	std::cout << "Hello, World!!" << std::endl;

	tungsten::ConfigFile cfg("..\\config\\config.cfg");

	bool separate_channels = cfg.getValueOfKey("separate_channels");
	std::string image_store_path = cfg.getValueOfKey<std::string>("image_store_path", "");

	cv::VideoCapture cap(0);
	if (!cap.isOpened())
		return -1;
	
	bool capture_images = false;
	int frame_counter = 0;
	int tick = 0;
	int fps = 0;
	std::time_t time_begin = std::time(0);

	for (;;)
	{
		cv::Mat frame;
		cap >> frame;

		cv::Mat img;
		img = preprocess_frame(frame, cfg);
		
		frame_counter++;
		std::time_t timeNow = std::time(0) - time_begin;

		if (timeNow - tick >= 1)
		{
			tick++;
			fps = frame_counter;
			frame_counter = 0;
		}

		if (capture_images) store_image_to_folder(img, image_store_path);

		cv::putText(img, cv::format("Average FPS=%d", fps), cv::Point(30, 30), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 0, 255));
		cv::imshow("grayscale", img);

		char key = cv::waitKey(30);
		if (113 == key) break;
		if (99 == key) capture_images = !capture_images;
	}

	return 0;
}
