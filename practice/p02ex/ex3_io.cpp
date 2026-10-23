#include<iostream>
#include<opencv2/opencv.hpp>
using namespace cv;
static const std::string OUT ="/mnt/c/Users/lenovo/Desktop/cv_output/";
int main()
{
    Mat img (200,200,CV_8UC3,Scalar(0,0,255));
    Mat gray,hsv;
    cvtColor(img,gray,COLOR_BGR2GRAY);
    cvtColor(img,hsv,COLOR_BGR2HSV);
    std::cout<<(int)gray.at<uchar>(100,100)<<"\n";
    Vec3b p=hsv.at<Vec3b>(100,100);
    for(int i=0;i<3;i++)
    {
        std::cout<<(int)p[i]<<"\n";
    }
    return 0;
}