#include<iostream>
#include<opencv2/opencv.hpp>
int main()
{
    cv::Mat img = cv::Mat::zeros(250,400,CV_8UC3);
    std::cout<<img.rows<<" "<<img.cols<<" "<<img.channels()<<" "<<img.size()<<" "<<img.empty();
    std::cout<<CV_8UC3;
    return 0;
}