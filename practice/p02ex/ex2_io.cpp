#include<iostream>
#include<opencv2/opencv.hpp>
using namespace cv;
static const std::string OUT ="/mnt/c/Users/lenovo/Desktop/cv_output/";
int main()
{
    Mat img =imread(OUT + "a.jpg");
    if(img.empty())
    {
        std::cout<<"failed\n";
        return -1;
    }
}