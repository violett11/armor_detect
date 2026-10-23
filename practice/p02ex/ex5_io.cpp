#include<iostream>
#include<opencv2/opencv.hpp>
using namespace cv;
static const std::string OUT = "/mnt/c/Users/lenovo/Desktop/cv_output/";
int main()
{
    Mat img(480,300,CV_8UC3,Scalar(35,35,35));
    img(Rect(140,80,18,140)).setTo(Scalar(250,250,250));
    img(Rect(200,240,60,40)).setTo(Scalar(120,120,120));
    Mat hsv,mask;
    cvtColor(img,hsv,COLOR_BGR2HSV);
    inRange(hsv,Scalar(0,0,200),Scalar(180,60,255),mask);
    std::cout<<countNonZero(mask)<<"\n";
    imwrite(OUT + "T5_mask.jpg",mask);
    return 0;

}