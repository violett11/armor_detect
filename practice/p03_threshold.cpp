// ============================================================
//  p03_threshold.cpp  ——  手册第 5 章：二值化与形态学
//  练习内容：threshold 各种类型、OTSU 自动阈值、morphologyEx 去噪
//  编译：cd ~/armor_detect/build && cmake .. && make p03_threshold -j6
//  运行：./p03_threshold
// ============================================================
#include <iostream>
#include <opencv2/opencv.hpp>
using namespace cv;

static const std::string OUT = "/mnt/c/Users/lenovo/Desktop/cv_output/";

int main()
{
    std::cout << "===== 第 5 章：二值化与形态学 =====\n\n";

    // ---------- 1. 造图：亮灯条 + 若干噪点 ----------
    Mat src(300, 480, CV_8UC3, Scalar(35, 35, 35));
    rectangle(src, Rect(140, 80, 18, 140), Scalar(250, 250, 250), -1);
    rectangle(src, Rect(322, 80, 18, 140), Scalar(250, 250, 250), -1);

    // 撒一些小噪点（模拟干扰）
    RNG rng(12345);
    for (int i = 0; i < 60; i++) {
        int x = rng.uniform(0, 479);
        int y = rng.uniform(0, 299);
        circle(src, Point(x, y), rng.uniform(1, 3), Scalar(200, 200, 200), -1);
    }
    imwrite(OUT + "p03_src.jpg", src);
    std::cout << "[1] 生成测试图：2 根灯条 + 60 个随机噪点\n\n";

    // ---------- 2. 转灰度再二值化 ----------
    Mat gray;
    cvtColor(src, gray, COLOR_BGR2GRAY);

    // 手工阈值
    Mat th1;
    threshold(gray, th1, 150, 255, THRESH_BINARY); 
    std::cout << "[2] threshold(thresh=150, THRESH_BINARY)\n";
    std::cout << "    白色像素数 = " << countNonZero(th1) << "\n";
    imwrite(OUT + "p03_binary.jpg", th1);

    // OTSU 自动阈值（compare 一下差别）
    Mat th2;
    double otsuVal = threshold(gray, th2, 0, 255, THRESH_BINARY | THRESH_OTSU);
    std::cout << "    OTSU 自动算出的阈值 = " << otsuVal << "\n";
    std::cout << "    OTSU 白色像素数 = " << countNonZero(th2) << "\n\n";
    imwrite(OUT + "p03_otsu.jpg", th2);

    // 反相二值化（演示 THRESH_BINARY_INV）
    Mat th3;
    threshold(gray, th3, 150, 255, THRESH_BINARY_INV);
    imwrite(OUT + "p03_binary_inv.jpg", th3);
    std::cout << "    THRESH_BINARY_INV 也存了一张（黑白反过来）\n\n";

    // ---------- 3. 形态学去噪（开运算去小白点）----------
    Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3));

    Mat opened;
    morphologyEx(th1, opened, MORPH_OPEN, kernel);
    std::cout << "[3] MORPH_OPEN 开运算（去噪点）\n";
    std::cout << "    处理前白像素 = " << countNonZero(th1) << "\n";
    std::cout << "    处理后白像素 = " << countNonZero(opened)
              << "   ← 应该明显减少\n";
    imwrite(OUT + "p03_open.jpg", opened);

    // 闭运算：填补内部小黑洞
    Mat closed;
    morphologyEx(th1, closed, MORPH_CLOSE, kernel);
    imwrite(OUT + "p03_close.jpg", closed);
    std::cout << "    MORPH_CLOSE 也存了一张（补洞）\n\n";

    // ---------- 4. 对比不同核大小 ----------
    std::cout << "[4] 不同核大小的开运算效果对比\n";
    for (int k : {3, 5, 7}) {
        Mat kk = getStructuringElement(MORPH_RECT, Size(k, k));
        Mat tmp;
        morphologyEx(th1, tmp, MORPH_OPEN, kk);
        std::cout << "    核 " << k << "x" << k
                  << " -> 白像素 " << countNonZero(tmp) << "\n";
        imwrite(OUT + "p03_open_" + std::to_string(k) + ".jpg", tmp);
    }

    std::cout << "\n===== 完成 =====\n";
    std::cout << "重点对比这两张：\n";
    std::cout << "  p03_binary.jpg   有很多小白点（噪点）\n";
    std::cout << "  p03_open.jpg     噪点被清掉了，只剩灯条\n";
    return 0;
}
