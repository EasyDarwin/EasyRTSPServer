# EasyRTSPServer

EasyRTSPServer是由[**EasyDarwin开源社区**](https://www.easydarwin.org "EasyDarwin")开发的一套非常稳定、易用、支持多种平台（包括Windows/Linux 32&64，Android，ARM hisiv100/hisiv200/hisiv300/hisiv400/hisiv500/hisiv600等平台）的RTSP Server组件，适用于IPCamera、NVR、编码器、安卓监控设备等软硬件产品，接口调用非常简单成熟，调用者无需关注RTSP Server中关于客户端监听接入、音视频多路复用、RTSP具体流程、RTP打包与发送等相关问题，支持多种音视频格式，再也不用像调用live555 RTSPServer那样处理整个RTSP OPTIONS/DESCRIBE/SETUP/PLAY/RTP/RTCP的复杂流程和担心内存释放的问题了！EasyIPCamera非常适合于安防领域、教育领域、互联网直播等领域；

BTW：EasyRTSPServer非常适合在海思系列芯片上运行，性能以及稳定性都非常优秀，并发方面，稳定保持在20路1080P并发：

 - TCP/UDP 方式分别连接20路下，1080P 4M 定码率，音频格式G711（64K）G726（16K 24K 32K 40K）AAC(64K 96K 128K)都没问题；
 
 - 支持Basic、Digest两种鉴权模式；

## 功能支持 ##

- [x] 标准、稳定运行的RTSP/RTP服务；
- [x] 支持RTP over UDP/RTP over TCP；
- [x] 视频编码格式支持：H.264、H.265；
- [x] 音频编码格式支持：G.711A、G.711U、G.726、AAC；
- [x] 支持标准RTSP鉴权认证;
- [x] 灵活的调用接口支持;
- [x] 丰富的接口调用示例;


## 设计方法 ##

EasyRTSPServer参考live555 testProg中的testOnDemandRTSPServer示例程序，将一个live555 testOnDemandRTSPServer封装在一个类中，例如，我们称为Class EasyRTSPServer，在EasyRTSPServer_Create接口调用时，我们新建一个EasyRTSPServer对象，再通过调用EasyRTSPServer_Startup接口，将EasyRTSPServer RTSPServer所需要的监听端口、认证信息、通道信息等参数输入到EasyRTSPServer中后，EasyRTSPServer就正式开始建立监听对外服务了，在服务的过程中，当有客户端的连接或断开，都会以回调事件的形式，通知给Controller调用者，调用者再具体来处理相关的回调任务，返回给EasyRTSPServer，在EasyRTSPServer服务的过程当中，如果回调要求需要Controller调用者提供音视频数据帧，Controller调用者可以通过EasyRTSPServer_PushFrame接口，向EasyRTSPServer输送具体的音视频帧数据，当调用者需要结束RTSPServer服务，只需要调用EasyRTSPServer_Shutdown停止服务，再调用EasyRTSPServer_Release释放EasyRTSPServer就可以了，这样整个服务过程就完整了！


### EasyRTSPServer支持数据格式说明 ###

EASY\_SDK\_VIDEO\_FRAME\_FLAG数据可支持多种视频格式：
		
	#define EASY_SDK_VIDEO_CODEC_H265			/* H265  */
	#define EASY_SDK_VIDEO_CODEC_H264			/* H264  */
	#define	EASY_SDK_VIDEO_CODEC_MJPEG			/* MJPEG */
	#define	EASY_SDK_VIDEO_CODEC_MPEG4			/* MPEG4 */

视频帧标识支持

	#define EASY_SDK_VIDEO_FRAME_I				/* I帧 */
	#define EASY_SDK_VIDEO_FRAME_P				/* P帧 */
	#define EASY_SDK_VIDEO_FRAME_B				/* B帧 */
	#define EASY_SDK_VIDEO_FRAME_J				/* JPEG */

EASY\_SDK\_AUDIO\_FRAME\_FLAG数据可支持多种音频格式：
	
	#define EASY_SDK_AUDIO_CODEC_AAC			/* AAC */
	#define EASY_SDK_AUDIO_CODEC_G711A			/* G711 alaw*/
	#define EASY_SDK_AUDIO_CODEC_G711U			/* G711 ulaw*/
	#define EASY_SDK_AUDIO_CODEC_G726			/* G726 */



## 目录说明

`./src/`目录主要为EasyRTSPServer核心源代码，通过此目录可以定向编译出各个不同平台的libEasyRTSPServer SDK，然后再被主程序调用；

`./demo/`目录中有EasyRTSPServer的调用示例，其主要功能是通过RTSP拉流的方式将外部实时流输入给EasyRTSPServer，进行多路的RTSP流分发输出，作用等同于NVR的作用；



## 更多流媒体音视频资源

EasyDarwin开源流媒体服务器：<a href="https://www.easydarwin.org" target="_blank" title="EasyDarwin开源流媒体服务器">www.EasyDarwin.org</a>