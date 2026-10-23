// ============================================================
//  p02_io_color.cpp  ——  手册第 2 章 + 第 4 章
//  练习内容：imwrite / imread、cvtColor、split / merge、inRange 做掩码
//  编译：cd ~/armor_detect/build && cmake .. && make p02_io_color -j6
//  运行：./p02_io_color
// ============================================================
#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
using namespace cv;

static const std::string OUT = "/mnt/c/Users/lenovo/Desktop/cv_output/";

int main()
{
    std::cout << "===== 第 2 + 4 章：图像 IO 与颜色空间 =====\n\n";

    // ---------- 1. 先造一张"假装甲板"图 ----------
    // 深灰背景 + 两根白色亮灯条（模拟真实场景）
    Mat src(300, 480, CV_8UC3, Scalar(35, 35, 35));

    // 左灯条：Rect(x, y, 宽, 高) —— 细长
    rectangle(src, Rect(140, 80, 18, 140), Scalar(250, 250, 250), -1);
    // 右灯条
    rectangle(src, Rect(322, 80, 18, 140), Scalar(250, 250, 250), -1);
    // 干扰：一个暗一点的方块（后面用来测试过滤）
    rectangle(src, Rect(200, 240, 60, 40), Scalar(120, 120, 120), -1);

    std::cout << "[1] 生成测试图 480x300，含两根白色灯条 + 一个干扰块\n";
    imwrite(OUT + "p02_src.jpg", src);

    // ---------- 2. imread 读回来（验证读写链路）----------
    std::string path = OUT + "p02_src.jpg";
    Mat img = imread(path, IMREAD_COLOR);
    if (img.empty()) {
        std::cout << "[2] 读图失败！路径: " << path << "\n";
        std::cout << "    请先确认 cv_output 目录存在\n";
        return -1;
    }
    std::cout << "[2] imread 成功: " << img.cols << " x " << img.rows
              << "  channels=" << img.channels() << "\n\n";

    // ---------- 3. cvtColor 转灰度 / HSV ----------
    Mat gray, hsv;
    cvtColor(img, gray, COLOR_BGR2GRAY);
    cvtColor(img, hsv,  COLOR_BGR2HSV);
    std::cout << "[3] cvtColor 完成\n";
    std::cout << "    灰度图类型 = " << gray.type() << " (应为 CV_8UC1)\n";
    imwrite(OUT + "p02_gray.jpg", gray);

    // HSV 里 H 的范围是 0~179
    Vec3b hsvBar = hsv.at<Vec3b>(150, 148);   // 左灯条内部一点
    std::cout << "    灯条处的 HSV = H:" << (int)hsvBar[0]
              << " S:" << (int)hsvBar[1]
              << " V:" << (int)hsvBar[2]
              << "   ← 灯条发光，V 应该很高\n\n";

    // ---------- 4. split / merge ----------
    std::vector<Mat> channels;
    split(img, channels);           // BGR → 3 个单通道
    std::cout << "[4] split 得到 " << channels.size() << " 个通道\n";
    std::cout << "    channels[0]=蓝  [1]=绿  [2]=红\n";
    imwrite(OUT + "p02_ch0_blue.jpg", channels[0]);

    Mat merged;
    merge(channels, merged);        // 再合回去
    std::cout << "    merge 后 channels = " << merged.channels() << "\n\n";

    // ---------- 5. inRange 做掩码（灯条识别的关键一步）----------
    // 思路：灯条"亮"(V 高) + "不太饱和"(S 低，因为偏白)
    Mat mask;
    inRange(hsv, Scalar(0, 0, 200),        // H 任意, S>=0, V>=200
                 Scalar(180, 60, 255),     // H<=180, S<=60, V<=255
                 mask);

    int whiteCount = countNonZero(mask);
    std::cout << "[5] inRange 提取高亮低饱和区域\n";
    std::cout << "    掩码中白色像素数 = " << whiteCount << "\n";
    imwrite(OUT + "p02_mask.jpg", mask);

    std::cout << "\n===== 完成 =====\n";
    std::cout << "去桌面 cv_output 看这几张图：\n";
    std::cout << "  p02_src.jpg       原图（两根白灯条）\n";
    std::cout << "  p02_gray.jpg      灰度图\n";
    std::cout << "  p02_ch0_blue.jpg  蓝色通道\n";
    std::cout << "  p02_mask.jpg      掩码（应该只剩灯条是白的）\n";
    return 0;
}
