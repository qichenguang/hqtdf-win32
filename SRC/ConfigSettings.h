#ifndef __CONFIG_SETTINGS_H__ 
#define __CONFIG_SETTINGS_H__
#include "../tdfsdkinclude/TDFAPIStruct.h"
#include <string>

class ConfigParser;

enum OUTPUTTYPE
{
    OUTPUTTYPE_NO,
    OUTPUTTYPE_SCREN=1, 
    OUTPUTTYPE_FILE = 2,
    OUTPUTTYPE_ALL = OUTPUTTYPE_SCREN | OUTPUTTYPE_FILE,
};

struct ServerInfo
{
	char szIP[32];
	short nPort;
	char szUser[64];
	char szPwd[64];
};

struct ConfigSettings
{
    enum
    {
        DEFAULT_MAX_MEM_BUF=8*1024*1024,
        DEFAULT_MAX_WRITE_TIME_GAP=10,
        DEFAFAULT_OUT_PUT_DEVICE=OUTPUTTYPE_ALL,
        DEFAULT_RECONNECT_COUNT=99999999,
        DEFAULT_RECONNECT_GAP=5,
        DEFAULT_TDF_OPEN_TIME_OUT=30,
        DEFAULT_HEART_BEAT_GAP=10,
        DEFAULT_MISSED_HEAT_BEAT_COUNT=2,
    };

    ConfigSettings();
    ~ConfigSettings();

    //char szIP[32];
    //short nPort;
    //char szUser[32];
    //char szPwd[32];

	ServerInfo si[4];

    int nDate;
    int nTime;
    char* szSubscriptions;          //dynamic memory
    char szMarketList[1024];
    char szDataDir[260];            //absolute path
    int nOutputDevice;
    bool bOutputCodeTable;
    unsigned int nMaxMemBuf;        //default 32M for each kind of data(unit:byte)
    unsigned int nMaxWriteTimeGap;  //default 10's(unit:s)
    unsigned int nReconnectCount;   //
    unsigned int nReconnGap;        //unit:s
    int nDataType;                  //0 means all type.
    unsigned int nCommonLogGap;     //>=1

    //代理连接类别
    bool bEnableProxy;
    TDF_PROXY_TYPE nProxyType;
    char szProxyHostIp[32];
    short nProxyHostPort;
    char szProxyUser[32];
    char szProxyPassword[32];

    unsigned int nOpenTimeOut;//TDF_Open超时时间:
    unsigned int nHeartBeatGap; //心跳时间
    unsigned int nMissedHeartBeatCount;//丢失心跳次数

    bool LoadSettings(const std::string& strConfigFile);
    std::string ToString();
	int ServerCount()
	{
		int nServerCount = 0; 
		for (int i = 0; i < sizeof(si)/sizeof(ServerInfo); ++i)
		{
			if (si[i].szIP && strlen(si[i].szIP) > 0)
				++nServerCount;
		}
		return nServerCount;
	}
    static std::string OutputDeviceToStr(OUTPUTTYPE);
    static std::string DataTypeToStr(DATA_TYPE_FLAG);

    static TDF_PROXY_TYPE ProxyTypeStr2TDF(int* nIsValid, const std::string& strProxy);
private:
	bool GetServerInfo(ConfigParser* pParser, const char* szIp, const char* szPort, const char* szUser, const char* szPwd, ServerInfo& si);
    ConfigParser* m_pConfigParser;
    void RestoreStatus();
};


#endif