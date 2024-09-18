
#IPC RTSP模拟器（RTSP Simulator）运行说明

##Win
运行EasyRTSPServer_Demo.exe


##Linux
运行easyrtspserver_demo


#RTSP拉流
##rtsp源地址为摄像机或NVR的rtsp地址. 比如rtsp://192.168.1.100/ch1
	rtsp://192.168.1.160:554/rtsp://192.168.1.100/ch1
##rtsp源需带用户名密码的地址如下(用户名: admin  密码:12345):
	rtsp://192.168.1.160:554/rtsp://admin:12345@192.168.1.100/ch1
##文件视频源是easy.mp4:
	rtsp://192.168.1.160:554/easy.mp4

##说明
示例代码中支持H264/H265;  
用户验证设置为不验证;