/*
2018-12-07:
金巴特交易所论坛地址： http://kingstargold.cn:8080/goldblog/c/%E9%87%91%E5%B7%B4%E7%89%B9%E4%B8%AD%E5%BF%83%E5%8C%96%E4%BA%A4%E6%98%93%E6%89%80
金巴特交易所作者： 赵耀
金巴特交易所手机： 15216631375
金巴特交易所qq交流群号： 959372362
金巴特交易所作者邮箱： myzhaoyao@126.com
*/

//该文件用来定义基础数据结构或者通用宏定义
#ifndef __BASEDEFINE_H__
#define __BASEDEFINE_H__

//未知定义 
#define MSS_UNKNOW		"1"     //未知状态，所有状态或者值在不确定或者无法获取的时候都可以使用未知定义
//买卖方向 变量名:BuyOrSell
#define MSS_BUY			'b'     //买
#define MSS_SELL		's'     //卖


//市场状态 变量名:
#define MSS_MARKET_OPEN			'1'     //开始
#define MSS_MARKET_PAUSE        '2'		//暂停交易
#define MSS_MARKET_CLOSE		'3'     //休市

//品种代码 
#define MSS_VARIETY1             'autd'		//Au(T+D)合约
#define MSS_VARIETY2             'agtd'		//Ag(T+D)合约

//返回码返回信息对照表
#define MSS_SUCESS				"0000"			//交易处理成功
#define MSS_ERR_UNDEFINE    	"0001"  		//未定义错误
#define MSS_CUSTNO_LENGTH_ERROR "0002"			//客户号长度有误
#define MSS_ENTRUST_ERROR		"0003"			//委托处理失败

//定义各个要素的长度
#define MSS_CUSTNO_LEN          31+1			//最大客户号长度为31
#define MSS_ENTRUSTNO_LEN		31+1			//最大委托单号长度为31
#define MSS_VATIETY_LEN			15+1			//委托买卖的品种长度
#define MSS_MAX_SERIAL_NO       31+1			//流水号最大长度

//定义系统中的最大buf大小
#define MSS_MAX_BUF_LEN			8191+1			//系统最大buf大小为8192

//定义本系统的json对象key字符串值，只有本定义中定义的key值才可以用
#define JSON_HEAD_STRING			"head"			//json对象的head标签值
#define JSON_BODY_STRING			"body"			//json对象的body标签值
#define JSON_BODY_RET_CODE			"RetCode"		//json对象中返回码
#define JSON_BODY_RET_MSG			"RetMsg"		//json对象中的返回信息



//add by zyao 20181114 begin 定义基础结构体 


///委托单要素
typedef struct _MSSEntrustField
{
	char CustNo[MSS_CUSTNO_LEN];		//客户号	
	char EntrustNo[MSS_ENTRUSTNO_LEN];	//委托单号
	char TradeDate[12];					//交易日期
	char TradeTime[12];					//交易时间
	char BuyOrSell[4];					//买卖标志
	double Price;						//价格
	int Quantity;						//买卖数量 Quantity = RemainQuantity + MatchQuantity
	int RemainQuantity;					//剩余手数，在委托被部分成交时使用RemainQuantity <= Quantity
	int MatchQuantity;					//已成交手数
	char Variety[MSS_VATIETY_LEN];		//委托买卖的品种
}S_ENTRUSTFIELD;

//委托回报要素
typedef struct _MSSEntrustReturn
{
	char CustNo[MSS_CUSTNO_LEN];		//客户号	
	char EntrustNo[MSS_ENTRUSTNO_LEN];	//委托单号
	char TradeDate[12];					//交易日期
	char TradeTime[12];					//交易时间
	char BuyOrSell[4];					//买卖标志
	double Price;						//价格
	int Quantity;						//买卖数量 Quantity = RemainQuantity + MatchQuantity
	int RemainQuantity;					//剩余手数，在委托被部分成交时使用RemainQuantity <= Quantity
	int MatchQuantity;					//已成交手数
	char Variety[MSS_VATIETY_LEN];		//委托买卖的品种
	char RetCode[8];					//返回码
	char RetMsg[2048];					//返回信息
}S_ENTRUST_RETURN;

//成交回报要素
typedef struct _MSSDoneReturn
{
	char CustNo[MSS_CUSTNO_LEN];		//客户号	
	char EntrustNo[MSS_ENTRUSTNO_LEN];	//委托单号
	char TradeDate[12];					//交易日期
	char TradeTime[12];					//交易时间
	char BuyOrSell[4];					//买卖标志
	double Price;						//价格
	int Quantity;						//买卖数量 Quantity = RemainQuantity + MatchQuantity
	int RemainQuantity;					//剩余手数，在委托被部分成交时使用RemainQuantity <= Quantity
	int MatchQuantity;					//已成交手数
	char Variety[MSS_VATIETY_LEN];		//委托买卖的品种
	char DoneSerialNo[MSS_MAX_SERIAL_NO];//交易所成交单号
}S_DONE_RETURN;


//配置文件要素
typedef struct _MSSCfgFile
{
	char ServerIp[64];
	int Port;
}S_CFG_FILE;


//end by zyao 20181114 begin 定义基础结构体 
#endif //__BASEDEFINE_H__
