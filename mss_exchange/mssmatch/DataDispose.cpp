/*
2018-12-07:
金巴特交易所论坛地址： http://kingstargold.cn:8080/goldblog/c/%E9%87%91%E5%B7%B4%E7%89%B9%E4%B8%AD%E5%BF%83%E5%8C%96%E4%BA%A4%E6%98%93%E6%89%80
金巴特交易所作者： 赵耀
金巴特交易所手机： 15216631375
金巴特交易所qq交流群号： 959372362
金巴特交易所作者邮箱： myzhaoyao@126.com
*/
// DataDispose.cpp: implementation of the CDataDispose class.
//
//////////////////////////////////////////////////////////////////////

#include "DataDispose.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//构造函数
CDataDispose::CDataDispose()
{
	m_listEntrustBuy.clear();
	m_listEntrustSell.clear();
	m_currentPrice = 0.00;
	memset(m_errMsg,0,sizeof(m_errMsg));

	//初始化redis连接
	struct timeval timeout = {5, 0}; //5s的超时时间
	m_redisClient = (redisContext*)redisConnectWithTimeout(g_cfgfile.ServerIp, g_cfgfile.Port, timeout);
	if (m_redisClient == NULL || m_redisClient->err)
	{
		if (m_redisClient)
		{
			g_logfile.Trace("mssexchange",__FILE__,__LINE__,0,"connect redis error:%s",m_redisClient->errstr);
		}
		else
		{
			g_logfile.Trace("mssexchange",__FILE__,__LINE__,0,"connect error: can't allocate redis context.");
		}
		return ;
	}
	else
	{
		g_logfile.Trace("mssexchange",__FILE__,__LINE__,0,"撮合回报处理连接redis成功!");
	}
}

//析构函数
CDataDispose::~CDataDispose()
{
	if (m_redisClient != NULL)
	{
		redisFree(m_redisClient);
	}
}

//各种请求分发处理
int CDataDispose::Dispose(CJsonPack &reqJson)
{
	//变量定义
	char TradeCode[8] = {0};
	int ret = -1;
	//cout << "打印解析到的json包" << endl;
	//cout << readJson.ToFormattedString() << endl;
	//第一步：获取包头信息
	memset(TradeCode,0,sizeof(TradeCode));
	reqJson["head"].Get("tradecode", TradeCode);
	//第二步：生成该笔请求的唯一流水号,写入请求json报文中
	reqJson["body"].Add("GlobalSerial", GetGlobalSerialNo());
	
	switch (atoi(TradeCode))
	{
	case 10001:
		ret = EntrustHandle(reqJson);
		break;
	default:
		sprintf(m_errMsg, "无此功能%d", atoi(TradeCode));
		g_logfile.Trace("mssexchange", __FILE__, __LINE__, 0, "MSS报错:%s", m_errMsg);
		return -1;
	}
	return ret;
}

//处理收到的委托请求
int CDataDispose::EntrustHandle(CJsonPack &reqJson)
{
	S_ENTRUSTFIELD EntrustField;
	int ret = -1;
	//第二步：获取包体
	memset(&EntrustField, 0, sizeof(S_ENTRUSTFIELD));
	GetValueToBuffer(reqJson,&EntrustField);
	//初始化当前价
	if ((m_currentPrice < -0.001) && (m_currentPrice > 0.001))  //double类型数据跟0.00比较的方法
	{
		m_currentPrice = EntrustField.Price; //当前最新价应该按照第一笔成交来确定，这里因为市场第一笔委托没有成交所以把该委托价格作为最新价
	}
	g_logfile.Trace("mssexchange", __FILE__, __LINE__, 0, "测试%s", m_errMsg);
	//收到委托单，开始撮合处理
	ret = -1; //初始化ret
	memset(m_errMsg, 0, sizeof(m_errMsg));
	ret = PreMatch(reqJson);
	if (ret < 0)
	{
		g_logfile.Trace("mssexchange", __FILE__, __LINE__, 0, m_errMsg);
		return -1;
	}
	g_logfile.Trace("mssexchange", __FILE__, __LINE__, 0, "撮合引擎处理请求[%s]完成", EntrustField.EntrustNo);
	return 0;
}

int CDataDispose::PreMatch(CJsonPack &reqJson)
{
	S_ENTRUSTFIELD newEntrust;
	//int ret = -1;
	//第二步：获取包体
	memset(&newEntrust, 0, sizeof(S_ENTRUSTFIELD));
	GetValueToBuffer(reqJson, &newEntrust);
	//对委托请求的剩余手数和成交手数进行预处理
	newEntrust.RemainQuantity = newEntrust.Quantity;	//委托单的初始状态剩余手数等于委托手数
	newEntrust.MatchQuantity = 0;						//委托单的初始状态成交手数等于0

	//预处理委托回报，所有委托请求先给委托回报
	EntrustReturn(reqJson);
	//其他预处理
	if (MSS_BUY == newEntrust.BuyOrSell[0])				//如果是买委托
	{
		if (m_listEntrustSell.empty())
		{
			int ret = -1;
			ret = AddEntrust2List(reqJson);
			if (ret < 0)
			{
				g_logfile.Trace("mssexchange", __FILE__, __LINE__, 0, "委托请求[%s]加入委托列表失败", newEntrust.EntrustNo);
				reqJson["body"].CreateOrReplace(JSON_BODY_RET_MSG, "委托处理失败");
				reqJson["body"].CreateOrReplace(JSON_BODY_RET_CODE, MSS_ENTRUST_ERROR);
				return -1;
			}
			return 0;
		}
		if (newEntrust.Price < m_listEntrustSell.front()->Price) //当前委托价格小于当前的最低卖出价格，则该委托不会被成交，直接加入买队列
		{
			int ret = -1;
			ret = AddEntrust2List(reqJson);
			if (ret < 0)
			{
				g_logfile.Trace("mssexchange", __FILE__, __LINE__, 0, "委托请求[%s]加入委托列表失败", newEntrust.EntrustNo);
				reqJson["body"].CreateOrReplace(JSON_BODY_RET_MSG, "委托处理失败");
				reqJson["body"].CreateOrReplace(JSON_BODY_RET_CODE, MSS_ENTRUST_ERROR);
				return -1;
			}
			else
			{
				//暂不处理
			}
		}
		else  //否则需要进行撮合
		{
			MatchEngine(reqJson);
		}
	}
	else if (MSS_SELL == newEntrust.BuyOrSell[0])
	{
		if (m_listEntrustSell.empty())
		{
			int ret = -1;
			ret = AddEntrust2List(reqJson);
			if (ret < 0)
			{
				g_logfile.Trace("mssexchange", __FILE__, __LINE__, 0, "委托请求[%s]加入委托列表失败\n", newEntrust.EntrustNo);
				return -1;
			}
			return 0;
		}
		if (newEntrust.Price > m_listEntrustSell.front()->Price) //当前委托价格大于买列表中当前的最高买入价格，则该委托不会被成交，直接加入卖出队列
		{
			int ret = -1;
			ret = AddEntrust2List(reqJson);
			if (ret < 0)
			{
				g_logfile.Trace("mssexchange", __FILE__, __LINE__, 0, "委托请求[%s]加入委托列表失败\n", newEntrust.EntrustNo);
				return -1;
			}
			else
			{
				//委托请求加入委托列表成功，为了效率成功的就不打日志了
			}
		}
		else  //否则需要进行撮合
		{
			MatchEngine(reqJson);
		}
	}
	else
	{
		g_logfile.Trace("mssexchange", __FILE__, __LINE__, 0, "委托请求[%s]买卖标志[%s]校验失败\n", newEntrust.EntrustNo, newEntrust.BuyOrSell);
		return -1;
	}
	return 0;
}


//将委托记录插入买或卖列表中价格合适的位置
//买队列按照价格从大到小的顺序排序：m_listEntrustBuy
//卖队列按照价格从小到大的顺序排序：m_listEntrustSell
int CDataDispose::AddEntrust2List(CJsonPack &reqJson)
{
	S_ENTRUSTFIELD newEntrust;
	
	//第二步：获取包体
	memset(&newEntrust, 0, sizeof(S_ENTRUSTFIELD));
	GetValueToBuffer(reqJson, &newEntrust);
	//对价格进行判断，如果买委托价格小于当前价
	if (MSS_BUY == newEntrust.BuyOrSell[0])   //买委托单加入list列表
	{
		auto it=m_listEntrustBuy.begin();
		for(; it != m_listEntrustBuy.end() ;it++)
		{
			if(newEntrust.Price > (*it)->Price)  //按照价格从大到小排序，插入到小于该委托金额的元素之前，相等价格之后
			{
				m_listEntrustBuy.insert(it,&newEntrust);
				break;
			}
		}
	}
	else if(MSS_SELL == newEntrust.BuyOrSell[0])  //卖委托单加入list列表
	{
		auto it=m_listEntrustSell.begin();
		for(; it != m_listEntrustSell.end() ;it++)
		{
			if(newEntrust.Price < (*it)->Price)  //按照价格从小到大排序，插入到大于该委托金额的元素之前，相等价格之后
			{
				m_listEntrustSell.insert(it,&newEntrust);
				break;
			}
		}
	}
	else //买卖方向未知异常处理
	{
		g_logfile.Trace("mssexchange",__FILE__,__LINE__,0,"委托请求[%s]买卖方向未知,无法插入列表",newEntrust.EntrustNo);
		reqJson["body"].CreateOrReplace(JSON_BODY_RET_MSG, "委托处理失败");
		reqJson["body"].CreateOrReplace(JSON_BODY_RET_CODE, MSS_ENTRUST_ERROR);
		return -1;
	}
	return 0;
}
//add by zyao 20181114 撮合核心处理模块
int CDataDispose::MatchEngine(CJsonPack &reqJson)
{
	S_ENTRUSTFIELD newEntrust;
	
	//第二步：获取包体
	memset(&newEntrust, 0, sizeof(S_ENTRUSTFIELD));
	GetValueToBuffer(reqJson, &newEntrust);
	if (MSS_BUY == newEntrust.BuyOrSell[0])  //如果是买委托
	{
		for (auto SellListRecord:m_listEntrustSell) //遍历，价格有大到小
		{
			if (newEntrust.Price >= SellListRecord->Price)  //买委托价格大于等于当前遍历节点的卖委托价格，则可以撮合
			{
				if (newEntrust.Quantity < SellListRecord->RemainQuantity) //卖列表中价格优先的第一笔单子数量足够成交当前的委托
				{
					newEntrust.RemainQuantity = 0;						//足够完全成交，故剩余手数为0
					newEntrust.MatchQuantity = newEntrust.Quantity;		//委托手数全部能成交，所以成交手数等于委托手数
					SellListRecord->RemainQuantity = SellListRecord->RemainQuantity - newEntrust.Quantity;
					SellListRecord->MatchQuantity = SellListRecord->MatchQuantity + newEntrust.Quantity;
					//m_currentPrice = SellListRecord->Price;				//设置当前最新价
					m_currentPrice = GetCurrentMatchPrice(newEntrust.Price,SellListRecord->Price);
					//发送成交回报
					DoneReturn(reqJson);
				}
				else if (newEntrust.Quantity == SellListRecord->RemainQuantity) //委托手数跟买列表的首节点完全成交
				{
					newEntrust.RemainQuantity = 0;						//足够完全成交，故剩余手数为0
					newEntrust.MatchQuantity = newEntrust.Quantity;		//委托手数全部能成交，所以成交手数等于委托手数
					SellListRecord->RemainQuantity = SellListRecord->RemainQuantity - newEntrust.Quantity;	//买卖方完全成交,SellListRecord.RemainQuantity的值应该等于0
					SellListRecord->MatchQuantity = SellListRecord->MatchQuantity + newEntrust.Quantity;		//买卖方完全成交,SellListRecord.MatchQuantity的值应该等于SellListRecord.Quantity
					//m_currentPrice = SellListRecord->Price;				//设置当前最新价
					m_currentPrice = GetCurrentMatchPrice(newEntrust.Price,SellListRecord->Price);
					//卖列表的首节点被完全成交了，所以应该删除该节点
					m_listEntrustSell.pop_front();
					//发送成交回报
					DoneReturn(reqJson);
				}
				else if (newEntrust.Quantity > SellListRecord->RemainQuantity)
				{
					newEntrust.RemainQuantity = newEntrust.RemainQuantity - SellListRecord->RemainQuantity;						//足够完全成交，故剩余手数为0
					newEntrust.MatchQuantity = newEntrust.MatchQuantity + SellListRecord->RemainQuantity;	//成交委托手数等于原已成交手数+当前成交手数
					SellListRecord->RemainQuantity = 0;					//买卖方完全成交,SellListRecord->RemainQuantity的值应该等于0
					SellListRecord->MatchQuantity = SellListRecord->MatchQuantity + SellListRecord->RemainQuantity;		//等于已成交的手数MatchQuantity + 本次成交的手数RemainQuantity
					//m_currentPrice = SellListRecord->Price;				//设置当前最新价
					m_currentPrice = GetCurrentMatchPrice(newEntrust.Price,SellListRecord->Price);
					//卖列表的首节点被完全成交了，所以应该删除该节点
					m_listEntrustSell.pop_front();
					//发送成交回报
					DoneReturn(reqJson);
				}
			}
			else  //买委托价格小于当前节点的价格,无法成交则将剩余的委托记录插入买委托列表中
			{
				int ret  = -1;
				ret = AddEntrust2List(reqJson);
				if (ret < 0)
				{
					g_logfile.Trace("mssexchange",__FILE__,__LINE__,0,"插入委托请求[%s]到列表失败\n",newEntrust.EntrustNo);
					reqJson["body"].CreateOrReplace(JSON_BODY_RET_MSG, "委托处理失败");
					reqJson["body"].CreateOrReplace(JSON_BODY_RET_CODE, MSS_ENTRUST_ERROR);
					return -1;
				}
			}
		}
	}
	else if (MSS_SELL == newEntrust.BuyOrSell[0])  //卖出委托单处理
	{
		for (auto BuyListRecord:m_listEntrustBuy) //遍历，价格有小到大
		{
			if (newEntrust.Price <= BuyListRecord->Price)					//卖委托价格小于等于当前遍历节点的卖委托价格，则可以撮合
			{
				if (newEntrust.Quantity < BuyListRecord->RemainQuantity) //卖列表中价格优先的第一笔单子数量足够成交当前的委托
				{
					newEntrust.RemainQuantity = 0;						//足够完全成交，故剩余手数为0
					newEntrust.MatchQuantity = newEntrust.Quantity;		//委托手数全部能成交，所以成交手数等于委托手数
					BuyListRecord->RemainQuantity = BuyListRecord->RemainQuantity - newEntrust.Quantity;
					BuyListRecord->MatchQuantity = BuyListRecord->MatchQuantity + newEntrust.Quantity;
					//m_currentPrice = BuyListRecord->Price;				//设置当前最新价
					m_currentPrice = GetCurrentMatchPrice(BuyListRecord->Price,newEntrust.Price);
					//发送成交回报
					DoneReturn(reqJson);
				}
				else if (newEntrust.Quantity == BuyListRecord->RemainQuantity) //委托手数跟买列表的首节点完全成交
				{
					newEntrust.RemainQuantity = 0;						//足够完全成交，故剩余手数为0
					newEntrust.MatchQuantity = newEntrust.Quantity;		//委托手数全部能成交，所以成交手数等于委托手数
					BuyListRecord->RemainQuantity = BuyListRecord->RemainQuantity - newEntrust.Quantity;	//买卖方完全成交,SellListRecord.RemainQuantity的值应该等于0
					BuyListRecord->MatchQuantity = BuyListRecord->MatchQuantity + newEntrust.Quantity;		//买卖方完全成交,SellListRecord.MatchQuantity的值应该等于SellListRecord.Quantity
					//m_currentPrice = BuyListRecord->Price;				//设置当前最新价
					m_currentPrice = GetCurrentMatchPrice(BuyListRecord->Price,newEntrust.Price);
					//卖列表的首节点被完全成交了，所以应该删除该节点
					m_listEntrustSell.pop_front();
					//发送成交回报
					DoneReturn(reqJson);
				}
				else if (newEntrust.Quantity > BuyListRecord->RemainQuantity)
				{
					newEntrust.RemainQuantity = newEntrust.RemainQuantity - BuyListRecord->RemainQuantity;						//足够完全成交，故剩余手数为0
					newEntrust.MatchQuantity = newEntrust.MatchQuantity + BuyListRecord->RemainQuantity;	//成交委托手数等于原已成交手数+当前成交手数
					BuyListRecord->RemainQuantity = 0;					//买卖方完全成交,SellListRecord->RemainQuantity的值应该等于0
					BuyListRecord->MatchQuantity = BuyListRecord->MatchQuantity + BuyListRecord->RemainQuantity;		//等于已成交的手数MatchQuantity + 本次成交的手数RemainQuantity
					//m_currentPrice = BuyListRecord->Price;				//设置当前最新价
					m_currentPrice = GetCurrentMatchPrice(BuyListRecord->Price,newEntrust.Price);
					//卖列表的首节点被完全成交了，所以应该删除该节点
					m_listEntrustSell.pop_front();
					//发送成交回报
					DoneReturn(reqJson);
				}
			}
			else  //买委托价格大于当前节点的价格,无法成交则将剩余的委托记录插入买委托列表中
			{
				int ret  = -1;
				ret = AddEntrust2List(reqJson);
				if (ret < 0)
				{
					g_logfile.Trace("mssexchange",__FILE__,__LINE__,0,"插入委托请求[%s]到列表失败",newEntrust.EntrustNo);
					reqJson["body"].CreateOrReplace(JSON_BODY_RET_MSG, "委托处理失败");
					reqJson["body"].CreateOrReplace(JSON_BODY_RET_CODE, MSS_ENTRUST_ERROR);
					return -1;
				}
			}
		}
	}
	return 0;
}

//计算当前成交价
double CDataDispose::GetCurrentMatchPrice(double buyPrice,double salePrice)
{
	if(m_currentPrice<=salePrice)
	{
		m_currentPrice = salePrice;
	}
	else if(m_currentPrice>salePrice && m_currentPrice<=buyPrice)
	{
		m_currentPrice= m_currentPrice;
	}
	else if(m_currentPrice>buyPrice)
	{
		m_currentPrice= buyPrice;
	}
	else
	{
		m_currentPrice= m_currentPrice;
	}
	return m_currentPrice;
}

//将委托回报发布到redis，可以用其他模块订阅然后分发给各个客户（暂时不处理分发）
int CDataDispose::EntrustReturn(CJsonPack &reqJson)
{
	//组装委托回报内容
	S_ENTRUSTFIELD newEntrust;
	
	//第二步：获取包体
	memset(&newEntrust, 0, sizeof(S_ENTRUSTFIELD));
	GetValueToBuffer(reqJson, &newEntrust);
	//第一步:构造一个json对象
	CJsonPack entrustReturnJson;
	entrustReturnJson.AddEmptySubObject("head");
	entrustReturnJson["head"].Add("RetCode",		MSS_SUCESS);
	entrustReturnJson["head"].Add("RetMsg",			"委托回报测试返回信息");
	entrustReturnJson.AddEmptySubObject("body");
	entrustReturnJson["body"].Add("CustNo",		newEntrust.CustNo);
	entrustReturnJson["body"].Add("EntrustNo",	newEntrust.EntrustNo);
	entrustReturnJson["body"].Add("TradeDate",	newEntrust.TradeDate);
	entrustReturnJson["body"].Add("TradeTime",	newEntrust.TradeTime);
	//entrustReturnJson["body"].Add("Price",		newEntrust.Price);
	entrustReturnJson["body"].Add("Variety",	newEntrust.Variety);
	redisReply *reply = NULL;
	reply = (redisReply *)redisCommand(m_redisClient, "PUBLISH %s %s", "entrust_return", entrustReturnJson.ToString().c_str());//发布委托回报entrust_return通道，标识为：entrust_return，count为发布的内容可以替换为行情报文
	freeReplyObject(reply);
	reply = NULL;
	return 0;
}


//将成交回报发布到redis，可以用其他模块订阅然后分发给各个客户（暂时不处理分发）
int CDataDispose::DoneReturn(CJsonPack &reqJson)
{
	//组装委托回报内容
	S_ENTRUSTFIELD newEntrust;
	
	//第二步：获取包体
	memset(&newEntrust, 0, sizeof(S_ENTRUSTFIELD));
	GetValueToBuffer(reqJson, &newEntrust);
	//第一步:构造一个json对象
	CJsonPack entrustReturnJson;
	entrustReturnJson.AddEmptySubObject("head");
	entrustReturnJson["head"].Add("RetCode",		MSS_SUCESS);
	entrustReturnJson["head"].Add("RetMsg",			"成交回报测试返回信息");
	entrustReturnJson.AddEmptySubObject("body");
	entrustReturnJson["body"].Add("CustNo",		newEntrust.CustNo);
	entrustReturnJson["body"].Add("EntrustNo",	newEntrust.EntrustNo);
	entrustReturnJson["body"].Add("TradeDate",	newEntrust.TradeDate);
	entrustReturnJson["body"].Add("TradeTime",	newEntrust.TradeTime);
	entrustReturnJson["body"].Add("Price",		newEntrust.Price);//成交价格这里是简单处理，实际不一定是这个值
	entrustReturnJson["body"].Add("Variety",	newEntrust.Variety);
	//生成成交编号
	entrustReturnJson["body"].Add("DoneSerialNo",GetGlobalSerialNo());
	redisReply *reply = NULL;
	reply = (redisReply *)redisCommand(m_redisClient, "PUBLISH %s %s", "entrust_return", entrustReturnJson.ToString().c_str());//发布委托回报entrust_return通道，标识为：entrust_return，count为发布的内容可以替换为行情报文
	freeReplyObject(reply);
	reply = NULL;
	return 0;
}


//获取全局唯一交易流水号（如果追求效率可将委托成交等交易跟其他交易流水号分开生成增加一个前缀进行区别）
char* CDataDispose::GetGlobalSerialNo()
{
	static char cSerail[MSS_MAX_SERIAL_NO] = { 0 };
	redisReply *reply = NULL;
	reply = (redisReply *)redisCommand(m_redisClient, "INCR serailno");
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
		//g_logfile.Trace("mssexchange",__FILE__,__LINE__,0,"get int data:%lld",reply->integer);
		sprintf(cSerail, "%s%08lld", g_pub.Today(), reply->integer);
	}
	else 
	{
		g_logfile.Trace("mssexchange", __FILE__, __LINE__, 0, "获取全局流水号序号异常");
	}
	
	freeReplyObject(reply);
	reply = NULL;
	//g_logfile.Trace("mssexchange", __FILE__, __LINE__, 0, cSerail);
	return cSerail;
}


//将委托请求要素写入结构体中
//2018-12-06：将reqJson对象中的业务要素复制到结构体S_ENTRUSTFIELD中
int CDataDispose::GetValueToBuffer(CJsonPack &reqJson, S_ENTRUSTFIELD *newEntrust)
{
	//获取请求要素值
	reqJson["body"].Get("CustNo", newEntrust->CustNo);
	reqJson["body"].Get("EntrustNo", newEntrust->EntrustNo);
	reqJson["body"].Get("BuyOrSell", newEntrust->BuyOrSell);
	reqJson["body"].Get("Price", newEntrust->Price);
	reqJson["body"].Get("Quantity", newEntrust->Quantity);
	reqJson["body"].Get("TradeTime", newEntrust->TradeTime);
	reqJson["body"].Get("TradeDate", newEntrust->TradeDate);
	reqJson["body"].Get("Variety", newEntrust->Variety);
	//对值的有效性进行校验
	if (strlen(newEntrust->CustNo) < 6)
	{
		reqJson["body"].CreateOrReplace("RetMsg", "客户号长度校验有误");
		reqJson["body"].CreateOrReplace("RetCode", MSS_CUSTNO_LENGTH_ERROR);
	}
	return 0;
}