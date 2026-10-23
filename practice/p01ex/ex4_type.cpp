#include<iostream>
#include<opencv2/opencv.hpp>
using namespace cv;
static const std::string OUT = "/mnt/c/Users/lenovo/Desktop/cv_output/";
int main()
{
    Mat img =Mat::zeros(300,300,CV_8UC3);
    img.at<Vec3b>(80,50)=(Vec3b)(0,255,0);
    Mat roi=img(Rect(100,50,80,120));
    roi.setTo(Scalar(0,255,255));
    imwrite(OUT + "ex4_yellow.jpg",img);
    return 0;
}
