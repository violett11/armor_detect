// ============================================================
//  p06_video.cpp  ——  视频读写教学样例
//
//  演示四件事：
//    ① 打开视频 + 读属性（分辨率 / 帧率 / 总帧数 / 时长）
//    ② 逐帧读取，按间隔存图片
//    ③ 把每帧写进一个新视频（VideoWriter）
//    ④ 重新打开输出视频，验证帧数对不对
//
//  编译：cd ~/armor_detect/build && cmake .. && make p06_video -j6
//  运行：./p06_video
// ============================================================
#include <iostream>
#include <opencv2/opencv.hpp>
using namespace cv;

// 所有输出图片/视频都放这个目录
static const std::string OUT = "/mnt/c/Users/lenovo/Desktop/cv_output/";

int main()
{
    // ========================================================
    // ① 打开视频
    // ========================================================
    std::string path = "/mnt/c/Users/lenovo/Desktop/装甲板视频.avi";

    VideoCapture cap(path);
    if (!cap.isOpened()) {
        std::cout << "打开视频失败: " << path << "\n";
        std::cout << "  检查路径是否正确、文件是否存在\n";
        return -1;
    }

    // ========================================================
    // ② 读属性（这些信息都在视频文件头里，不用逐帧读）
    // ========================================================
    double fps   = cap.get(CAP_PROP_FPS);            // 帧率
    double total = cap.get(CAP_PROP_FRAME_COUNT);    // 总帧数
    int    w     = (int)cap.get(CAP_PROP_FRAME_WIDTH);   // 宽
    int    h     = (int)cap.get(CAP_PROP_FRAME_HEIGHT);  // 高

    std::cout << "===== 视频信息 =====\n";
    std::cout << "  路径    : " << path << "\n";
    std::cout << "  分辨率  : " << w << " x " << h << "\n";
    std::cout << "  帧率    : " << fps << " fps\n";
    std::cout << "  总帧数  : " << total << "\n";
    std::cout << "  时长    : " << (total / fps) << " 秒\n\n";

    // ========================================================
    // ③ 准备输出视频
    //    参数：路径, 编码, 帧率, 尺寸
    //    ⚠️ 尺寸必须和帧的实际尺寸完全一致
    // ========================================================
    std::string outPath = OUT + "p06_out.avi";

    VideoWriter writer(outPath,
                       VideoWriter::fourcc('M', 'J', 'P', 'G'),   // 编码
                       fps,                                       // 帧率
                       Size(w, h));                               // 尺寸
    if (!writer.isOpened()) {
        std::cout << "VideoWriter 打开失败: " << outPath << "\n";
        return -1;
    }

    // ========================================================
    // ④ 逐帧循环
    //    cap.read(frame) 返回 false 表示读完了（或出错）
    // ========================================================
    Mat frame;
    int idx = 0;          // 当前帧号
    int savedPics = 0;    // 存了几张图

    while (cap.read(frame)) {
        if (frame.empty()) break;      // 双保险：读到空帧也退出

        // ---- 每 20 帧存一张图片，方便肉眼检查 ----
        if (idx % 20 == 0) {
            std::string pic = OUT + "p06_frame_" + std::to_string(idx) + ".jpg";
            imwrite(pic, frame);
            savedPics++;
            std::cout << "  存图: " << pic << "\n";
        }

        // ---- 把这一帧原样写进输出视频 ----
        writer.write(frame);

        idx++;
    }

    // ========================================================
    // ⑤ 收尾（很重要，不 release 文件可能不完整）
    // ========================================================
    cap.release();
    writer.release();

    std::cout << "\n===== 读取结果 =====\n";
    std::cout << "  实际读到帧数: " << idx << "\n";
    std::cout << "  存了图片数  : " << savedPics << "\n";

    // ========================================================
    // ⑥ 验证输出视频（重新打开数一遍）
    // ========================================================
    VideoCapture check(outPath);
    if (!check.isOpened()) {
        std::cout << "  输出视频打不开！\n";
        return -1;
    }
    double outTotal = check.get(CAP_PROP_FRAME_COUNT);
    double outFps   = check.get(CAP_PROP_FPS);
    int    outW     = (int)check.get(CAP_PROP_FRAME_WIDTH);
    int    outH     = (int)check.get(CAP_PROP_FRAME_HEIGHT);
    check.release();

    std::cout << "\n===== 输出视频验证 =====\n";
    std::cout << "  文件      : " << outPath << "\n";
    std::cout << "  分辨率    : " << outW << " x " << outH << "\n";
    std::cout << "  帧率      : " << outFps << " fps\n";
    std::cout << "  帧数      : " << outTotal << "\n";

    if ((int)outTotal == idx)
        std::cout << "  ✓ 帧数一致（" << idx << " 帧），转码成功\n";
    else
        std::cout << "  ⚠️ 帧数不一致：原 " << idx << "，输出 " << outTotal << "\n";

    return 0;
}
