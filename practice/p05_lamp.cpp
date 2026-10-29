#include<iostream>
#include<opencv2/opencv.hpp>
#include<vector>
#include<algorithm>
using namespace cv;
static const std::string OUT = "/mnt/c/Users/lenovo/Desktop/cv_output/";
int main()
{
    std::string path = OUT + "ex_armor_test.png";
    Mat img = imread(path);
    if (img.empty()) {
        std::cout << "读图失败: " << path << "\n";
        return -1;
    }
    Mat gray;
    cvtColor(img,gray,COLOR_BGR2GRAY);
    Mat binary;
    threshold(gray,binary,150,255,THRESH_BINARY);
    imwrite(OUT+"step1_binary.jpg",binary);
    Mat kernel=getStructuringElement(MORPH_RECT,Size(5,5));
    morphologyEx(binary,binary,MORPH_OPEN,kernel);
    std::vector<std::vector<Point>> contours;
    std::vector<Vec4i>hierarchy;
    findContours(binary.clone(),contours,hierarchy,RETR_EXTERNAL,CHAIN_APPROX_SIMPLE);
    for(size_t i=0;i<contours.size();i++)
    {
        const auto& c=contours[i];
        double area = contourArea(c);
        RotatedRect rr = minAreaRect(c);
        float w = rr.size.width;
        float h = rr.size.height;
        float ratio = std::max(w,h)/std::min(w,h);
        if(area<=100 || ratio <=2.5)    continue;
        Point2f pts[4];
        rr.points(pts);
        std::sort(pts,pts+4,[](const Point2f& a,const Point2f& b){return a.y<b.y;});
        if(pts[0].x>pts[1].x)   std::swap(pts[0],pts[1]);
        if(pts[2].x>pts[3].x)   std::swap(pts[2],pts[3]);
        std::vector<Point> poly={Point((int)pts[0].x,(int)pts[0].y),Point((int)pts[1].x,(int)pts[1].y),Point((int)pts[3].x,(int)pts[3].y),Point((int)pts[2].x,(int)pts[2].y)};
        polylines(img,poly,true,Scalar(0,255,0),2);
    }
    imwrite(OUT + "lamp.jpg",img);
    return 0;
}