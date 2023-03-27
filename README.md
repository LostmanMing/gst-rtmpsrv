# **gst-rtmpsrv**

将server端opencv读取的视频帧通过gstreamer推流到rtmp服务器上。

## **Quick Start**

### 环境配置

1. 安装 `gstreamer`
   `sudo apt-get install libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libgstreamer-plugins-bad1.0-dev gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly gstreamer1.0-libav gstreamer1.0-doc gstreamer1.0-tools gstreamer1.0-x gstreamer1.0-alsa gstreamer1.0-gl gstreamer1.0-gtk3 gstreamer1.0-qt5 gstreamer1.0-pulseaudio`
2. 安装`deepstream`
   需要使用deepstream提供的元素使用N卡资源加速格式转换。例如 `nvvideoconvert` 、`nvv4l2h264enc`等。
   找到和本机 `cuda` 版本对应的 `deepstream` 下载。  
   [deepstream下载地址](https://developer.nvidia.com/deepstream-getting-started#downloads)
3. 部署 `rtmp` 服务器
   `rtmp` 服务器使用 `srs`
   
   - 安装 `docker`
     ```
     curl -fsSL https://get.docker.com -o get-docker.sh
     sudo sh get-docker.sh --mirror=Aliyun       # 一键安装 docker
     sudo systemctl start docker //启动docker  
     sudo systemctl enable docker //开机启动docker
     ```
   - 拉取 `srs` 镜像  
     `sudo docker pull ossrs/srs`
   - 创建并运行容器  
     `docker run --rm -it --net=host -p 1935:1935 -p 1985:1985 -p 8080:8080 registry.cn-hangzhou.aliyuncs.com/ossrs/srs:4 ./objs/srs -c conf/docker.conf`  
     如果有特殊需求，如低延迟等，可更改配置文件
4. 运行代码，并在 `srs` 控制台(`rtmp`服务器`8080`端口查看推流情况)。

## **管道元素**

### 文件推流

文件推流使用的管道如下：

`gst-launch-1.0 filesrc location=test.mp4 ! qtdemux name=demux demux.video_0  ! h264parse ! nvv4l2decoder ! nvvideoconvert ! nvv4l2h264enc control-rate=1 bitrate=2000000 ! h264parse ! flvmux ! rtmpsink location="rtmp://localhost/live/livestream"`

- filesrc  
  从文件系统读取视频文件。  
  输入：无  
  输出：视频数据流  
- qtdemux  
  将QuickTime MP4视频封装中的音视频流分离。  
  输入：视频数据流  
  输出：视频数据流、音频数据流  
- demux.video_0  
  作用：选择分离后的第一个，即视频流。  
  输入：视频数据流、音频数据流  
  输出：视频数据流  
- h264parse  
  作用：解析`H.264`视频流。  
  输入：视频数据流  
  输出：`H.264`视频流  
- nvv4l2decoder    
  作用：使用NVIDIA硬件解码器解码`H.264`视频流。  
  输入：`H.264`视频流  
  输出：解码后的视频数据流,可指定格式，一般为`NV12`  
- nvvideoconvert  
  作用：将解码后的视频数据流从源数据格式转换为nvidia支持格式。  
  输入：解码后的视频数据流  
  输出：转换后的视频数据流  
  **注**：`nvvideoconvert` 的输入格式并不直接支持 `BGR` 格式，只支持部分常见的像素格式，包括 `RGBA`、`NV12`、`NV16`、`NV24`、`UYVY`和`YUY2`等。因此，如果要将 `BGR` 格式的图像转换为`video/x-raw(memory:NVMM), format=NV12`格式，需要在`nvvideoconvert`之前添加一个`videoconvert`元素，将输入的 `BGR` 格式图像转换为 RGBA 格式的图像，再将 RGBA 格式的图像输入到`nvvideoconvert` 中进行转换。  
- nvv4l2h264enc  
  作用：使用NVIDIA硬件编码器将视频流编码为`H.264`格式。  
  输入：转换后的视频数据流  
  输出：`H.264`视频流  
- h264parse  
  作用：对编码后的`H.264`视频流进行解析。  
  输入：H.264视频流  
  输出：解析后的`H.264`视频流  
- flvmux  
  作用：将`H.264`视频流封装成`FLV`格式。  
  输入：解析后的`H.264`视频流  
  输出：`FLV`视频流  
- rtmpsink  
  作用：将`FLV`格式的视频流推送到`RTMP`服务器。  
  输入：`FLV`视频流  
  输出：无  
 
### OpenCV视频帧推流

视频帧推流管道如下：
`gst-launch-1.0 appsrc name=mysource  is-live=true do-timestamp=true format=3  ! videoconvert ! video/x-raw, format=RGBA ! nvvideoconvert ! video/x-raw(memory:NVMM),format=NV12, width=1920 , height=1080 ,framerate=9/1 !nvv4l2h264enc control-rate=1 bitrate=2000000 !  h264parse  ! flvmux  ! rtmpsink location=rtmp://localhost/live/livestream`

appsrc 构建str,即输出格式`video/x-raw,format=BGR , width=1920, height=1080, framerate=9/1`

- appsrc  
  作用：从应用程序中获取视频数据的源元素。  
  `name`: 元素的名称，用于在管道中引用该元素。  
  `is-live`: 标志位，表示数据源是实时的。  
  `do-timestamp`: 标志位，表示元素应该自动生成时间戳。  
  `format`: 输入视频数据的格式。  
  输入：无  
  输出：程序内设置  
- videoconvert  
  作用：将不同格式的视频数据转换为指定格式，本项目为BGR->RGBA格式。  
  输入：`video/x-raw`：原始的视频数据格式，包括色彩空间、帧率、分辨率等参数，常见的子格式有`RGB`、`YUV`、`GRAY`、`RGBA`、`BGR`等。本项目为BGR。  
  输出：`video/x-raw,format=RGB、BGR、GRAY8、I420、YV12、NV12、NV21、YUY2、UYVY、ARGB、BGRA、RGBA、ABGR`。不管是输入还是输出，具体支持格式还是看`gstreamer`安装的插件，本项目为`RGBA`格式。  
- nvvideoconvert  
  作用：将原始视频帧格式转化为`nvidia gpu`可处理的格式。  
  输入：`RGBA`、`NV12`、`NV16`、`NV24`、`UYVY`、`BGRx`和 `YUY2` 等，由于不支持opencv读取的`BGR`格式所以本项目才需要使用 `videoconvert` 将 `BGR` 转为 `RGBA` 作为 `nvvideoconvert` 输入。  
  输出：  
  `video/x-raw(memory:NVMM), format=NV12`: NVIDIA NV12 格式。  
  `video/x-raw(memory:NVMM), format=RGBA`: NVIDIA RGBA 格式。  
  `video/x-raw(memory:NVMM), format=I420`: NVIDIA I420 格式。  
  `video/x-raw(memory:NVMM), format=YV12`: NVIDIA YV12 格式。  
  `video/x-raw(memory:NVMM), format=UYVY`: NVIDIA UYVY 格式。  
  `video/x-raw(memory:NVMM), format=BGRA`: NVIDIA BGRA 格式。  
  `video/x-raw(memory:NVMM), format=GRAY8`: NVIDIA GRAY8 格式。  
  `video/x-raw(memory:NVMM), format=GRAY16_LE`: NVIDIA GRAY16_LE 格式。  
  `video/x-raw, format=NV12`: YUV NV12 格式。  
  `video/x-raw, format=RGBA`: RGB RGBA 格式。  
  `video/x-raw, format=RGB`: RGB 格式。  
  `video/x-raw, format=BGR`: BGR 格式。  
  `video/x-raw, format=GRAY8`: GRAY8 格式。  
  `video/x-raw, format=GRAY16_LE`: GRAY16_LE 格式。  
  `video/x-raw, format=JPEG`: JPEG 格式。 
  本项目为 NV12 。  
- nvv4l2h264enc  
  作用：使用Nvidia显卡进行`H.264`编码。  
  `control-rate`: 控制编码比特率的方式。  
  `bitrate`: 指定编码的比特率。   
  输入：`video/x-raw(memory:NVMM), format=I420、NV12、RGBA或BGRA`，这些格式通常是由NVIDIA GPU硬件加速编解码器产生的。  
  输出：`video/x-h264`,向.h264格式转换是推流时常用的操作，它可以将原始视频数据压缩为较小的.h264格式，然后进行传输、存储等操作。  
- h264parse  
  作用：对编码后的`H.264`视频流进行解析。  
  输入：`H.264`视频流  
  输出：解析后的`H.264`视频流  
- flvmux  
  作用：将`H.264`视频流封装成`FLV`格式。  
  输入：解析后的`H.264`视频流  
  输出：`FLV`视频流  
- rtmpsink  
  作用：将`FLV`格式的视频流推送到`RTMP`服务器。  
  输入：`FLV`视频流  
  输出：无  

## **Q&A**

1. **报错` Internal data stream error`**  
   某个元素的输入数据格式不匹配，查阅官方文档找到适合的格式。





