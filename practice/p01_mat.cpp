// ============================================================
//  p01_mat.cpp  ——  手册第 1 章：核心数据结构
//  练习内容：Mat 构造、属性读取、像素访问、ROI、灰度转换、存图
//  编译：cd ~/armor_detect/build && cmake .. && make p01_mat -j6
//  运行：./p01_mat
// ============================================================
#include <iostream>
#include <opencv2/opencv.hpp>
using namespace cv;

// 所有练习的输出目录（对应 Windows 桌面 cv_output 文件夹）
static const std::string OUT = "/mnt/c/Users/lenovo/Desktop/cv_output/";

int main()
{
    std::cout << "===== 第 1 章：Mat 基础 =====\n\n";

    // ---------- 1. 创建图像 ----------
    // 注意：Mat(行数=高, 列数=宽, 类型)
    Mat img = Mat::zeros(200, 300, CV_8UC3);
    std::cout << "[1] 创建 Mat::zeros(200, 300, CV_8UC3)\n";
    std::cout << "    rows(高)   = " << img.rows    << "\n";
    std::cout << "    cols(宽)   = " << img.cols    << "\n";
    std::cout << "    channels   = " << img.channels() << "\n";
    std::cout << "    type       = " << img.type()  << "   (CV_8UC3 = " << CV_8UC3 << ")\n";
    std::cout << "    size()     = " << img.size()  << "   ← 注意是 (宽, 高)\n\n";

    // ---------- 2. 访问单个像素 ----------
    // at<Vec3b>(行, 列) = at<Vec3b>(y, x)，和 Mat 构造的 (高,宽) 顺序不同！
    img.at<Vec3b>(100, 150) = Vec3b(0, 0, 255);      // BGR → 红
    Vec3b p = img.at<Vec3b>(100, 150);
    std::cout << "[2] 写像素 at<Vec3b>(100, 150) = Vec3b(0,0,255)\n";
    std::cout << "    读回 B=" << (int)p[0] << " G=" << (int)p[1]
              << " R=" << (int)p[2] << "   ← BGR 顺序，R=255 就是红色\n\n";

    // ---------- 3. ROI（感兴趣区域）+ setTo ----------
    // Rect(x, y, 宽, 高) —— 这里是 (x, y) 顺序
    Mat roi = img(Rect(50, 50, 100, 60));
    roi.setTo(Scalar(255, 0, 0));                     // BGR → 蓝
    std::cout << "[3] Rect(50, 50, 100, 60) 填成蓝色\n";
    std::cout << "    ROI 尺寸 = " << roi.cols << " x " << roi.rows << "\n\n";

    // ---------- 4. 转灰度 ----------
    Mat gray;
    cvtColor(img, gray, COLOR_BGR2GRAY);
    std::cout << "[4] cvtColor -> 灰度\n";
    std::cout << "    channels = " << gray.channels()
              << "   type = " << gray.type()
              << "   (CV_8UC1 = " << CV_8UC1 << ")\n\n";

    // ---------- 5. empty() 检查 ----------
    if (gray.empty()) {
        std::cout << "[5] 图像为空！\n";
        return -1;
    }
    std::cout << "[5] empty() = false，图像有效\n\n";

    // ---------- 6. 存图 ----------
    std::string path = OUT + "p01_mat.jpg";
    bool ok = imwrite(path, img);
    std::cout << "[6] imwrite -> " << path << "\n";
    std::cout << "    " << (ok ? "保存成功" : "保存失败（检查 cv_output 目录是否存在）") << "\n\n";

    std::cout << "===== 完成，去桌面 cv_output 文件夹看图 =====\n";
    std::cout << "图上应该有：黑色背景 + 一个蓝色矩形 + 中心一个红色像素点\n";
    return 0;
}
