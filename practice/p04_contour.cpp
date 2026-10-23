// ============================================================
//  p04_contour.cpp  ——  手册第 6 + 7 章：轮廓处理 + 绘制
//  ★ 这是"迷你版灯条识别"：把 PDF 要求的流程完整走一遍
//    二值化 -> 找轮廓 -> minAreaRect -> 长宽比筛选 -> 排序角点 -> 画框
//  编译：cd ~/armor_detect/build && cmake .. && make p04_contour -j6
//  运行：./p04_contour
// ============================================================
#include <iostream>
#include <vector>
#include <algorithm>
#include <opencv2/opencv.hpp>
using namespace cv;

static const std::string OUT = "/mnt/c/Users/lenovo/Desktop/cv_output/";

int main()
{
    std::cout << "===== 第 6 + 7 章：迷你版灯条识别 =====\n\n";

    // ---------- 1. 造一张带"装甲板灯条"的图 ----------
    Mat src(400, 640, CV_8UC3, Scalar(30, 30, 30));

    // 两个真灯条（细长）
    rectangle(src, Rect(200, 100, 16, 150), Scalar(250, 250, 250), -1);
    rectangle(src, Rect(400, 100, 16, 150), Scalar(250, 250, 250), -1);

    // 一个假目标（方块：面积够但长宽比小，应该被过滤掉）
    rectangle(src, Rect(300, 320, 70, 60), Scalar(240, 240, 240), -1);

    // 几个小噪点
    RNG rng(7);
    for (int i = 0; i < 25; i++)
        circle(src, Point(rng.uniform(0, 639), rng.uniform(0, 399)),
               rng.uniform(1, 3), Scalar(230, 230, 230), -1);

    imwrite(OUT + "p04_src.jpg", src);
    std::cout << "[1] 测试图: 2 根真灯条 + 1 个方块(假目标) + 25 个噪点\n\n";

    // ---------- 2. 灰度 + 二值化 + 去噪 ----------
    Mat gray, binary;
    cvtColor(src, gray, COLOR_BGR2GRAY);
    threshold(gray, binary, 180, 255, THRESH_BINARY);

    Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3));
    morphologyEx(binary, binary, MORPH_OPEN, kernel);
    std::cout << "[2] 二值化(180) + 开运算去噪 完成\n\n";
    imwrite(OUT + "p04_binary.jpg", binary);

    // ---------- 3. 找轮廓 ----------
    std::vector<std::vector<Point>> contours;
    std::vector<Vec4i> hierarchy;
    // OpenCV 4.x 会修改输入图，所以传 clone()
    findContours(binary.clone(), contours, hierarchy,
                 RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    std::cout << "[3] findContours 找到 " << contours.size() << " 个轮廓\n\n";

    // ---------- 4. 逐个筛选（这就是自瞄的核心逻辑）----------
    Mat result = src.clone();
    int passed = 0;

    std::cout << "[4] 筛选过程：\n";
    std::cout << "    序号  面积    长宽比   判定\n";
    std::cout << "    ----  ------  -------  ----\n";

    for (size_t i = 0; i < contours.size(); i++) {
        const auto& c = contours[i];

        double area = contourArea(c);
        RotatedRect rr = minAreaRect(c);

        float w = rr.size.width;
        float h = rr.size.height;
        float ratio = std::max(w, h) / std::min(w, h);

        // 条件 1：面积不能太小
        bool areaOK  = area > 100;
        // 条件 2：必须细长（灯条特征）
        bool ratioOK = ratio > 2.5;

        printf("    %4zu  %6.0f  %7.2f  %s\n",
               i, area, ratio,
               (areaOK && ratioOK) ? "通过 ✓" : "过滤");

        if (!areaOK || !ratioOK) continue;

        passed++;

        // ---------- 5. 取 4 个角点并排序 ----------
        Point2f pts[4];
        rr.points(pts);

        // 关键：minAreaRect 返回的点顺序不保证，必须自己排
        // 按 y 排序 -> 前两个是上端，后两个是下端
        std::sort(pts, pts + 4,
                  [](const Point2f& a, const Point2f& b) { return a.y < b.y; });

        // 上下各按 x 排，得到 左上 / 右上 / 左下 / 右下
        if (pts[0].x > pts[1].x) std::swap(pts[0], pts[1]);
        if (pts[2].x > pts[3].x) std::swap(pts[2], pts[3]);

        // ---------- 6. 画四边形（顺序：左上→右上→右下→左下）----------
        std::vector<Point> poly = {
            Point((int)pts[0].x, (int)pts[0].y),   // 左上
            Point((int)pts[1].x, (int)pts[1].y),   // 右上
            Point((int)pts[3].x, (int)pts[3].y),   // 右下
            Point((int)pts[2].x, (int)pts[2].y)    // 左下
        };
        polylines(result, poly, true, Scalar(0, 255, 0), 2);

        // 顺便在中心画个点 + 标注长宽比
        circle(result, rr.center, 4, Scalar(0, 0, 255), -1);
        char buf[64];
        snprintf(buf, sizeof(buf), "%.1f", ratio);
        putText(result, buf, Point((int)rr.center.x - 10, (int)rr.center.y - 10),
                FONT_HERSHEY_SIMPLEX, 0.45, Scalar(0, 255, 255), 1);
    }

    std::cout << "\n    ----\n";
    std::cout << "    共 " << contours.size() << " 个轮廓，通过筛选 " << passed << " 个\n\n";

    // ---------- 7. 输出 ----------
    imwrite(OUT + "p04_result.jpg", result);
    std::cout << "[7] 结果已保存: " << OUT << "p04_result.jpg\n";
    std::cout << "    绿框 = 识别出的灯条，红点 = 中心，黄字 = 长宽比\n";
    std::cout << "    方块那个假目标应该被过滤掉（长宽比不够）\n\n";

    std::cout << "===== 完成 =====\n";
    std::cout << "对比 p04_source(原图) 和 p04_result(结果)，确认只框住了细长的灯条\n";
    return 0;
}
