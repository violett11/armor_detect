#include <iostream>
#include <opencv2/opencv.hpp>
using namespace cv;
static const std::string OUT = "/mnt/c/Users/lenovo/Desktop/cv_output/";

static const Mat K = (Mat_<double>(3, 3) <<
    928.130989,   0.0,        377.572945,
      0.0,      930.138391,   283.892859,
      0.0,        0.0,          1.0);

static const Mat DIST = (Mat_<double>(1, 5) <<
    -0.254433647, 0.569431382, 0.00365405229, -0.00109433818, -1.33846840);

static const double pw = 130.0;
static const double ph = 50.0;
void drawaxis(Mat& img,const Mat& rvec,const Mat& tvec,const Mat& K,const Mat& dist,float axislen=130.0f)
{
    std::vector<Point3f> axis={
                Point3f(0,0,0),
                Point3f(axislen,0,0),
                Point3f(0,axislen,0),
                Point3f(0,0,axislen)
    };
    std::vector<Point2f> proj;
    projectPoints(axis,rvec,tvec,K,dist,proj);
    Point o=proj[0];
    Point x=proj[1];
    Point y=proj[2];
    Point z=proj[3];
    line(img,o,x,Scalar(0,0,255),3,LINE_AA);
    line(img,o,y,Scalar(0,255,0),3,LINE_AA);
    line(img,o,z,Scalar(255,0,0),3,LINE_AA);
    circle(img,o,4,Scalar(0,255,255),-1,LINE_AA);
};
int main()
{
    Mat rvecPrev, tvecPrev;
    bool flag = false;
    std::vector<Point3f> objectpoint={
                Point3f(0.0f,(float)(-pw/2),(float)(ph/2)),//左上
                Point3f(0.0f,(float)(pw/2),(float)(ph/2)),//右上
                Point3f(0.0f,(float)(pw/2),(float)(-ph/2)),//右下
                Point3f(0.0f,(float)(-pw/2),(float)(-ph/2))//左下
            };
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
    namedWindow("compare",WINDOW_NORMAL);
    resizeWindow("compare",1200,470);
    moveWindow("compare",80,80);
    bool pause=false;
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
        std::vector<Vec4i> hier;
        findContours(mask.clone(),contours,hier,RETR_EXTERNAL,CHAIN_APPROX_SIMPLE);
        Mat result = frame.clone(); 
        std::vector<RotatedRect> lamp; 
        for(size_t i=0;i<contours.size();i++)
       {
            const auto& c=contours[i];
            double area = contourArea(c);           
            RotatedRect rr = minAreaRect(c);  
            float ww = rr.size.width, ht = rr.size.height;
            float ratio = std::max(ww, ht) / std::min(ww, ht);
            if (area <= 5 || ratio <= 3.0) continue; 
           
            lamp.push_back(rr);               
        }
        if(lamp.size()==2)
        {
            if(lamp[0].center.x>lamp[1].center.x)   std::swap(lamp[0],lamp[1]);
            auto getends = [](const RotatedRect& r,Point2f& top,Point2f& bottom)
            {
                Point2f p[4];
                r.points(p);
                std::sort(p, p + 4,
                          [](const Point2f& a, const Point2f& b) { return a.y < b.y; });
                top    = (p[0]+p[1])*0.5f;
                bottom = (p[2]+p[3])*0.5f;           
            };
            Point2f lt,lb,rt,rb;
            getends(lamp[0],lt,lb);
            getends(lamp[1],rt,rb);
            std::vector<Point> plate = {
                Point((int)lt.x,(int)lt.y),
                Point((int)lb.x,(int)lb.y),
                Point((int)rb.x,(int)rb.y),
                Point((int)rt.x,(int)rt.y),
            };
            polylines(result,plate,true,Scalar(0,255,0),2);  
            std::vector<Point2f> imagepoint={
                lt,rt,rb,lb
            };//定义像点
            Mat rvec,tvec;
            if(!flag)
            {
                solvePnP(objectpoint,imagepoint,K,DIST,rvec,tvec,false,SOLVEPNP_IPPE);
            }
            else{
                rvec=rvecPrev.clone();
                tvec=tvecPrev.clone();
                solvePnP(objectpoint,imagepoint,K,DIST,rvec,tvec,true,SOLVEPNP_ITERATIVE);
            } 

            rvecPrev = rvec.clone();
            tvecPrev = tvec.clone();
            flag = true;
            drawaxis(result, rvec, tvec, K, DIST, 130.0f);
            std::string s= "dist " +std::to_string((int)norm(tvec))+" mm";
            putText(result,s,Point(20,result.rows-70),FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0, 255, 255), 2);       
        }
        writer.write(result);
        //呈现成果图
        Mat sideBySide(frame.rows, frame.cols*2, CV_8UC3);
        frame.copyTo(sideBySide(Rect(0, 0, frame.cols, frame.rows)));
        result.copyTo(sideBySide(Rect(frame.cols, 0, frame.cols, frame.rows)));
        line(sideBySide, Point(frame.cols, 0), Point(frame.cols, frame.rows),Scalar(90, 90, 90), 2);
        putText(sideBySide, "RAW",    Point(20, 40), FONT_HERSHEY_SIMPLEX,1.0, Scalar(0, 235, 255), 2);               
        putText(sideBySide, "RESULT", Point(frame.cols + 20, 40),FONT_HERSHEY_SIMPLEX, 1.0, Scalar(0, 235, 255), 2);
        imshow("compare", sideBySide);
        int key=waitKey(80);                   // 固定 40ms
        if (key=='q') break;
    }
    cap.release();
    writer.release();
    destroyAllWindows();
    return 0;
}