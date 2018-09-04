/*
* Copyright (c) 2014,Ѹ��(����)��Ѷ�Ƽ����޹�˾
* All rights reserved.
*
* �ļ����ƣ�cv_iniparser.h
* ժ            Ҫ��iniģ��ͷ�ļ�
*
* ��ǰ�汾��1.0
* ��            �ߣ�dangqian
* ������ڣ�2014-05-14
*
*/

#ifndef _CV_INIPARSER_H_
#define _CV_INIPARSER_H_

#include <pthread.h>
#include "ty_type.h"


#define SPS_PPS 128
#define MAX_STREAM_NUM 32
#if 1
/*
*�������ܣ��ӿڳ�ʼ������
*����������߳̾��
*�����������
*����      ֵ��0:�ɹ�!0:ʧ��
*/
unsigned int cv_Ini_Init(char *ini_path);

void Ini_Save();

/*
*�������ܣ��ӿ�ȥ��ʼ������
*�����������
*�����������
*����ֵ��	0:�ɹ�!0:ʧ��
*/
void cv_Ini_Uninit(char *ini_path);
#endif

typedef struct
{
	unsigned int VideoSize;
	unsigned int VideoCach;
}IPCVideoBUF;
typedef struct
{
	unsigned int AudioSize;
	//unsigned int AudioCach;
}IPCVOtherBUF;

typedef struct
{
	unsigned int VideoType;
	unsigned int VideoBit;
	unsigned int resolution;
	unsigned int AudioEn;
	unsigned int AudioType;
	unsigned char sps_info[SPS_PPS];
	unsigned char pps_info[SPS_PPS];
	unsigned int profileID;
}IPCMeida;


typedef struct
{
	TY_VOID *dictionary;
	TY_CHAR  Ini_Path[256];
	pthread_mutex_t mutex;
}TY_INI_PARSER_CFG, *TY_INI_PARSER_HANDLE;

int TY_get_cam_name( TY_INI_PARSER_HANDLE Ini_handle, char *pcam_name);
int cv_get_cam_name(char *pcam_name);
void TY_ini_Save(TY_INI_PARSER_HANDLE Ini_Handle);
double TY_ini_GetDouble(TY_INI_PARSER_HANDLE Ini_Handle, const char *_pcSec, const char *_pcKey, unsigned int _iDef);
int TY_ini_GetInt(TY_INI_PARSER_HANDLE Ini_Handle, const char *_pcSec, const char *_pcKey, unsigned int _iDef);
int TY_ini_SetInt(TY_INI_PARSER_HANDLE Ini_Handle, const char *_pcSec, const char *_pcKey, unsigned int _iVal);
int TY_ini_SetString(TY_INI_PARSER_HANDLE Ini_Handle, const char *sec, const char *key, char *str);
int TY_ini_GetString(TY_INI_PARSER_HANDLE Ini_Handle, const char *sec, const char *key, char *str, char *def);
int TY_ini_init( TY_INI_PARSER_HANDLE Ini_Handle );
int TY_ini_Uninit(TY_INI_PARSER_HANDLE Ini_Handle );





#endif
