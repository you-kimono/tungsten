#include <iostream>
#include <opencv2/opencv.hpp>

int main(int argc, char *argv[])
{
	std::cout << "Hello, World!!" << std::endl;

	cv::VideoCapture cap(0);
	if (!cap.isOpened())
		return -1;

	for (;;)
	{
		cv::Mat frame;
		cap >> frame;

		cv::imshow("captured", frame);
		if (cv::waitKey(30) > 0) break;
	}

	return 0;
}
