#ifndef COMMON_H
#define COMMON_H
//
#include <windows.h>
#include <assert.h>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <map>
#include <queue>
#include <cctype>
#include <assert.h>
#include <list>
#include <assert.h>
#include <time.h>
//
#include "../tdfsdkinclude/TDFAPI.h"
#include "../tdfsdkinclude/TDFAPIStruct.h"
//
using namespace std;

namespace com { namespace autotrade { namespace common {

#define DISPLAY_TIME_GAP(nGap, nLastTime, const_expr, ...)                 \
{                                                                  \
	\
	__time64_t nCurTime = ::_time64(NULL);                          \
	if ((nCurTime - (nLastTime) > (nGap) )|| ((nLastTime)-nCurTime>(nGap)))                        \
	{                                                   \
		Print(const_expr, __VA_ARGS__);                     \
		nLastTime = nCurTime;                               \
	}                                                   \
}

int MAKEYMD(int y,int m,int d);

int MAKEHHMMSSsss(int H, int M, int S, int ms);

template<class ArrayDataType>
int ELEM_COUNT(ArrayDataType arr) {
	return (sizeof(arr)/sizeof(arr[0]));
}

char SAFE_CHAR(char ch);

template<class DataType>
DataType* GETRECORD(void* pBase,int nIndex){
	return ((DataType*)((char*)(pBase) + sizeof(DataType)*(nIndex)));
}

class AutoLock{
public:
    AutoLock(){
        InitializeCriticalSection(&m_cs);
    }
    void Lock(){
        EnterCriticalSection(&m_cs);
    }
    void UnLock(){
        LeaveCriticalSection(&m_cs);
    }
    ~AutoLock(){
        LeaveCriticalSection(&m_cs);
    }
private:
    CRITICAL_SECTION m_cs;
};

struct MarketInfo{
	SYSTEMTIME st;
	unsigned int nServerTime;
	int nItems;
	TDF_MARKET_DATA rawData[1];
};

struct DataBlockHead{
	THANDLE hTdf;//which handle 
	TDF_MSG_ID nMsgId;
	unsigned int m_nTotalSize;
	unsigned int m_nOffset;//
};

char* intarr2str(char* szBuf, int nBufLen, int arr[], int n);

char* chararr2str(char* szBuf, int nBufLen, char arr[], int n);

char* DeepCopy(const char*szIn);

template <class InfoType, class DataType> 
unsigned int GetInfoSize(unsigned int nItemCount){
	assert(nItemCount);
	unsigned int nLen = sizeof(InfoType) + (nItemCount-1)*sizeof(DataType) ;
	return nLen;
}

template <class InfoType, class DataType> 
unsigned int GetInfoTotalSize(unsigned int nItemCount){
    assert(nItemCount);
    unsigned int nLen = sizeof(DataBlockHead) + sizeof(InfoType) + (nItemCount-1)*sizeof(DataType) ;
    return nLen;
}

template <class InfoType>
InfoType* InitDataBlockHead(DataBlockHead** ppHead, char* pBlockStart, unsigned int nBlockSize){
    assert(nBlockSize >= sizeof(DataBlockHead));
    DataBlockHead* pHead = (DataBlockHead*)pBlockStart;
    pHead->m_nTotalSize = nBlockSize;
    pHead->m_nOffset = sizeof(DataBlockHead);
    //
    if (ppHead){
        *ppHead =  (DataBlockHead*)pBlockStart;
    }
    InfoType* pRet = (InfoType*)(pBlockStart + sizeof(DataBlockHead));
    return pRet;
}

}}} // namespace

#endif






