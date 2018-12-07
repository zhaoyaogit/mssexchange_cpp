/*
2018-12-07:
金巴特交易所论坛地址： http://kingstargold.cn:8080/goldblog/c/%E9%87%91%E5%B7%B4%E7%89%B9%E4%B8%AD%E5%BF%83%E5%8C%96%E4%BA%A4%E6%98%93%E6%89%80
金巴特交易所作者： 赵耀
金巴特交易所手机： 15216631375
金巴特交易所qq交流群号： 959372362
金巴特交易所作者邮箱： myzhaoyao@126.com
*/
// DataDispose.h: interface for the CDataDispose class.
// Create by zyao 20181114 for mss exchange 撮合处理引擎
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DATADISPOSE_H__CCDC35C7_562B_477D_BA48_A3DB4B042D3F__INCLUDED_)
#define AFX_DATADISPOSE_H__CCDC35C7_562B_477D_BA48_A3DB4B042D3F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "jsonpack.h"  //标准mssjson格式数据包解析类
#include "BaseDefine.h"  //基础结构体定义文件
#include "msslog.h"
#include "hiredis.h"
#include "MssPubfunc.h"
#include <iostream>
#include <string.h>
#include <list>
#include <vector>
#include <time.h>
#ifdef WIN32
#include<windows.h>
#endif
using namespace std;

extern CMssLog g_logfile;
extern S_CFG_FILE g_cfgfile; //全局配置文件
extern CMssPubfunc g_pub;	//可以全局调用的基础库


class CDataDispose  
{
public:
	CDataDispose();
	virtual ~CDataDispose();
public:
	list<S_ENTRUSTFIELD*> m_listEntrustSell;	//卖委托单列表
	list<S_ENTRUSTFIELD*> m_listEntrustBuy;		//买委托单列表
	double m_currentPrice;						//当前最新价格
	char m_errMsg[8192];						//错误信息
	redisContext *m_redisClient;				//redis句柄
	//char m_tradeDate[16];						//存放当前自然日期,用来作为流水号的前缀
public:
	//处理收到的mssjson格式报文
	int Dispose(CJsonPack &reqJson);

	//将委托记录插入买或卖列表中价格合适的位置
	int AddEntrust2List(CJsonPack &reqJson);
	
	//收到一笔委托单开始进行撮合前处理
	int PreMatch(CJsonPack &reqJson);

	//收到一笔委托单开始进行撮合处理
	int MatchEngine(CJsonPack &reqJson);

	//计算当前最新成交价作为行情当前市价
	double GetCurrentMatchPrice(double buyPrice,double salePrice);

	//委托回报
	int EntrustReturn(CJsonPack &reqJson);

	//成交回报
	int DoneReturn(CJsonPack &reqJson);

	//获取全局唯一交易流水号（如果追求效率可将委托成交等交易跟其他交易流水号分开生成）
	//int64 GetGlobalSerialNo();
	char *GetGlobalSerialNo(); //最后还是决定用字符串吧

	//委托处理入口
	int EntrustHandle(CJsonPack &reqJson);

	//将委托请求要素写入结构体中
	int GetValueToBuffer(CJsonPack &reqJson, S_ENTRUSTFIELD *newEntrust);
};

#endif // !defined(AFX_DATADISPOSE_H__CCDC35C7_562B_477D_BA48_A3DB4B042D3F__INCLUDED_)
