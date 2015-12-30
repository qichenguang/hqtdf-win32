#include "common.h"

using namespace std;

namespace com { namespace autotrade { namespace common {


int MAKEYMD(int y,int m,int d){
	return ((y)*10000+(m)*100+(d));
}

int MAKEHHMMSSsss(int H, int M, int S, int ms) {
	return ((H)*10000000 + (M)*100000 + (S)*1000 + (ms));
}

char SAFE_CHAR(char ch){
	return	((ch) ? (ch) : ' ');
}

char* intarr2str(char* szBuf, int nBufLen, int arr[], int n){
	int nOffset = 0;
	for (int i=0; i<n; i++){
		nOffset += _snprintf(szBuf+nOffset, nBufLen-nOffset, "%d ", arr[i]);
	}
	return szBuf;
}

char* chararr2str(char* szBuf, int nBufLen, char arr[], int n){
	int nOffset = 0;
	for (int i=0; i<n; i++){
		nOffset += _snprintf(szBuf+nOffset, nBufLen-nOffset, "%d(%c) ", arr[i], SAFE_CHAR(arr[i]));
	}
	return szBuf;
}

char* DeepCopy(const char*szIn){
	if (szIn){
		int nLen = strlen(szIn);
		char* pRet = new char[nLen+1];
		pRet[nLen] = 0;
		strncpy(pRet, szIn, nLen+1);
		return pRet;
	}else{
		return NULL;
	}
}



}}} // namespace







