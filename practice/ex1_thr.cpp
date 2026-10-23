#include <iostream>
#include <opencv2/opencv.hpp>
using namespace cv;
static const std::string OUT = "/mnt/c/Users/lenovo/Desktop/cv_output/";

int main()
{
    Mat img (300,480,CV_8UC3,Scalar(35,35,35));
    img(Rect(140,80,18,140)).setTo(Scalar(250,250,250));
    RNG rng(12345);
    for(int i=1;i<=60;i++)
    {
        int x=rng.uniform(0,479);
        int y=rng.uniform(0,299);
        circle(img,Point(x,y),rng.uniform(1,3),Scalar(200,200,200),-1);
    }
    Mat gray;
    cvtColor(img,gray,COLOR_BGR2GRAY);
    Mat src1;
    threshold(gray,src1,150,255,THRESH_BINARY);
    std::cout<<countNonZero(src1)<<"\n";
    Mat src2;
    threshold(gray,src2,150,255,THRESH_BINARY_INV);
    std::cout<<countNonZero(src2)<<"\n";  
    imwrite(OUT + "src1.jpg",src1);  
    imwrite(OUT + "src2.jpg",src2);  
    return 0;
}