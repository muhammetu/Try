#ifndef _CMacAddr_H____
#define _CMacAddr_H____

#include <string>
#include "CNetworkAdapter.h"

class CMacAddr
{
public:
	CMacAddr();
	~CMacAddr();
	std::string getMacAddr() { return m_macAddr; };

private:
	typedef unsigned char UCHAR;
	void setMACData(std::string test);
	bool IsPrimaryAdapter(DWORD dwIndex);
	bool findValidMac();
	bool initAdapters();
	std::string m_macAddr;
	CNetworkAdapter* m_pAdapters;
	UINT				m_nCount;
};

#endif //CMacAddr