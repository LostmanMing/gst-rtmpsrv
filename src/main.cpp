#include <iostream>
#include <gs_pushstream.h>


int main() {
    cv::VideoCapture cap("/home/nvidia/zgm/samples/test.mp4");
    if(!cap.isOpened()){
        spdlog::error("could not open such file ...");
        return -1;
    }
    int frame_width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    int frame_height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    int frame_fps = cap.get(cv::CAP_PROP_FPS);
    spdlog::info("video frame caps : width {} / height {} / fps {}",frame_width,frame_height,frame_fps);
    cv::Mat* frame = new cv::Mat(frame_width,frame_height,CV_8UC3);
    cv::Mat frameBuffer;
    //初始化推流参数
    VideoOptions options;
    options.width = frame_width;
    options.height = frame_height;
    options.frameRate = frame_fps;
    options.imageFormat = IMAGE_FORMAT::BGR;
    options.deviceType = DEVICE_TYPE::RTMP;
    options.uri ="rtmp://localhost/live/livestream";
    auto gs_push = GSStreamPushForSRS::create(std::move(options));
    gs_push->open();
//    float *arr = new float[3* frame_width * frame_height];
//    std::memset(arr,0,sizeof(float)*3* frame_width * frame_height);
//    cv::Mat pic =  cv::imread("/home/nvidia/zgm/samples/garbage_1_0.jpg");
    while(true){
        cap >> frameBuffer;
        if(frameBuffer.empty()){
            spdlog::error("frame empty!");
            break;
        }
        //cv::cvtColor(frameBuffer,frameBuffer,cv::COLOR_BGR2RGBA);
        //frameBuffer.copyTo(*frame);
        gs_push->Render(frameBuffer,3 * frame_width * frame_height,frame_width, frame_height);
        if ( cv::waitKey(50) == 27) {
            break;
        }
        spdlog::info("push one");
    }
    gs_push->close();
    delete frame;
    return 0;
}
