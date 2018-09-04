#include "stdio.h"
#include "cv_iniparser.h"
#include "string.h"
#include "time.h"

int main()
{
	int ret = 0;
	char tmpchar[128] = { 0 };
	char ini_path[64] = "./main_control.ini";
	cv_Ini_Init(ini_path);
	//cv_ini_get_Stream_ID("MainStream");

	ret = Ini_GetString("RTSP", "stream[0]name", tmpchar, NULL);
	printf("ret:%d tmp[%s]\n", ret, tmpchar);
	return 0;

	printf("function[%s] LINE[%d]\n", __FUNCTION__, __LINE__);
	//return 0;

	printf("function[%s] LINE[%d] PORT[%d]\n", __FUNCTION__, __LINE__, cv_ini_Get_RTSP_PORT());
	ret = cv_ini_Set_RTSP_PORT(554);
	printf("function[%s] LINE[%d] ret[%d]\n", __FUNCTION__, __LINE__, ret);
	ret = cv_ini_Set_Stream_Num(2);
	printf("function[%s] LINE[%d] ret[%d]\n", __FUNCTION__, __LINE__, ret);
	IPCVideoBUF ipcVideoBuf;
	ipcVideoBuf.VideoSize = 1024 * 1024 * 4;
	ipcVideoBuf.VideoCach = 1024 * 1024;
	ret = cv_ini_Set_IPC_Video_BUF(0, ipcVideoBuf);
	printf("function[%s] LINE[%d] ret[%d]\n", __FUNCTION__, __LINE__, ret);
	ipcVideoBuf.VideoSize = 1024 * 1024 * 2;
	ipcVideoBuf.VideoCach = 512 * 1024;
	ret = cv_ini_Set_IPC_Video_BUF(1, ipcVideoBuf);
	printf("function[%s] LINE[%d] ret[%d]\n", __FUNCTION__, __LINE__, ret);

	IPCVOtherBUF ipcotherBuf;
	ipcotherBuf.AudioSize = 512 * 1024;
	ret = cv_ini_Set_IPC_Other_BUF(ipcotherBuf);
	printf("function[%s] LINE[%d] ret[%d]\n", __FUNCTION__, __LINE__, ret);

	IPCMeida ipcMedia;
	memset(&ipcMedia, 0x0, sizeof(IPCMeida));
	//
	ret = cv_ini_Set_IPC_Meida(0, ipcMedia);

	printf("function[%s] LINE[%d] ret[%d]\n", __FUNCTION__, __LINE__, ret);
	memset(&ipcMedia, 0x0, sizeof(IPCMeida));
	ret = cv_ini_Set_IPC_Meida(1, ipcMedia);

	printf("function[%s] LINE[%d] ret[%d]\n", __FUNCTION__, __LINE__, ret);
	cv_ini_Set_Stream_ID(0, "MainStream");
	cv_ini_Set_Stream_ID(1, "SubStream1");
	Ini_Save();
	//sleep(1);
}