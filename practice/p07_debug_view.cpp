// ============================================================
//  p07_debug_view.cpp  ——  灯条识别调参工具
//
//  作用：
//    读一帧视频，把【处理流水线的每一步】用 imshow 显示出来，
//    并带 4 个滑动条，拖动就能实时改参数，不用重新编译。
//
//  用法：
//    cd ~/armor_detect/build
//    cmake .. && make p07_debug_view -j6
//    ./p07_debug_view
//    按 q 或 ESC 退出
//
//  窗口：
//    1-frame      原始帧
//    2-red        红色掩码（只有颜色条件）
//    3-bright     亮度掩码（只有亮度条件）
//    4-mask       最终掩码（颜色 AND 亮度）
//    5-result     画了框的结果
// ============================================================
#include <iostream>
#include <opencv2/opencv.hpp>
using namespace cv;

static const std::string OUT   = "/mnt/c/Users/lenovo/Desktop/cv_output/";
static const std::string VIDEO = "/mnt/c/Users/lenovo/Desktop/装甲板视频.avi";

int main(int argc, char** argv)
{
    // ---------- 读一帧 ----------
    int frameNo = 50;
    if (argc > 1) frameNo = std::atoi(argv[1]);

    VideoCapture cap(VIDEO);
    if (!cap.isOpened()) {
        std::cout << "视频打不开: " << VIDEO << "\n";
        return -1;
    }
    cap.set(CAP_PROP_POS_FRAMES, frameNo);     // 直接跳到指定帧
    Mat frame;
    if (!cap.read(frame)) {
        std::cout << "读不到第 " << frameNo << " 帧\n";
        return -1;
    }
    cap.release();

    // ---------- 预处理（只做一次，滑动条循环里复用）----------
    Mat hsv, gray;
    cvtColor(frame, hsv,  COLOR_BGR2HSV);
    cvtColor(frame, gray, COLOR_BGR2GRAY);

    // ---------- 滑动条变量 ----------
    int tbV     = 150;    // V 下限
    int tbG     = 100;    // 灰度下限
    int tbArea  = 12;     // 面积下限
    int tbRatio = 30;     // 长宽比 ×10（30 = 3.0）

    // ---------- 建窗口 + 挂滑动条 ----------
    namedWindow("4-mask", WINDOW_AUTOSIZE);
    createTrackbar("V min",      "4-mask", &tbV,     255);
    createTrackbar("gray min",   "4-mask", &tbG,     255);
    createTrackbar("area min",   "4-mask", &tbArea,  300);
    createTrackbar("ratio x10",  "4-mask", &tbRatio, 100);

    printf("拖动 4-mask 窗口里的滑动条实时调参，按 q 退出\n");
    printf("帧号 = %d\n\n", frameNo);

    // 可选：设置环境变量 P07_AUTO=<次数> 可在指定轮数后自动退出（便于脚本测试）
    int autoLoop = 0;
    if (const char* e = std::getenv("P07_AUTO")) autoLoop = std::atoi(e);
    int loopCount = 0;

    int lastV = -1, lastG = -1, lastA = -1, lastR = -1;

    Mat mask, result;          // 放在循环外，退出时还要用来存图

    while (true) {
        int v      = tbV;
        int g      = tbG;
        double amin = tbArea;
        double rmin = tbRatio / 10.0;

        // ---------- ① 红色掩码（H 双区间，V 用滑动条）----------
        Mat m1, m2, red;
        inRange(hsv, Scalar(0,   60, v), Scalar(30,  255, 255), m1);   // 橙红段
        inRange(hsv, Scalar(170, 60, v), Scalar(180, 255, 255), m2);   // 纯红段
        bitwise_or(m1, m2, red);

        // ---------- ② 亮度掩码 ----------
        Mat bright;
        threshold(gray, bright, g, 255, THRESH_BINARY);

        // ---------- ③ 合并：既红又亮 ----------
        bitwise_and(red, bright, mask);

        // ---------- ④ 找轮廓 + 筛选 + 画框 ----------
        std::vector<std::vector<Point>> contours;
        std::vector<Vec4i> hier;
        findContours(mask.clone(), contours, hier,
                     RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

        result = frame.clone();
        int pass = 0;

        for (size_t i = 0; i < contours.size(); i++) {
            const auto& c = contours[i];
            double area = contourArea(c);
            RotatedRect rr = minAreaRect(c);
            float w = rr.size.width, ht = rr.size.height;
            float ratio = std::max(w, ht) / std::min(w, ht);

            if (area <= amin || ratio <= rmin) continue;
            pass++;

            Point2f pts[4];
            rr.points(pts);
            std::sort(pts, pts + 4,
                      [](const Point2f& a, const Point2f& b) { return a.y < b.y; });
            if (pts[0].x > pts[1].x) std::swap(pts[0], pts[1]);
            if (pts[2].x > pts[3].x) std::swap(pts[2], pts[3]);

            std::vector<Point> poly = {
                Point((int)pts[0].x, (int)pts[0].y),   // 左上
                Point((int)pts[1].x, (int)pts[1].y),   // 右上
                Point((int)pts[3].x, (int)pts[3].y),   // 右下
                Point((int)pts[2].x, (int)pts[2].y)    // 左下
            };
            polylines(result, poly, true, Scalar(0, 255, 0), 2);

            // 在框旁边标出面积和长宽比
            char txt[64];
            snprintf(txt, sizeof(txt), "%.0f/%.1f", area, ratio);
            putText(result, txt, Point((int)pts[0].x, (int)pts[0].y - 5),
                    FONT_HERSHEY_SIMPLEX, 0.4, Scalar(0, 255, 255), 1);
        }

        // ---------- ⑤ 参数变了才打印，避免刷屏 ----------
        if (v != lastV || g != lastG || tbArea != lastA || tbRatio != lastR) {
            printf("V>=%3d  灰度>=%3d  面积>%3.0f  长宽比>%.1f  "
                   "→ 轮廓 %zu 个，通过 %d 个\n",
                   v, g, amin, rmin, contours.size(), pass);
            lastV = v; lastG = g; lastA = tbArea; lastR = tbRatio;
        }

        // ---------- ⑥ 显示 ----------
        imshow("1-frame",  frame);
        imshow("2-red",    red);
        imshow("3-bright", bright);
        imshow("4-mask",   mask);
        imshow("5-result", result);

        int key = waitKey(30);
        if (key == 'q' || key == 27) break;      // q 或 ESC 退出

        if (autoLoop > 0 && ++loopCount >= autoLoop) break;
    }

    destroyAllWindows();

    // ---------- 退出时把当前结果存一份，方便对照 ----------
    imwrite(OUT + "p07_mask.png", mask);
    imwrite(OUT + "p07_result.jpg", result);
    printf("\n已保存 p07_mask.png 和 p07_result.jpg\n");
    return 0;
}
