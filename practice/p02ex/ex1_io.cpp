#include<iostream>
#include<opencv2/opencv.hpp>
using namespace cv;
static const std::string OUT= "/mnt/c/Users/lenovo/Desktop/cv_output/";
int main()
{
    Mat img (200,300,CV_8UC1,Scalar(0));
    img (Rect(100,0,200,200)).setTo(Scalar(255));
    imwrite(OUT + "t1.png", img);
    imwrite(OUT + "t1.jpg", img);
    Mat png = imread (OUT + "t1.png",IMREAD_GRAYSCALE);
    Mat jpg = imread (OUT + "t1.jpg",IMREAD_GRAYSCALE);
    if(png.empty()||jpg.empty())
    {
        std::cout<<"读图失败\n";
        return -1;
    }
    std::cout<<(int)img.at<uchar>(50,99)<<" "<<(int)img.at<uchar>(50,100)<<" "<<(int)img.at<uchar>(50,101)<<"\n";
    std::cout<<(int)jpg.at<uchar>(50,99)<<" "<<(int)jpg.at<uchar>(50,100)<<" "<<(int)jpg.at<uchar>(50,101)<<"\n";
    std::cout<<(int)png.at<uchar>(50,99)<<" "<<(int)png.at<uchar>(50,100)<<" "<<(int)png.at<uchar>(50,101)<<"\n";
    return 0;
}