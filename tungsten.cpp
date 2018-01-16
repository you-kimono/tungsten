#include <iostream>
#include <opencv2/opencv.hpp>

#include "ConfigFile.h"

int main(int argc, char *argv[])
{
	std::cout << "Hello, World!!" << std::endl;

	tungsten::ConfigFile cfg("..\\config\\config.cfg");

	bool exists = cfg.keyExists("prova");
	std::string prova = cfg.getValueOfKey<std::string>("prova", "ERROR!");

	cv::VideoCapture cap(0);
	if (!cap.isOpened())
		return -1;

	for (;;)
	{
		cv::Mat frame;
		cap >> frame;

		cv::Mat bgr[3];
		cv::split(frame, bgr);
		cv::imshow("grayscale", bgr[0]);
		if (cv::waitKey(30) > 0) break;
	}

	return 0;
}
