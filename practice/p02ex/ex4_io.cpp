#include<iostream>
#include<opencv2/opencv.hpp>
using namespace cv;
static const std::string OUT = "/mnt/c/Users/lenovo/Desktop/cv_output/";
int main()
{
    Mat img(200,300,CV_8UC3,Scalar(0,0,255));
    img(Rect(150,0,150,200)) .setTo(Scalar(255,0,0));
    std::vector<Mat> channels;
    split(img,channels);
    imwrite(OUT + "t4_b.jpg",channels[0]);
    imwrite(OUT + "t4_g.jpg",channels[1]);
    imwrite(OUT + "t4_r.jpg",channels[2]);
    Mat merged;
    merge(channels,merged);
    std::cout<<img.at<Vec3b>(50,50)<<"\n";
    std::cout<<merged.at<Vec3b>(50,50)<<"\n";
    std::cout<<img.at<Vec3b>(50,250)<<"\n";
    std::cout<<merged.at<Vec3b>(50,250)<<"\n";
    return 0;
}