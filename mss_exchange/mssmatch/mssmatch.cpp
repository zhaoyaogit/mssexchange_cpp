/*  
2018-12-07:
金巴特交易所论坛地址： http://kingstargold.cn:8080/goldblog/c/%E9%87%91%E5%B7%B4%E7%89%B9%E4%B8%AD%E5%BF%83%E5%8C%96%E4%BA%A4%E6%98%93%E6%89%80
金巴特交易所作者： 赵耀
金巴特交易所手机： 15216631375
金巴特交易所qq交流群号： 959372362
金巴特交易所作者邮箱： myzhaoyao@126.com
*/    
    
#include "hiredis.h"
#include "DataDispose.h"
#include "msslog.h"
#ifdef WIN32
#include<windows.h>
#endif

#include <set>    
#include <vector>    
#include <iostream>
#include "MssPubfunc.h"

using namespace std;    


CMssLog g_logfile;
S_CFG_FILE g_cfgfile;
CMssPubfunc g_pub;   //可以全局调用的基础函数库

//create by zyao 2018-11-13 for trade match
//说明：暂时只实现基于redis的消息队列list的数据撮合处理
//方案一：基于redis的消息队列进行数据处理                         2018-11-13：暂时只实现该方案
//方案二：基于socket的消息队列进行数据处理,也可以改造将socket用于接收数据后push到redis内存库，再由撮合引擎处理
//方案三：基于redis的消息发布订阅模式获取数据来撮合处理
int main(int argc,char *argv[])    
{    
	g_logfile.Trace("mssexchange",__FILE__,__LINE__,0,"欢迎使用MSS交易所系统");
	//读取配置文件
	CJsonPack jsonConfig;
	CJsonPack reqJson;
	memset(&g_cfgfile,0,sizeof(S_CFG_FILE));
	jsonConfig.FromFile("mssexchange.json");
	jsonConfig["redis"].Get("server_ip",g_cfgfile.ServerIp);
	jsonConfig["redis"].Get("server_port",g_cfgfile.Port);
	//方案一：基于redis的消息队列进行数据处理
	int count = 1;
	redisReply *reply = NULL;
	CDataDispose myDispose;
	struct timeval timeout = {5, 0}; //5s的超时时间
	//redisContext* redisClient = redisConnect("127.0.0.1", 6379);//同redis服务建立连接
	redisContext *redisClient = (redisContext*)redisConnectWithTimeout(g_cfgfile.ServerIp, g_cfgfile.Port, timeout);
	if (redisClient == NULL || redisClient->err)
	{
		if (redisClient)
		{
			g_logfile.Trace("mssexchange",__FILE__,__LINE__,0,"connect redis [%s][%d] error:%s",g_cfgfile.ServerIp,g_cfgfile.Port,redisClient->errstr);
		}
		else
		{
			g_logfile.Trace("mssexchange",__FILE__,__LINE__,0,"connect redis [%s][%d] error: can't allocate redis context.",g_cfgfile.ServerIp,g_cfgfile.Port);
		}
		return -1;
	}
	else
	{
		g_logfile.Trace("mssexchange",__FILE__,__LINE__,0,"取redis队列数据连接redis成功!");
	}
	
	/*
	//如果redis数据库设置了密码，请设置正确的连接密码方能正常使用redis
	reply = (redisReply *)redisCommand(redisClient, "AUTH 123456");//设置同redis建立好的连接的认证密码
	freeReplyObject(reply);
	*/
	
	reply = NULL;
	count = 1;
	while (1)
	{
		g_logfile.Trace("mssexchange",__FILE__,__LINE__,0,"等待redis队列请求");
		//从redis队列list中阻塞式取值，0表示等待直到有消息，非0表示等待X秒然后返回
		reply = (redisReply *)redisCommand(redisClient, "brpop autd 0");//从redis 队列list中阻塞式获取数据
		g_logfile.Trace("mssexchange",__FILE__,__LINE__,0,"收到redis队列数据开始处理第[%d]个请求",count);
		//reply = (redisReply *)redisCommand(redisClient, "get hello");//get执行后得到的结果街是string，对list执行brpop得到的结果集为数组
		//REDIS_REPLY_STRING   1   //返回字符串，查看str,len字段
		//REDIS_REPLY_ARRAY    2    //返回一个数组，查看elements的值（数组个数），通过element[index]的方式访问数组元素，每个数组元素是一个redisReply对象的指针
		//REDIS_REPLY_INTEGER  3  //返回整数，从integer字段获取值
		//REDIS_REPLY_NIL      4      //没有数据返回
		//REDIS_REPLY_STATUS   5   //表示状态，内容通过str字段查看，字符串长度是len字段
		//REDIS_REPLY_ERROR    6    //表示出错，查看出错信息，如上的str,len字段
		if (reply->type == REDIS_REPLY_ERROR)
		{
			g_logfile.Trace("mssexchange",__FILE__,__LINE__,0,"redisCommand failure");
		}
		else if(reply->type == REDIS_REPLY_STRING)  //返回是字符串
		{
			g_logfile.Trace("mssexchange",__FILE__,__LINE__,0,"get string data:%s\n",reply->str);
		}
		else if(reply->type == REDIS_REPLY_INTEGER) //返回是整型值
		{
			g_logfile.Trace("mssexchange",__FILE__,__LINE__,0,"get int data:%lld",reply->integer);
		}
		else if(reply->type == REDIS_REPLY_ARRAY) //返回一个数组，查看elements的值（数组个数），通过element[index]的方式访问数组元素，每个数组元素是一个redisReply对象的指针
		{
			for(unsigned int i = 0; i < reply->elements; i++)
			{
				if(reply->element[i]->type == REDIS_REPLY_STRING)
				{
					
					//收到mssjson格式报文开始进行撮合处理
					if (i == 0)  //当i等于1的时候，得到的字符串是对列名
					{
						g_logfile.Trace("mssexchange",__FILE__,__LINE__,0,"收到数据的redis队列名为[%s]",reply->element[i]->str);
					}
					else if(i == 1)
					{
						g_logfile.Trace("mssexchange",__FILE__,__LINE__,0,"收到的数据为[%s]",reply->element[i]->str);
						reqJson.Clear();
						string temp;
						bool flag = false;
						flag = reqJson.Parse(reply->element[i]->str);
						if (flag == false)
						{
							g_logfile.Trace("mssexchange", __FILE__, __LINE__, 0, "json解析失败,请求数据[%s]", reply->element[i]->str);
							return -1;
						}
						int ret = myDispose.Dispose(reqJson);
						if (ret < 0)
						{
							g_logfile.Trace("mssexchange", __FILE__, __LINE__, 0, "对请求[%s]处理失败", reqJson.ToFormattedString().c_str());
							//此时需要给失败回报
							char RetCode[8] = {0};
							reqJson[JSON_BODY_STRING].Get(JSON_BODY_RET_CODE, RetCode);
						}
					}
				}
			}
		}
		freeReplyObject(reply);
		count++;
	}
	
	reply = NULL;
	redisFree(redisClient);
	return 0;
//方案二：基于socket的消息队列进行数据处理
/*
    printf("pid: %d\n", getpid());    
    BFServer server(3);   //默认启动线程数 
    server.AddSignalEvent(SIGINT, BFServer::QuitCb);  //注册退出信号   
    timeval tv = {10, 0};    
    server.AddTimerEvent(BFServer::TimeOutCb, tv, false);    
    server.SetPort(9998);    
    server.StartRun();    
    printf("done\n");    
  
    return 0; 
*/	
} 
