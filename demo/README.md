## 示例说明

本示例的主要作用就是通过EasyRTSPClient将外部的RTSP流取到EasyRTSPServer调用程序本地，再通过EasyRTSPServer的接口，将音视频数据推送进入EasyRTSPServer内部，再由EasyRTSPServer进行多路的分发：


> IPC --RTSP拉流--> 取得音视频AVFrame --> EasyRtspServer_PushFrame(AVFrame) --> RTSPServer分发



假设，本示例运行在IP：192.168.1.99上，EasyRtspServer_Startup监听的端口为554，示例启动后，比如我们要拉取并进行分发的IPC的RTSP地址是：rtsp://admin:12345@192.168.1.100/ch1，那我们只需要在VLC中请求输入：

> rtsp://192.168.1.99:554/rtsp://admin:12345@192.168.1.100/ch1

EasyRTSPServer就启动了对IPC的RTSP的实时分发，支持多次重复请求，一路进多路出！
	