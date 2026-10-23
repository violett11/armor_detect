#include <iostream>
#include <opencv2/opencv.hpp>

int main()
{
    std::cout << "OpenCV version: " << CV_VERSION << std::endl;

    cv::Mat img = cv::Mat::zeros(200, 300, CV_8UC3);
    cv::circle(img, {150, 100}, 50, {0, 0, 255}, -1);
    cv::rectangle(img, {20, 20}, {80, 80}, {0, 255, 0}, 3);

    std::cout << "image size: " << img.cols << " x " << img.rows << std::endl;

    cv::imwrite("/mnt/c/Users/lenovo/Desktop/opencv_test.jpg", img);
    std::cout << "saved to Desktop" << std::endl;

    return 0;
}