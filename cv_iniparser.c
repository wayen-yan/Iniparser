/*
* Copyright (c) 2014,迅驰(北京)视讯科技有限公司
* All rights reserved.
*
* 文件名称：cv_iniparser.c
* 摘 要：ini文件读写模块
*
* 当前版本：1.0
* 作 者：dangq
* 完成日期：2014/05/20
*
*/

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#ifdef MEMWATCH
#include "memwatch.h"
#endif

#include "iniparser.h"
#include "cv_iniparser.h"
#include "system.h"
//#include "ty_type.h"


#define VERSION         "V2.1.1."  // 软件主版本
#define MAIN_PRM_PATH   "./main_control.ini"
#define CAM_CONTROL		"./MagCamCtrl.ini"

static dictionary *dic;     // main_control.ini
//static dictionary *dic_ex;  // protect.ini
static unsigned int iThreadFlag;
static unsigned int iModifyFlag;
static unsigned int iProtectFlag;

static pthread_mutex_t mutex;
//static pthread_mutex_t mutex_ex;

/****************************  内部接口*****************************/
/*
*函数介绍：保存主配置文件main_control.ini
*输入参数：无
*输出参数：无
*返回值：	0:成功!0:失败
*/
void Ini_Save(char *ini_path)
{
	if (1 != iModifyFlag)
	{
		return;
	}

	pthread_mutex_lock(&mutex);

	FILE *fp = NULL;
	fp = fopen(ini_path, "w");
	if (NULL != fp)
	{
		iniparser_dump_ini(dic, fp);
		fclose(fp);
		//system("sync");
		iModifyFlag = 0;
	}

	pthread_mutex_unlock(&mutex);
}

/*
*函数介绍：自动保存配置文件线程
*输入参数：无
*输出参数：无
*返回值：	0:成功!0:失败
*/
static void *Ini_AutoSavethread(void *p)
{
	char *ini_path = (char *)p;
	while (1 == iThreadFlag)
	{
		Sleep(10 * 1000);
		Ini_Save(ini_path);
	}

	return NULL;
}

static void Ini_CheckSection(const char *_pcSec, const char *_pcKey, const char *_pcVal)
{
	char cEntry[128] = { 0 };

	if (0 == iniparser_find_entry(dic, _pcSec))
	{
		_snprintf(cEntry, 128, "%s:%s", _pcSec, _pcKey);
		// section 不存在
		if (0 != dictionary_add(dic, (char *)_pcSec, NULL))
		{
			printf("%s(%d): Add section is error!\n", __FUNCTION__, __LINE__);
			return;
		}

		if (0 != dictionary_add(dic, (char *)cEntry, (char *)_pcVal))
		{
			printf("%s(%d): Add key and value is error!\n", __FUNCTION__, __LINE__);
			return;
		}
	}
}


/*
*函数介绍：获取double类型的参数
*输入参数：sec: section
				  key: key
				  _iDef:  默认值
*输出参数：返回获取的int型参数
*返回值：	0:成功!0:失败
*/
double ini_GetDouble(const char *_pcSec, const char *_pcKey, unsigned int _iDef)
{
	if (NULL == _pcSec || NULL == _pcKey)
	{
		return _iDef;
	}
	char cString[100] = { 0 };
	sprintf(cString, "%s:%s", _pcSec, _pcKey);
	
	return iniparser_getdouble(dic, cString, _iDef);
}

/*
*函数介绍：获取int类型的参数
*输入参数：sec: section
				  key: key
				  _iDef:  默认值
*输出参数：返回获取的int型参数
*返回值：	0:成功!0:失败
*/
int Ini_GetInt(const char *_pcSec, const char *_pcKey, unsigned int _iDef)
{
	if (NULL == _pcSec || NULL == _pcKey)
	{
		return _iDef;
	}

	char cString[100] = { 0 };

	sprintf(cString, "%s:%s", _pcSec, _pcKey);

	return iniparser_getint(dic, cString, _iDef);
}

/*
*函数介绍：设置int类型的参数
*输入参数：sec: section
				  key: key
				  _iVal:  需要设置的值
*输出参数：无
*返回值：	0:成功!0:失败
*/
int Ini_SetInt(const char *_pcSec, const char *_pcKey, unsigned int _iVal)
{
	if (NULL == _pcSec || NULL == _pcKey)
	{
		return -1;
	}

	char cEntry[100] = { 0 };
	char string[32] = { 0 };

	sprintf(cEntry, "%s:%s", _pcSec, _pcKey);
	sprintf(string, "%d", _iVal);
	//  cv_public_Itoa(_iVal, string, 16);
	Ini_CheckSection(_pcSec, _pcKey, string);

	if (0 != iniparser_setstr(dic, cEntry, string))
	{
		printf("iniparser_setstr error\n");
		return -1;
	}
	iModifyFlag = 1;
	return 0;
}

/*
*函数介绍：获取string类型的参数
*输入参数：sec: section
				  key: key
				  def: 默认要返回的值
*输出参数：str:返回string类型的值
*返回值：	0:成功!0:失败
*/
int Ini_GetString(const char *sec, const char *key, char *str, char *def)
{
	if ((NULL == sec) || (NULL == key) || (NULL == str))
	{
		return -1;
	}

	char *ret_str;
	char str_key[100] = { 0 };
	//char def[] = "none";

	sprintf(str_key, "%s:%s", sec, key);

	ret_str = iniparser_getstring(dic, str_key, def);
	if (ret_str == def)
	{
		return -1;
	}
	sprintf(str, "%s", ret_str);
	//iModifyFlag = 1;
	//dbgprintf(1, "iniparser get str success");
	return 0;
}

/*
*函数介绍：设置string类型的参数
*输入参数：sec: section
				  key: key
				  str: 字符参数
*输出参数：str:返回int类型的值
*返回值：	0:成功!0:失败
*/
int Ini_SetString(const char *sec, const char *key, char *str)
{
	if ((NULL == sec) || (NULL == key))
	{
		return -1;
	}

	Ini_CheckSection(sec, key, str);

	char cEntry[100] = { 0 };

	sprintf(cEntry, "%s:%s", sec, key);

	if (0 != iniparser_setstr(dic, cEntry, str))
	{
		return -1;
	}

	return 0;
}

void TY_ini_Save(TY_INI_PARSER_HANDLE Ini_Handle)
{
	if(Ini_Handle == NULL ){
		printf("TY_ini_Save error !\n");
		return TY_FAILURE;
	}

	pthread_mutex_lock(&Ini_Handle->mutex);
	

	FILE *fp = NULL;
	fp = fopen(Ini_Handle->Ini_Path, "w");
	if (NULL != fp)
	{
		iniparser_dump_ini(Ini_Handle->dictionary, fp);
		fclose(fp);
		//system("sync");
		iModifyFlag = 0;
	}

	pthread_mutex_unlock(&Ini_Handle->mutex);
}


static int TY_ini_CheckSection(dictionary *dic, const char *_pcSec, const char *_pcKey, const char *_pcVal)
{
	char cEntry[128] = { 0 };
	if(NULL == dic){
		return TY_FAILURE;
	}

	if (0 == iniparser_find_entry(dic, _pcSec))
	{
		_snprintf(cEntry, 128, "%s:%s", _pcSec, _pcKey);
		// section 不存在
		if (0 != dictionary_add(dic, (char *)_pcSec, NULL))
		{
			printf("%s(%d): Add section is error!\n", __FUNCTION__, __LINE__);
			return TY_FAILURE;
		}

		if (0 != dictionary_add(dic, (char *)cEntry, (char *)_pcVal))
		{
			printf("%s(%d): Add key and value is error!\n", __FUNCTION__, __LINE__);
			return TY_FAILURE;
		}
	}

	return TY_SUCCESS;
}


/*
*函数介绍：获取double类型的参数
*输入参数：sec: section
				  key: key
				  _iDef:  默认值
*输出参数：返回获取的int型参数
*返回值：	0:成功!0:失败
*/
double TY_ini_GetDouble(TY_INI_PARSER_HANDLE Ini_Handle, const char *_pcSec, const char *_pcKey, unsigned int _iDef)
{
	if ((NULL == Ini_Handle) || NULL == _pcSec || NULL == _pcKey)
	{
		return _iDef;
	}
	char cString[100] = { 0 };
	sprintf(cString, "%s:%s", _pcSec, _pcKey);
	
	return iniparser_getdouble(Ini_Handle->dictionary, cString, _iDef);
}


/*
*函数介绍：获取int类型的参数
*输入参数：sec: section
				  key: key
				  _iDef:  默认值
*输出参数：返回获取的int型参数
*返回值：	0:成功!0:失败
*/
int TY_ini_GetInt(TY_INI_PARSER_HANDLE Ini_Handle, const char *_pcSec, const char *_pcKey, unsigned int _iDef)
{
	if ((NULL == Ini_Handle) || NULL == _pcSec || NULL == _pcKey)
	{
		return _iDef;
	}

	char cString[100] = { 0 };

	sprintf(cString, "%s:%s", _pcSec, _pcKey);

	return iniparser_getint(Ini_Handle->dictionary, cString, _iDef);
}


/*
*函数介绍：设置int类型的参数
*输入参数：sec: section
				  key: key
				  _iVal:  需要设置的值
*输出参数：无
*返回值：	0:成功!0:失败
*/
int TY_ini_SetInt(TY_INI_PARSER_HANDLE Ini_Handle, const char *_pcSec, const char *_pcKey, unsigned int _iVal)
{
	if ((NULL == Ini_Handle) || NULL == _pcSec || NULL == _pcKey)
	{
		return -1;
	}

	char cEntry[100] = { 0 };
	char string[32] = { 0 };

	sprintf(cEntry, "%s:%s", _pcSec, _pcKey);
	sprintf(string, "%d", _iVal);
	//  cv_public_Itoa(_iVal, string, 16);
	TY_ini_CheckSection( Ini_Handle->dictionary, _pcSec, _pcKey, string);

	if (0 != iniparser_setstr(Ini_Handle->dictionary, cEntry, string))
	{
		printf("iniparser_setstr error\n");
		return -1;
	}

	return 0;
}


int TY_ini_SetString(TY_INI_PARSER_HANDLE Ini_Handle, const char *sec, const char *key, char *str)
{
	if ((NULL == Ini_Handle) || (NULL == sec) || (NULL == key))
	{
		return TY_FAILURE;
	}

	int ret = TY_ini_CheckSection(Ini_Handle->dictionary, sec, key, str);
	if(ret != TY_SUCCESS){
		return TY_FAILURE;
	}

	char cEntry[100] = { 0 };

	sprintf(cEntry, "%s:%s", sec, key);

	if (0 != iniparser_setstr(Ini_Handle->dictionary, cEntry, str))
	{
		return TY_FAILURE;
	}

	return TY_SUCCESS;
	
}

/*
*函数介绍：获取string类型的参数
*输入参数：sec: section
				  key: key
				  def: 默认要返回的值
*输出参数：str:返回string类型的值
*返回值：	0:成功!0:失败
*/
int TY_ini_GetString(TY_INI_PARSER_HANDLE Ini_Handle, const char *sec, const char *key, char *str, char *def)
{
	if ((NULL == Ini_Handle) || (NULL == sec) || (NULL == key) || (NULL == str))
	{
		return -1;
	}

	char *ret_str;
	char str_key[100] = { 0 };
	//char def[] = "none";

	sprintf(str_key, "%s:%s", sec, key);

	ret_str = iniparser_getstring(Ini_Handle->dictionary, str_key, def);
	if (ret_str == def)
	{
		return -1;
	}
	sprintf(str, "%s", ret_str);
	//iModifyFlag = 1;
	//dbgprintf(1, "iniparser get str success");
	return 0;
}


int TY_ini_init( TY_INI_PARSER_HANDLE Ini_Handle )
{
	if(Ini_Handle == NULL ){
		printf("TY_ini_init error !\n");
		return TY_FAILURE;
	}

	if (pthread_mutex_init(&Ini_Handle->mutex, NULL) != 0)
	{
		printf("log mutex init error %d", errno);
		return TY_FAILURE;
	}
	
	// 检查配置文件是否存在
	if (0 != Ini_CheckFile(Ini_Handle->Ini_Path))
	{
		printf("main_control.ini not exit!\n");
		return TY_FAILURE;
	}

	dictionary *dic;
	
	dic = iniparser_load(Ini_Handle->Ini_Path);
	if (NULL == dic)
	{
		printf("load main_control.ini error!\n");
		return TY_FAILURE;
	}

	// 判断配置文件是否完整
	if (iniparser_find_entry(dic, "end") != 1)
	{
		// 释放之前打开的配置文件指针
		iniparser_freedict(dic);
		dic = iniparser_load(Ini_Handle->Ini_Path);
		if (NULL == dic)
		{
			printf("load main_control.ini error!\n");
			return TY_FAILURE;
		}

		printf("%s %d\n", __FUNCTION__, __LINE__);
	}

	Ini_Handle->dictionary = (TY_VOID *)dic;

	return TY_SUCCESS;
	
}


int TY_ini_Uninit(TY_INI_PARSER_HANDLE Ini_Handle )
{
	if(Ini_Handle == NULL || NULL == Ini_Handle->dictionary){
		printf("TY_ini_Uninit error !\n");
		return TY_FAILURE;
	}
	
	iniparser_freedict((dictionary *)Ini_Handle->dictionary);

	return TY_SUCCESS;
}

unsigned int Ini_CheckFile(char *_pcFile)
{
	if (NULL == _pcFile)
	{
		return -1;
	}

	if (0 != access(_pcFile, 0))
	{
		if (0 != access(_pcFile, 0))
		{
			return -1;
		}
	}

	return 0;
}

unsigned int Ini_Init(char *ini_path)
{
	pthread_t th_ini;

	if (pthread_mutex_init(&mutex, NULL) != 0)
	{
		printf("log mutex init error %d", errno);
		return -1;
	}

	// 检查配置文件是否存在
	if (0 != Ini_CheckFile(ini_path))
	{
		printf("main_control.ini not exit!\n");
		return -1;
	}

	dic = iniparser_load(ini_path);
	if (NULL == dic)
	{
		printf("load main_control.ini error!\n");
		return -1;
	}

	// 判断配置文件是否完整
	if (iniparser_find_entry(dic, "end") != 1)
	{
		// 释放之前打开的配置文件指针
		iniparser_freedict(dic);
		dic = iniparser_load(ini_path);
		if (NULL == dic)
		{
			printf("load main_control.ini error!\n");
			return -1;
		}

		printf("%s %d\n", __FUNCTION__, __LINE__);
	}

	iThreadFlag = 1;
	if (0 != pthread_create(&th_ini, NULL, Ini_AutoSavethread, (void *)ini_path))
	{
		printf("create ini thread fail!\n");
		return -1;
	}
	pthread_detach(th_ini);

	return 0;
}


/****************************  外部调用接口*****************************/
unsigned int cv_Ini_Init(char *ini_path)
{
	// 初始化main_control.ini
	if (0 != Ini_Init(ini_path))
	{
		printf("init main_control.ini error!\n");
		//cv_log_write("Init main_control.ini failure!");
		return -1;
	}

	return 0;
}

void cv_Ini_Uninit(char *ini_path)
{
	Ini_Save(ini_path);

	iniparser_freedict(dic);
	dic = NULL;

	iThreadFlag = 0;
}

int TY_get_cam_name( TY_INI_PARSER_HANDLE Ini_handle, char *pcam_name)
{
	if(NULL == Ini_handle || NULL == pcam_name){
		return TY_FAILURE;
	}
	
	int ret = TY_ini_init(Ini_handle);
	if(ret != TY_SUCCESS){
		printf("TY_ini_init error !\n");
		return TY_FAILURE;
	}

	TY_ini_GetString(Ini_handle, "cam_type", "ty_cam_type", pcam_name, "TY-CM-8225A2-V1");

	return TY_SUCCESS;
}

int cv_get_cam_name(char *pcam_name)
{
	//初始化MagCamCtrl.ini
	// 检查配置文件是否存在
	if (0 != Ini_CheckFile(CAM_CONTROL))
	{
		printf("MagCamCtrl.ini not exit!\n");
		return -1;
	}

	dictionary *dic = iniparser_load(CAM_CONTROL);
	if (NULL == dic)
	{
		printf("load MagCamCtrl.ini error!\n");
		return -1;
	};

	// 判断配置文件是否完整
	if (iniparser_find_entry(dic, "end") != 1)
	{
		// 释放之前打开的配置文件指针
		iniparser_freedict(dic);
		dic = iniparser_load(CAM_CONTROL);
		if (NULL == dic)
		{
			printf("load MagCamCtrl.ini error!\n");
			return -1;
		}

		printf("%s %d\n", __FUNCTION__, __LINE__);
	}
	
	char str_key[100] = { 0 };
	sprintf(str_key, "%s:%s", "cam_type", "ty_cam_type");
	char *ret_str = iniparser_getstring(dic, str_key, NULL);
	if (ret_str == NULL)
	{
		return -1;
	}
	sprintf(pcam_name, "%s", ret_str);	
	iniparser_freedict(dic);
	
	printf("pcam_name === %s\n", pcam_name);
	return 0;

}

int cv_ini_Get_RTSP_PORT()
{
	return Ini_GetInt("RTSP", "ServerPort", 0);
}
int cv_ini_Set_RTSP_PORT(int pPort)
{
	return Ini_SetInt("RTSP", "ServerPort", pPort);
}

int cv_ini_Get_Stream_Num()
{
	return Ini_GetInt("RTSP", "StreamNum", 0);
}

int cv_ini_Set_Stream_Num(int pNum)
{
	return Ini_SetInt("RTSP", "StreamNum", pNum);
}

int cv_ini_Get_IPC_Video_BUF(int BufId, IPCVideoBUF *ipcVideoBuf)
{
	char VideoSizeName[16] = { 0 };
	char VideoCachName[16] = { 0 };
	if (ipcVideoBuf == NULL)
		return -1;
	sprintf(VideoSizeName, "Video[%d]Size", BufId);
	sprintf(VideoCachName, "Video[%d]Cach", BufId);

	if ((ipcVideoBuf->VideoSize = Ini_GetInt("RTSP", VideoSizeName, -1)) == -1)
	{
		return -1;
	}

	if ((ipcVideoBuf->VideoCach = Ini_GetInt("RTSP", VideoCachName, -1)) == -1)
	{
		return -1;
	}

	return 0;
}
int cv_ini_Set_IPC_Video_BUF(int BufId, IPCVideoBUF ipcVideoBuf)
{
	char VideoSizeName[16] = { 0 };
	char VideoCachName[16] = { 0 };

	sprintf(VideoSizeName, "Video[%d]Size", BufId);
	sprintf(VideoCachName, "Video[%d]Cach", BufId);
	Ini_SetInt("RTSP", VideoSizeName, ipcVideoBuf.VideoSize);
	Ini_SetInt("RTSP", VideoCachName, ipcVideoBuf.VideoCach);
	return 0;
}

int cv_ini_Get_IPC_Other_BUF(IPCVOtherBUF *ipcotherBuf)
{
	if (ipcotherBuf == NULL)
		return -1;
	if ((ipcotherBuf->AudioSize = Ini_GetInt("RTSP", "AudioSize", -1)) == -1)
	{
		return -1;
	}
	return 0;
}
int cv_ini_Set_IPC_Other_BUF(IPCVOtherBUF ipcotherBuf)
{
	Ini_SetInt("RTSP", "AudioSize", ipcotherBuf.AudioSize);
	return 0;
}

int cv_ini_Get_IPC_Meida(int BufId, IPCMeida *ipcMedia)
{
	char VideoTypeName[24] = { 0 };
	char VideoBitName[24] = { 0 };
	char resolutionName[24] = { 0 };
	char AudioEnName[24] = { 0 };
	char AudioTypeName[24] = { 0 };
	char sps_infoName[24] = { 0 };
	char pps_infoName[24] = { 0 };
	char profileIDName[24] = { 0 };
	if (ipcMedia == NULL)
		return -1;

	sprintf(VideoTypeName, "Video[%d]Type", BufId);
	sprintf(VideoBitName, "Video[%d]Bit", BufId);
	sprintf(resolutionName, "resolution[%d]", BufId);
	sprintf(AudioEnName, "Audio[%d]En", BufId);
	sprintf(AudioTypeName, "Audio[%d]Type", BufId);
	sprintf(sps_infoName, "sps_info[%d]", BufId);
	sprintf(pps_infoName, "pps_info[%d]", BufId);
	sprintf(profileIDName, "profileID[%d]", BufId);

	if ((ipcMedia->VideoType = Ini_GetInt("RTSP", VideoTypeName, -1)) == -1)
	{
		return -1;
	}

	if ((ipcMedia->VideoBit = Ini_GetInt("RTSP", VideoBitName, -1)) == -1)
	{
		return -1;
	}

	if ((ipcMedia->resolution = Ini_GetInt("RTSP", resolutionName, -1)) == -1)
	{
		return -1;
	}

	if ((ipcMedia->AudioType = Ini_GetInt("RTSP", AudioTypeName, -1)) == -1)
	{
		return -1;
	}

	if (Ini_GetString("RTSP", sps_infoName, ipcMedia->sps_info, NULL) != 0)
	{
		return -1;
	}

	if (Ini_GetString("RTSP", pps_infoName, ipcMedia->pps_info, NULL) != 0)
	{
		return -1;
	}

	if ((ipcMedia->profileID = Ini_GetInt("RTSP", profileIDName, -1)) == -1)
	{
		return -1;
	}
	return 0;
}
int cv_ini_Set_IPC_Meida(int BufId, IPCMeida ipcMedia)
{
	char VideoTypeName[24] = { 0 };
	char VideoBitName[24] = { 0 };
	char resolutionName[24] = { 0 };
	char AudioEnName[24] = { 0 };
	char AudioTypeName[24] = { 0 };
	char sps_infoName[24] = { 0 };
	char pps_infoName[24] = { 0 };
	char profileIDName[24] = { 0 };

	sprintf(VideoTypeName, "Video[%d]Type", BufId);
	sprintf(VideoBitName, "Video[%d]Bit", BufId);
	sprintf(resolutionName, "resolution[%d]", BufId);
	sprintf(AudioEnName, "Audio[%d]En", BufId);
	sprintf(AudioTypeName, "Audio[%d]Type", BufId);
	sprintf(sps_infoName, "sps_info[%d]", BufId);
	sprintf(pps_infoName, "pps_info[%d]", BufId);
	sprintf(profileIDName, "profileID[%d]", BufId);
	Ini_SetInt("RTSP", VideoTypeName, ipcMedia.VideoType);
	Ini_SetInt("RTSP", VideoBitName, ipcMedia.VideoBit);
	Ini_SetInt("RTSP", resolutionName, ipcMedia.resolution);
	Ini_SetInt("RTSP", AudioEnName, ipcMedia.AudioEn);
	Ini_SetInt("RTSP", AudioTypeName, ipcMedia.AudioType);
	Ini_SetString("RTSP", sps_infoName, ipcMedia.sps_info);
	Ini_SetString("RTSP", pps_infoName, ipcMedia.pps_info);
	Ini_SetInt("RTSP", profileIDName, ipcMedia.profileID);
}
int cv_ini_get_Stream_ID(char *object)
{
	char streamName[32] = { 0 };
	char TmpName[128] = { 0 };
	int i = 0;
	int ret = 0;

	if (object == NULL)
		return -1;

	for (i = 0; i < MAX_STREAM_NUM; i++)
	{
		memset(streamName, 0x0, sizeof(streamName));
		sprintf(streamName, "STREAM[%d]Name", i);
		memset(TmpName, 0x0, sizeof(TmpName));
		ret = Ini_GetString("RTSP", streamName, TmpName, NULL);
		//printf("ret:%d %s,%s,%s\n",ret,streamName,TmpName,object);
		if (!_stricmp(TmpName, object))
		{
			break;
		}
	}
	if (i >= MAX_STREAM_NUM)
		return -1;
	return i;
}
int cv_ini_Set_Stream_ID(int bufId, char *object)
{
	char streamName[32] = { 0 };
	char TmpName[128] = { 0 };
	if (object == NULL)
		return -1;
	if (bufId >= MAX_STREAM_NUM)
		return -1;
	sprintf(streamName, "STREAM[%d]Name", bufId);
	if (Ini_SetString("RTSP", streamName, object) != 0)
		return -1;

	return 0;
}

int cv_ini_Get_MeidaFileInfo(int bufId, char *VfileName, int *mediaType)
{
	char StreamFileName[128] = { 0 };
	char StreamMediaType[128] = { 0 };
	sprintf(StreamFileName, "Stream[%d]FileName", bufId);
	sprintf(StreamMediaType, "Video[%d]Type", bufId);

	if (Ini_GetString("RTSP", StreamFileName, VfileName, NULL) != 0)
	{
		return -1;
	}

	if ((*mediaType = Ini_GetInt("RTSP", StreamMediaType, -1)) == -1)
	{
		return -1;
	}
	return 0;
}

int cv_ini_Set_MeidaFileInfo(int bufId, char *VfileName)
{
	char StreamFileName[128] = { 0 };
	sprintf(StreamFileName, "Stream[%d]FileName", bufId);
	if (Ini_SetString("RTSP", StreamFileName, VfileName) != 0)
		return -1;
	return 0;
}
