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

void create_histogram_image(cv::Mat &input, cv::Mat &output)
{
	std::vector<cv::Mat> bgr_planes;
	cv::split(input, bgr_planes);

	/// Establish the number of bins
	int histSize = 256;

	/// Set the ranges ( for B,G,R) )
	float range[] = { 0, 256 };
	const float* histRange = { range };

	bool uniform = true; bool accumulate = false;

	cv::Mat b_hist, g_hist, r_hist;

	/// Compute the histograms:
	calcHist(&bgr_planes[0], 1, 0, cv::Mat(), b_hist, 1, &histSize, &histRange, uniform, accumulate);
	calcHist(&bgr_planes[1], 1, 0, cv::Mat(), g_hist, 1, &histSize, &histRange, uniform, accumulate);
	calcHist(&bgr_planes[2], 1, 0, cv::Mat(), r_hist, 1, &histSize, &histRange, uniform, accumulate);

	// Draw the histograms for B, G and R
	int hist_w = 512; int hist_h = 400;
	int bin_w = cvRound((double)hist_w / histSize);

	cv::Mat histImage(hist_h, hist_w, CV_8UC3, cv::Scalar(0, 0, 0));

	normalize(b_hist, b_hist, 0, histImage.rows, cv::NORM_MINMAX, -1, cv::Mat());
	normalize(g_hist, g_hist, 0, histImage.rows, cv::NORM_MINMAX, -1, cv::Mat());
	normalize(r_hist, r_hist, 0, histImage.rows, cv::NORM_MINMAX, -1, cv::Mat());

	/// Draw for each channel
	for (int i = 1; i < histSize; i++)
	{
		line(histImage, cv::Point(bin_w*(i - 1), hist_h - cvRound(b_hist.at<float>(i - 1))),
			cv::Point(bin_w*(i), hist_h - cvRound(b_hist.at<float>(i))),
			cv::Scalar(255, 0, 0), 2, 8, 0);
		line(histImage, cv::Point(bin_w*(i - 1), hist_h - cvRound(g_hist.at<float>(i - 1))),
			cv::Point(bin_w*(i), hist_h - cvRound(g_hist.at<float>(i))),
			cv::Scalar(0, 255, 0), 2, 8, 0);
		line(histImage, cv::Point(bin_w*(i - 1), hist_h - cvRound(r_hist.at<float>(i - 1))),
			cv::Point(bin_w*(i), hist_h - cvRound(r_hist.at<float>(i))),
			cv::Scalar(0, 0, 255), 2, 8, 0);
	}

	output = histImage;
}

int main(int argc, char *argv[])
{
	std::cout << "Hello, World!!" << std::endl;

	tungsten::ConfigFile cfg("..\\config\\config.cfg");

	bool separate_channels = cfg.getValueOfKey("separate_channels");
	bool equalize_colors = cfg.getValueOfKey("equalize_colors");
	bool show_histograms = cfg.getValueOfKey("show_histograms");
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

		if (show_histograms)
		{
			cv::Mat histograms;
			create_histogram_image(frame, histograms);
			cv::imshow("histograms", histograms);
		}

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

		if (equalize_colors)
		{
			cv::cvtColor(img, img, cv::COLOR_BGR2GRAY);
			cv::equalizeHist(img, img);
			cv::cvtColor(img, img, cv::COLOR_GRAY2BGR);
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
