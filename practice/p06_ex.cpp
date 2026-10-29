#include <iostream>
#include <opencv2/opencv.hpp>
using namespace cv;
static const std::string OUT = "/mnt/c/Users/lenovo/Desktop/cv_output/";

int main()
{
    std::string path="/mnt/c/Users/lenovo/Desktop/装甲板视频.avi";
    VideoCapture cap(path);
    if(!cap.isOpened())
    {
        std::cout<<"failed";
        return -1;
    }
    double fps   = cap.get(CAP_PROP_FPS);            // 帧率
    int    w     = (int)cap.get(CAP_PROP_FRAME_WIDTH);   // 宽
    int    h     = (int)cap.get(CAP_PROP_FRAME_HEIGHT);  // 高
    Mat frame;
    /*for(int i=1;i<=50;i++)
    {
        if(!cap.read(frame))     return -1;
    }*/
    VideoWriter writer(OUT+ "p06_ex.avi",
                       VideoWriter::fourcc('M', 'J', 'P', 'G'),   // 编码
                       fps,                                       // 帧率
                       Size(w, h));
    while(cap.read(frame)){
        Mat hsv;
        cvtColor(frame,hsv,COLOR_BGR2HSV);
        Mat m1,m2,red;
        inRange(hsv,Scalar(0,150,60),Scalar(30,255,255),m1);
        inRange(hsv,Scalar(170,150,60),Scalar(180,255,255),m2);
        bitwise_or(m1,m2,red);
        Mat gray,bright;
        cvtColor(frame,gray,COLOR_BGR2GRAY);
        threshold(gray,bright,100,255,THRESH_BINARY);
        Mat mask;
        bitwise_and(red,bright,mask);
        std::vector<std::vector<Point>> contours;
        std::vector<Vec4i> h;
        findContours(mask.clone(),contours,h,RETR_EXTERNAL,CHAIN_APPROX_SIMPLE);
        Mat result = frame.clone();  
        for(size_t i=0;i<contours.size();i++)
       {
            const auto& c=contours[i];
            double area = contourArea(c);           
            RotatedRect rr = minAreaRect(c);  
            float w = rr.size.width, ht = rr.size.height;
            float ratio = std::max(w, ht) / std::min(w, ht);
            if (area <= 5 || ratio <= 3.0) continue; 
            Point2f pts[4];
            rr.points(pts);
            std::sort(pts,pts+4,[](const Point2f& a,const Point2f& b){return a.y<b.y;});
            if(pts[0].x>pts[1].x)   std::swap(pts[0],pts[1]);
            if(pts[2].x>pts[3].x)   std::swap(pts[2],pts[3]);
            std::vector<Point> poly={Point((int)pts[0].x,(int)pts[0].y),Point((int)pts[1].x,(int)pts[1].y),Point((int)pts[3].x,(int)pts[3].y),Point((int)pts[2].x,(int)pts[2].y)};
            polylines(result,poly,true,Scalar(0,255,0),1);                
        }
        writer.write(result);
    }
    
    return 0;
}