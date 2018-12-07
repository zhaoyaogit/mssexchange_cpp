/*
2018-12-07:
金巴特交易所论坛地址： http://kingstargold.cn:8080/goldblog/c/%E9%87%91%E5%B7%B4%E7%89%B9%E4%B8%AD%E5%BF%83%E5%8C%96%E4%BA%A4%E6%98%93%E6%89%80
金巴特交易所作者： 赵耀
金巴特交易所手机： 15216631375
金巴特交易所qq交流群号： 959372362
金巴特交易所作者邮箱： myzhaoyao@126.com
*/
#include <iostream>
#include "hiredis.h"
#include <string.h>
#ifndef WIN32
#include <unistd.h>
#else
#include <windows.h>
#endif

#include "msslog.h"
#include "jsonpack.h"

using namespace std;





void RedisSync(char *buf);
void RedisSendList(char *buf);

//CMssLog g_logfile;
int main()
{

	//RedisSync(buf);
	RedisSendList(NULL);
	return 0;
}

//redis订阅发布,同步发布接口
void RedisSync(char *buf)
{
	redisContext* rc = redisConnect("140.143.244.108", 9998);//同redis服务建立连接
	if (rc == NULL || rc->err)
	{
		printf("redis connect error\n");
		return;
	}
	redisReply *reply = NULL;
//	reply = (redisReply *)redisCommand(rc, "AUTH 123456");//设置同redis建立好的连接的认证密码
//	freeReplyObject(reply);
	int count = 1;
	reply = NULL;
	while (1)
	{
		reply = (redisReply *)redisCommand(rc, "PUBLISH %s %d", "quotation", count);//订阅quotation通道，标识为：quotation，count为发布的内容可以替换为行情报文
#ifdef WIN32
		Sleep(3000);
#else
		sleep(3);
#endif // WIN32
		
		printf("发布:%d次", count);
		count++;
		freeReplyObject(reply);
	}
	
	reply = NULL;
	redisFree(rc);
}

void RedisSendList(char *req)
{
	//从文件中读取请求报文
	FILE *fp;
	CJsonPack readJson;
	fp = fopen("entrust.req","r");
	if (fp == NULL)
	{
		printf("fopen entrust.req error\n");
		return;
	}
	char buf[8192] = {0};
	
	redisContext* rc = redisConnect("140.143.244.108", 9998);//同redis服务建立连接
	if (rc == NULL || rc->err)
	{
		printf("redis connect error\n");
		return;
	}
	redisReply *reply = NULL;
	int count = 1;
	reply = NULL;
	char temp[32] = {0};
	while (1)
	{
		memset(buf,0,sizeof(buf));
		if (fgets(buf,8192,fp) == NULL)   //读取mssjson格式委托请求
		{
			printf("文件读取结束,程序退出\n");
			break;
		}
		bool flag = false;
		readJson.Clear();
		flag = readJson.Parse(buf);
		if (flag == false)
		{
			//g_logfile.Trace("mssexchange",__FILE__,__LINE__,0,"json解析失败,请求数据[%s]",buf);
			return ;
		}
		//对委托数据进行处理，以保证能成交来验证撮合功能
		memset(temp,0,sizeof(temp));
		readJson["body"].Get("CustNo",temp);
		sprintf(temp,"%8d",count);
		readJson["body"].Replace("EntrustNo",temp);
		//发送请求
		reply = (redisReply *)redisCommand(rc, "lpush autd %s", readJson.ToString().c_str());//向list列表中插入委托请求报文
		
		//sleep(3);
		printf("请求:%d次\n", count);
		count++;
		freeReplyObject(reply);
	}

	return ;
}