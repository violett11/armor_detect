#include<iostream>
#include<opencv2/opencv.hpp>
using namespace cv;
static const std::string OUT = "/mnt/c/Users/lenovo/Desktop/cv_output/";
int main()
{
    Mat img(100,300,CV_8UC3,Scalar(0,0,0));
    img(Rect(0,0,100,100)).setTo(Scalar(0,0,255));
    img(Rect(100,0,100,100)).setTo(Scalar(0,255,0));
    img(Rect(200,0,100,100)).setTo(Scalar(255,0,0));
    imwrite(OUT + "ex5_color.jpg",img);
    Mat grey;
    cvtColor(img,grey,COLOR_BGR2GRAY);
    imwrite(OUT+"ex5_grey.jpg",grey);
    std::cout<<(int)grey.at<uchar>(50,50)<<"\n";
    std::cout<<(int)grey.at<uchar>(50,150)<<"\n";
    std::cout<<(int)grey.at<uchar>(50,250)<<"\n";
    return 0;
}