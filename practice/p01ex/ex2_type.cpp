#include<iostream>
#include<opencv2/opencv.hpp>
int main()
{
    cv::Mat img1=cv::Mat::zeros(100,100,CV_8UC1);
    cv::Mat img2=cv::Mat::ones(100,100,CV_8UC1);// 如果要填255，则改为Mat o(100,100,CV_8UC1,Scalar(255))
    std::cout<<(int)img1.at<uchar>(0,0);
    std::cout<<(int)img2.at<uchar>(0,0);
    return 0;
}