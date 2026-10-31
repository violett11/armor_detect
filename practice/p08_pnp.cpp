// ============================================================
//  p08_pnp.cpp  ——  PnP 位姿解算样例（PDF 选做部分）
//
//  流程：
//    单帧 → HSV 红色掩码 + 亮度掩码 → 筛选出 2 根灯条
//         → 取 4 个端点（左上/右上/右下/左下）
//         → solvePnP 求位姿 (rvec, tvec)
//         → projectPoints 把 3 个坐标轴投影回图像
//         → 画四边形 + 坐标轴
//
//  坐标系（PDF 第 7 页 FLU）：
//    原点 = 装甲板中心
//    x 轴 = 装甲板前方（法线，朝外指向相机）
//    y 轴 = 装甲板左侧
//    z 轴 = 装甲板上方
//
//  编译：cd ~/armor_detect/build && cmake .. && make p08_pnp -j6
//  运行：./p08_pnp [帧号]
// ============================================================
#include <iostream>
#include <opencv2/opencv.hpp>
using namespace cv;

static const std::string OUT   = "/mnt/c/Users/lenovo/Desktop/cv_output/";
static const std::string VIDEO = "/mnt/c/Users/lenovo/Desktop/装甲板视频.avi";

// ---------- 相机内参（PDF 第 8 页）----------
static const Mat K = (Mat_<double>(3, 3) <<
    928.130989,   0.0,        377.572945,
      0.0,      930.138391,   283.892859,
      0.0,        0.0,          1.0);

// ---------- 畸变系数（PDF 第 8 页）----------
static const Mat DIST = (Mat_<double>(1, 5) <<
    -0.254433647, 0.569431382, 0.00365405229, -0.00109433818, -1.33846840);

// ---------- 装甲板端点矩形的实际尺寸（mm）----------
static const double PLATE_W = 130.0;   // 两灯条中心水平间距
static const double PLATE_H = 50.0;    // 灯条长度

// 取灯条的上端点 / 下端点
static void getEnds(const RotatedRect& rr, Point2f& top, Point2f& bottom)
{
    Point2f p[4];
    rr.points(p);
    std::sort(p, p + 4, [](const Point2f& a, const Point2f& b) { return a.y < b.y; });
    top    = (p[0] + p[1]) * 0.5f;
    bottom = (p[2] + p[3]) * 0.5f;
}

int main(int argc, char** argv)
{
    int frameNo = 50;
    if (argc > 1) frameNo = std::atoi(argv[1]);

    // ---------- ① 读一帧 ----------
    VideoCapture cap(VIDEO);
    if (!cap.isOpened()) { std::cout << "视频打不开\n"; return -1; }
    cap.set(CAP_PROP_POS_FRAMES, frameNo);
    Mat frame;
    if (!cap.read(frame)) { std::cout << "读不到该帧\n"; return -1; }
    cap.release();

    // ---------- ② 掩码：颜色 + 亮度 ----------
    Mat hsv, gray;
    cvtColor(frame, hsv,  COLOR_BGR2HSV);
    cvtColor(frame, gray, COLOR_BGR2GRAY);

    Mat m1, m2, red;
    inRange(hsv, Scalar(0,   60, 150), Scalar(30,  255, 255), m1);
    inRange(hsv, Scalar(170, 60, 150), Scalar(180, 255, 255), m2);
    bitwise_or(m1, m2, red);

    Mat bright;
    threshold(gray, bright, 100, 255, THRESH_BINARY);

    Mat mask;
    bitwise_and(red, bright, mask);

    // ---------- ③ 找轮廓 + 筛选 ----------
    std::vector<std::vector<Point>> contours;
    std::vector<Vec4i> hier;
    findContours(mask.clone(), contours, hier, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    std::vector<RotatedRect> lamps;
    for (auto& c : contours) {
        double area = contourArea(c);
        RotatedRect rr = minAreaRect(c);
        float w = rr.size.width, h = rr.size.height;
        float ratio = std::max(w, h) / std::min(w, h);
        if (area <= 12 || ratio <= 3.0) continue;
        lamps.push_back(rr);
    }

    printf("找到 %zu 根灯条\n", lamps.size());
    if (lamps.size() != 2) {
        printf("灯条数不是 2，本样例只处理正好 2 根的情况\n");
        imwrite(OUT + "p08_mask.png", mask);
        return 0;
    }

    // ---------- ④ 左灯条 / 右灯条 ----------
    if (lamps[0].center.x > lamps[1].center.x)
        std::swap(lamps[0], lamps[1]);

    Point2f lTop, lBottom, rTop, rBottom;
    getEnds(lamps[0], lTop,    lBottom);     // 左
    getEnds(lamps[1], rTop,    rBottom);     // 右

    printf("\n=== 4 个端点（像素）===\n");
    printf("  左上 (%.1f, %.1f)\n", lTop.x,    lTop.y);
    printf("  右上 (%.1f, %.1f)\n", rTop.x,    rTop.y);
    printf("  右下 (%.1f, %.1f)\n", rBottom.x, rBottom.y);
    printf("  左下 (%.1f, %.1f)\n", lBottom.x, lBottom.y);

    // ---------- ⑤ 物点（FLU：x=前/法线, y=板左侧, z=板上方）----------
    // 注意：板自己的"左侧"在画面里出现在【右】边（像看人脸，他的左手在你右边）
    //       所以 图像左上 ← object(y=-W/2, z=+H/2)
    std::vector<Point3f> objectPoints = {
        Point3f(0.0f, (float)(-PLATE_W / 2), (float)( PLATE_H / 2)),   // 图像左上
        Point3f(0.0f, (float)( PLATE_W / 2), (float)( PLATE_H / 2)),   // 图像右上
        Point3f(0.0f, (float)( PLATE_W / 2), (float)(-PLATE_H / 2)),   // 图像右下
        Point3f(0.0f, (float)(-PLATE_W / 2), (float)(-PLATE_H / 2))    // 图像左下
    };

    // ---------- ⑥ 像点（顺序必须和物点一一对应）----------
    std::vector<Point2f> imagePoints = { lTop, rTop, rBottom, lBottom };

    // ---------- ⑦ solvePnP（共面 4 点用 IPPE，会返回 2 个解）----------
    std::vector<Mat> rvecs, tvecs;
    solvePnPGeneric(objectPoints, imagePoints, K, DIST, rvecs, tvecs,
                    false, SOLVEPNP_IPPE);

    if (tvecs.empty()) {
        printf("solvePnP 失败\n");
        return -1;
    }

    printf("\n=== solvePnP 给出 %zu 个解 ===\n", tvecs.size());
    int best = 0;
    for (size_t i = 0; i < tvecs.size(); i++) {
        Mat Ri;
        Rodrigues(rvecs[i], Ri);
        Mat ni = Ri * (Mat_<double>(3, 1) << 1, 0, 0);   // 该解下的法线
        double angI = norm(rvecs[i]) * 180.0 / CV_PI;

        printf("  解 %zu: tvec = (%8.2f, %8.2f, %8.2f) mm  距离 %.1f mm\n",
               i, tvecs[i].at<double>(0), tvecs[i].at<double>(1),
               tvecs[i].at<double>(2), norm(tvecs[i]));
        printf("        旋转角 %.2f 度   法线 = (%6.3f, %6.3f, %6.3f)  %s\n",
               angI, ni.at<double>(0), ni.at<double>(1), ni.at<double>(2),
               ni.at<double>(2) < 0 ? "朝相机 ←" : "背离相机");
        if (tvecs[i].at<double>(2) > 0 && tvecs[best].at<double>(2) <= 0)
            best = (int)i;
    }
    printf("  采用解 %d\n", best);

    Mat rvec = rvecs[best], tvec = tvecs[best];

    // 打印旋转角（旋转向量的模长 = 旋转角度，弧度）
    Mat R;
    Rodrigues(rvec, R);
    double angleDeg = norm(rvec) * 180.0 / CV_PI;
    printf("\n=== 位姿 ===\n");
    printf("  tvec (mm)      = (%.2f, %.2f, %.2f)\n",
           tvec.at<double>(0), tvec.at<double>(1), tvec.at<double>(2));
    printf("  距离 |tvec|    = %.1f mm\n", norm(tvec));
    printf("  旋转角         = %.1f 度\n", angleDeg);
    printf("  rvec           = (%.4f, %.4f, %.4f)\n",
           rvec.at<double>(0), rvec.at<double>(1), rvec.at<double>(2));

    // ---------- ⑧ 画结果 ----------
    Mat result = frame.clone();

    // 原来的两根灯条（细蓝线）
    for (auto& rr : lamps) {
        Point2f p[4];
        rr.points(p);
        std::vector<Point> poly;
        for (int i = 0; i < 4; i++) poly.push_back(Point((int)p[i].x, (int)p[i].y));
        polylines(result, poly, true, Scalar(255, 100, 0), 1);
    }

    // 装甲板四边形（绿粗线）
    std::vector<Point> plate = {
        Point((int)lTop.x,    (int)lTop.y),
        Point((int)rTop.x,    (int)rTop.y),
        Point((int)rBottom.x, (int)rBottom.y),
        Point((int)lBottom.x, (int)lBottom.y)
    };
    polylines(result, plate, true, Scalar(0, 255, 0), 2);

    // ---------- ⑨ 画坐标轴 ----------
    double AXIS_LEN = 50.0;      // 轴长 50mm
    std::vector<Point3f> axisPoints = {
        Point3f(0, 0, 0),                          // 原点
        Point3f((float)AXIS_LEN, 0, 0),            // x：前（法线）
        Point3f(0, (float)AXIS_LEN, 0),            // y：左
        Point3f(0, 0, (float)AXIS_LEN)             // z：上
    };

    std::vector<Point2f> proj;
    projectPoints(axisPoints, rvec, tvec, K, DIST, proj);

    // 打印投影结果，便于判断轴向对不对
    printf("\n=== 坐标轴投影到图像（像素）===\n");
    const char* names[4] = {"原点", "x端", "y端", "z端"};
    for (int i = 0; i < 4; i++) {
        double len = (i == 0) ? 0.0 : norm(proj[i] - proj[0]);
        printf("  %-4s (%.1f, %.1f)   距原点 %.1f px\n",
               names[i], proj[i].x, proj[i].y, len);
    }

    // 打印装甲板法线（物体系 x 轴）在相机坐标系下的方向
    Mat R2;
    Rodrigues(rvec, R2);
    Mat nx = R2 * (Mat_<double>(3, 1) << 1, 0, 0);
    printf("\n=== 法线方向（物体系 x 轴 → 相机坐标系）===\n");
    printf("  (%.3f, %.3f, %.3f)\n",
           nx.at<double>(0), nx.at<double>(1), nx.at<double>(2));
    printf("  说明：相机 z 轴指向正前方，法线应接近 (0,0,-1) 表示板面朝向相机\n");
    printf("  法线与光轴夹角 = %.1f 度\n",
           acos(std::min(1.0, std::max(-1.0, -nx.at<double>(2)))) * 180.0 / CV_PI);

    line(result, proj[0], proj[1], Scalar(0, 0, 255), 3);      // x 红
    line(result, proj[0], proj[2], Scalar(0, 255, 0), 3);      // y 绿
    line(result, proj[0], proj[3], Scalar(255, 0, 0), 3);      // z 蓝
    circle(result, proj[0], 4, Scalar(0, 255, 255), -1);       // 原点 黄点

    putText(result, "x", proj[1], FONT_HERSHEY_SIMPLEX, 0.6, Scalar(0, 0, 255), 2);
    putText(result, "y", proj[2], FONT_HERSHEY_SIMPLEX, 0.6, Scalar(0, 255, 0), 2);
    putText(result, "z", proj[3], FONT_HERSHEY_SIMPLEX, 0.6, Scalar(255, 0, 0), 2);

    // 左下角打印距离
    char info[128];
    snprintf(info, sizeof(info), "dist = %.0f mm", norm(tvec));
    putText(result, info, Point(20, frame.rows - 25), FONT_HERSHEY_SIMPLEX,
            0.8, Scalar(0, 255, 255), 2);

    imwrite(OUT + "p08_result.jpg", result);
    imwrite(OUT + "p08_mask.png", mask);
    printf("\n已保存 p08_result.jpg 和 p08_mask.png\n");

    imshow("p08 result", result);
    waitKey(0);
    destroyAllWindows();
    return 0;
}
