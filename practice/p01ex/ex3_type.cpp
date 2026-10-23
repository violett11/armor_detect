#include<iostream>
#include<opencv2/opencv.hpp>
using namespace cv;

static const std::string OUT = "/mnt/c/Users/lenovo/Desktop/cv_output/";
int main()
{
    Mat img=Mat::zeros(200,200,CV_8UC3);
    img.at<Vec3b>(80,50)=Vec3b(0,255,0);
    Vec3b p=img.at<Vec3b>(80,50);
    std::cout<<(int)p[0]<<" "<<(int)p[1]<<" "<<(int)p[2]<<"\n";
    imwrite(OUT + "ex3_green.jpg",img);
    std::cout<<"saved\n";
    return 0;
} 