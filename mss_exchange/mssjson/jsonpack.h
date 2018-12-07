/*
2018-12-07:
金巴特交易所论坛地址： http://kingstargold.cn:8080/goldblog/c/%E9%87%91%E5%B7%B4%E7%89%B9%E4%B8%AD%E5%BF%83%E5%8C%96%E4%BA%A4%E6%98%93%E6%89%80
金巴特交易所作者： 赵耀
金巴特交易所手机： 15216631375
金巴特交易所qq交流群号： 959372362
金巴特交易所作者邮箱： myzhaoyao@126.com
*/
/*******************************************************************************
 create for kingstar gold preject 20181025 by zyao
 ******************************************************************************/

#ifndef CJSONPACK_KS_
#define CJSONPACK_KS_

#include <stdio.h>
#include <stddef.h>
#include <malloc.h>
#include <errno.h>
#ifdef WIN32

#else
#include <unistd.h>
#endif

#include <limits.h>
#include <math.h>
#include <float.h>
#include <string>
#include <string.h>
#include <map>
#include "jsonbase.h"



//add by zyao 20181022 for win complier begin
#if _MSC_VER
//#ifdef WIN32
#define snprintf _snprintf
#endif
//end by zyao 20181022 for win complier 

#ifdef LIBJSON_EXPORTS
#define JSONLIB_API __declspec(dllexport)
#else
#ifdef WIN_DLL
#define JSONLIB_API __declspec(dllimport)
#else
#define JSONLIB_API 
#endif
#endif

using namespace std;
class JSONLIB_API CJsonPack
{
	//金仕达黄金标准json包接口方法
// public:
// 	string GetJsonHeadValueS(const string& strKey);
// 	string GetJsonPackValueS(const string& strKey);
// 	string GetJsonPackArrayValueS(int item, const string& strKey);
public:     //处理json对象或者json数组的方法
    CJsonPack();
    CJsonPack(const string& strJson);
    CJsonPack(const CJsonPack* pJsonPack);
    CJsonPack(const CJsonPack& pJsonPack);
    virtual ~CJsonPack();

    CJsonPack& operator=(const CJsonPack& pJsonPack);
    bool operator==(const CJsonPack& pJsonPack) const;
    bool Parse(const string& strJson);
    void Clear();
    bool IsEmpty() const;
    bool IsArray() const;
    string ToString() const;
    string ToFormattedString() const;
    const string& GetErrMsg() const
    {
        return(m_strErrMsg);
    }
	//从文件中读取json格式的数据
	bool FromFile(const char *fileName);
public:     // method of ordinary json object
    bool AddEmptySubObject(const string& strKey);
    bool AddEmptySubArray(const string& strKey);
    CJsonPack& operator[](const string& strKey);
    string operator()(const string& strKey) const;
    bool Get(const string& strKey, CJsonPack& pJsonPack) const;
    bool Get(const string& strKey, string& strValue) const;
	bool Get(const string& strKey, char *sValue) const; //add 20181114 by zyao
    bool Get(const string& strKey, int32& iValue) const;
    bool Get(const string& strKey, uint32& uiValue) const;
    bool Get(const string& strKey, int64& llValue) const;
    bool Get(const string& strKey, uint64& ullValue) const;
    bool Get(const string& strKey, bool& bValue) const;
    bool Get(const string& strKey, float& fValue) const;
    bool Get(const string& strKey, double& dValue) const;
    bool Add(const string& strKey, const CJsonPack& pJsonPack);
    bool Add(const string& strKey, const string& strValue);
    bool Add(const string& strKey, int32 iValue);
    bool Add(const string& strKey, uint32 uiValue);
    bool Add(const string& strKey, int64 llValue);
    bool Add(const string& strKey, uint64 ullValue);
    bool Add(const string& strKey, bool bValue, bool bValueAgain);
    bool Add(const string& strKey, float fValue);
    bool Add(const string& strKey, double dValue);
    bool Delete(const string& strKey);
    bool Replace(const string& strKey, const CJsonPack& pJsonPack);
    bool Replace(const string& strKey, const string& strValue);
    bool Replace(const string& strKey, int32 iValue);
    bool Replace(const string& strKey, uint32 uiValue);
    bool Replace(const string& strKey, int64 llValue);
    bool Replace(const string& strKey, uint64 ullValue);
    bool Replace(const string& strKey, bool bValue, bool bValueAgain);
    bool Replace(const string& strKey, float fValue);
    bool Replace(const string& strKey, double dValue);
	//add by zyao 20181206
	bool CreateOrReplace(const string& strKey, const string& strValue);//如果key已经存在则进行替换，如果不存在则新建
public:     // method of json array
    int GetArraySize();  //获取数组的大小
    CJsonPack& operator[](unsigned int uiWhich);
    string operator()(unsigned int uiWhich) const;
    bool Get(int iWhich, CJsonPack& pJsonPack) const;
    bool Get(int iWhich, string& strValue) const;
    bool Get(int iWhich, int32& iValue) const;
    bool Get(int iWhich, uint32& uiValue) const;
    bool Get(int iWhich, int64& llValue) const;
    bool Get(int iWhich, uint64& ullValue) const;
    bool Get(int iWhich, bool& bValue) const;
    bool Get(int iWhich, float& fValue) const;
    bool Get(int iWhich, double& dValue) const;
    bool Add(const CJsonPack& pJsonPack);
    bool Add(const string& strValue);
    bool Add(int32 iValue);
    bool Add(uint32 uiValue);
    bool Add(int64 llValue);
    bool Add(uint64 ullValue);
    bool Add(int iAnywhere, bool bValue);
    bool Add(float fValue);
    bool Add(double dValue);
    bool AddAsFirst(const CJsonPack& pJsonPack);
    bool AddAsFirst(const string& strValue);
    bool AddAsFirst(int32 iValue);
    bool AddAsFirst(uint32 uiValue);
    bool AddAsFirst(int64 llValue);
    bool AddAsFirst(uint64 ullValue);
    bool AddAsFirst(int iAnywhere, bool bValue);
    bool AddAsFirst(float fValue);
    bool AddAsFirst(double dValue);
    bool Delete(int iWhich);
    bool Replace(int iWhich, const CJsonPack& pJsonPack);
    bool Replace(int iWhich, const string& strValue);
    bool Replace(int iWhich, int32 iValue);
    bool Replace(int iWhich, uint32 uiValue);
    bool Replace(int iWhich, int64 llValue);
    bool Replace(int iWhich, uint64 ullValue);
    bool Replace(int iWhich, bool bValue, bool bValueAgain);
    bool Replace(int iWhich, float fValue);
    bool Replace(int iWhich, double dValue);

private:
    CJsonPack(cJSON* pJsonData);

private:
    cJSON* m_pJsonData;
    cJSON* m_pExternJsonDataRef;
    string m_strErrMsg;
    map<unsigned int, CJsonPack*> m_mapJsonArrayRef;
    map<string, CJsonPack*> m_mapJsonPackRef;
};


#endif /* CJSONPACK_KS_ */
