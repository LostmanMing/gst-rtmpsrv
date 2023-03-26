//
// Created by 迷路的男人 on 2023/3/24.
//

#ifndef SRS_GSTREAMER_GS_PUSHSTREAM_H
#define SRS_GSTREAMER_GS_PUSHSTREAM_H
#include <iostream>
#include <sstream>
#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <spdlog/spdlog.h>
#include <opencv2/opencv.hpp>
enum class IMAGE_FORMAT{
    JPEG,BGRA,BGR,GRAY16,GRAY8,RGB,RGBA
};
enum class DEVICE_TYPE{
    FILE,RTMP
};

class VideoOptions{
public:
    IMAGE_FORMAT imageFormat;
    DEVICE_TYPE deviceType;
    int frameRate;
    int width;
    int height;
    std::string uri;
};

class GSStreamPushForSRS {
public:
    //constructer

    explicit GSStreamPushForSRS(VideoOptions& videoOption);
    static GSStreamPushForSRS* create(VideoOptions &&videoOptions);
    bool init();
    bool gstreamerInit();
    bool buildLaunchStr();
    static void cb_need_data (GstElement *appsrc,guint unused_size, gpointer user_data);
    static void cb_enough_data(GstElement *appsrc,guint unused_size, gpointer user_data);

    //发送bgr格式图片，并并封装成视频流
    bool Render(cv::Mat &image, uint32_t size, uint32_t width, uint32_t height);
    bool open();
    void checkMsgBus();
    bool buildCapsStr();
    void close();

    ~GSStreamPushForSRS();
private:
    VideoOptions videoOption;

    bool gstreamerInitialized = false;
    std::string mLaunchStr;
    std::string mCapsStr;

    _GstElement* mPipeline = nullptr;
    _GstBus* mBus = nullptr;
    _GstElement * mAppSrc = nullptr;
    _GstCaps *mBufferCaps= nullptr;

    std::atomic<bool> mNeedData;
    std::atomic<bool> mStreaming;

};
#endif //SRS_GSTREAMER_GS_PUSHSTREAM_H
