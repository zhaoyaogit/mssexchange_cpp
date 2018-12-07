/*
2018-12-07:
金巴特交易所论坛地址： http://kingstargold.cn:8080/goldblog/c/%E9%87%91%E5%B7%B4%E7%89%B9%E4%B8%AD%E5%BF%83%E5%8C%96%E4%BA%A4%E6%98%93%E6%89%80
金巴特交易所作者： 赵耀
金巴特交易所手机： 15216631375
金巴特交易所qq交流群号： 959372362
金巴特交易所作者邮箱： myzhaoyao@126.com
*/

//////////////////////////////////////////////////////////////////////

#include "msslog.h"
#include "stdio.h"
//#include "string.h"
#include "stdlib.h"
#include "time.h"
#include <string>
#ifndef WIN32
#include <arpa/inet.h>
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
using namespace std;
CMssLog::CMssLog()
{
	memset(m_filepath,0,sizeof(m_filepath));
	memset(m_file,0,sizeof(m_file));
	//sprintf(m_filepath,"%s/log/",(char *)getenv("HOME"));
#ifndef WIN32
	sprintf(m_filepath,"./");
#else
	sprintf(m_filepath, ".");
#endif
	
	
}

CMssLog::~CMssLog()
{

}

//函数名：Trace
//参数1 ：obj： 日志文件名中的关键字20150607bankbu.log 中的bankbu
//参数2 ：file：日志所在代码文件的文件名
//参数3 ：line：日志所在代码文件的行数
//参数4 ：code：整型输出码
//参数5 ：szcmf：变参、
void CMssLog::Trace(const char *obj,const char *file,int line,int buflen,const char* szcmf,...)
{

		va_list argList;
		if (szcmf == NULL || szcmf[0] == '\0') 
		{
			return;
		}
		va_start(argList, szcmf);

        char szTmp[4096] = {0};
#ifndef WIN32
			vsnprintf(szTmp,4095,szcmf,argList);
#else
			vsprintf(szTmp,szcmf,argList);
#endif
        fprintf(stderr, "%s\n", szTmp);	
        //写日志
		FILE *fp;
		char errfile[280];
		char date[16]={0};
		char time[64]={0};
		memset(date,0,sizeof(date));
		memset(time,0,sizeof(time));
		Today(date);
#ifdef WIN32
		if (strlen(m_filepath) >0)
		{
			sprintf(errfile, "%s\\%s%s.log", m_filepath, date, obj);
		}
		else
		{
			sprintf(errfile,"log\\%s%s.log",date,obj);
		}
#else
		sprintf(errfile,"%s%s%s.log",m_filepath,date,obj);	
#endif
		GetTime(time);
		fp=fopen(errfile,"a");
		if(fp==NULL)
		{
			return;
		}
		fprintf(fp,"%s %18s line=[%8d] int=[%4d]log=[%s]\n",time,file,line,buflen,szTmp);
		fflush(fp);
		fclose(fp);
		va_end(argList);
}


//函数名：Trace
//参数 ：dst： 将日志作为返回信息写入传出参数dst中，此时要求dstLen长度为传入的dst内存的buf大小，以便再次初始化
//参数 ：obj： 日志文件名中的关键字20150607bankbu.log 中的bankbu
//参数 ：file：日志所在代码文件的文件名
//参数 ：line：日志所在代码文件的行数
//参数 ：dstLen：dst内存的大小
//参数 ：szcmf：变参、
//20181206:该接口已经没有存在意义了，以为报错信息都会写入json对象里，json对象会串联整个交易流程
void CMssLog::Trace(char *dst,const char *obj,const char *file,int line,int dstLen,const char* szcmf,...)
{
		memset(dst,0,dstLen);
		va_list argList;
		if (szcmf == NULL || szcmf[0] == '\0') 
		{
			return;
		}
		va_start(argList, szcmf);

        char szTmp[8192] = {0};
#ifndef WIN32
			vsnprintf(szTmp,8191,szcmf,argList);
#else
			vsprintf(szTmp,szcmf,argList);
#endif
        fprintf(stderr, "%s\n", szTmp);	
		strncpy(dst,szTmp,strlen(szTmp));
        //写日志
		FILE *fp;
		char errfile[280] = {0};
		char date[16]={0};
		char time[64]={0};
		memset(date,0,sizeof(date));
		memset(time,0,sizeof(time));
		Today(date);
#ifdef WIN32
		if (strlen(m_filepath) >0)
		{
			sprintf(errfile, "%s\\%s%s.log", m_filepath, date, obj);
		}
		else
		{
			sprintf(errfile,"log\\%s%s.log",date,obj);
		}
#else
		sprintf(errfile,"%s%s%s.log",m_filepath,date,obj);	
#endif
		GetTime(time);
		fp=fopen(errfile,"a");
		if(fp==NULL)
		{
			return;
		}
#ifndef WIN32
		fprintf(fp,"%s %18s line=[%8d] int=[%4lu] log=[%s]\n",time,file,line,strlen(szTmp),szTmp);
#else
		fprintf(fp, "%s %18s line=[%8d] int=[%4lu] log=[%s]\n", time, file, line, strlen(szTmp), szTmp);
#endif
		fclose(fp);
		va_end(argList);
}

//YYYY年 MM月 DD日 HH时 MM分 SS秒 MS毫秒
void CMssLog::GetTime(char* time)
{
    struct timeb tb_time;
    ftime(&tb_time);

	string sRet;
	char pBuff[30];
	tm* pCurTime;
	/*
	time_t t;
	time(&t);
	pCurTime = localtime(&t);
	*/
	pCurTime = localtime(&tb_time.time);
//获取年份
		sprintf(pBuff, "%4d", pCurTime->tm_year + 1900);
		sRet += pBuff;
		sRet += "-";
//获取月份
		sprintf(pBuff, "%02d", pCurTime->tm_mon + 1);
		sRet += pBuff;
		sRet += "-";
//获取日
		sprintf(pBuff, "%02d", pCurTime->tm_mday);
		sRet += pBuff;
		sRet += " ";
//获取小时
		sprintf(pBuff, "%02d", pCurTime->tm_hour);
		sRet += pBuff;
		sRet += ":";
//获取分钟
		sprintf(pBuff, "%02d", pCurTime->tm_min);
		sRet += pBuff;
		sRet += ":";
//获取秒数
		sprintf(pBuff, "%02d", pCurTime->tm_sec);
		sRet += pBuff;
		sRet += ".";
//获取毫秒
		sprintf(pBuff, "%03d", tb_time.millitm);
		sRet += pBuff;
		strcpy(time,sRet.c_str());
	return;
}


void CMssLog::Today(char *date)
{
	
	time_t t;
	time(&t);
#ifndef _AIX32_THREADS
	struct tm *tt;
	tt=localtime(&t);
	sprintf(date,"%4d%02d%02d",\
		1900+tt->tm_year,tt->tm_mon+1,tt->tm_mday);
#else
	struct tm tt;
	localtime_r(&t,&tt);
	sprintf(date,"%4d%02d%02d",\
		1900+tt.tm_year,tt.tm_mon+1,tt.tm_mday);
#endif
	//printf("测试%s\n",date);
	return;
}

unsigned char  CMssLog::AscToHex(unsigned char hex)
{
	if((hex>=0)&&(hex<=9))
		{
		hex += 0x30;
	}
	else if ((hex >=10)&&(hex<=15))
		{
		hex += 0x37;
		}
	else
		{
			hex = 0xff;
		}
	return hex;
}