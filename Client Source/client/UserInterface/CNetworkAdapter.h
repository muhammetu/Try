#ifndef _ADAPTER_INFO_H____
#define _ADAPTER_INFO_H____
#include <windows.h>
#include <tchar.h>
#include <iphlpapi.h>
#include <ipifcons.h>
#include <vector>
#include <string>

class CNetworkAdapter;

#ifndef tstring
#ifdef _UNICODE
#define tstring			std::wstring
#else
#define tstring			std::string
#endif
#endif

// import the internet protocol helper libarary
#pragma comment( lib, "iphlpapi.lib" )

#define DEFAULT_GATEWAY_ADDR	0

struct _IPINFO {
	tstring sIp;
	tstring sSubnet;
};

class CIpInfoArray : public std::vector < _IPINFO > {};
class StringArray : public std::vector < tstring > {};

DWORD EnumNetworkAdapters(CNetworkAdapter* lpBuffer, ULONG ulSzBuf, LPDWORD lpdwOutSzBuf);

class CNetworkAdapter {
public:
	CNetworkAdapter();
	~CNetworkAdapter();
	BOOL SetupAdapterInfo(IP_ADAPTER_INFO* pAdaptInfo);

	// information about the adapters name for the users
	// and its name to the system
	tstring GetAdapterName() const;
	tstring GetAdapterDescription() const;

	// dhcp lease access functions
	time_t	GetLeaseObtained() const;
	time_t	GetLeaseExpired() const;

	// access to lists of various server's ip address
	SIZE_T	GetNumIpAddrs() const;
	SIZE_T	GetNumDnsAddrs() const;
	tstring	GetIpAddr(int nIp = 0) const;
	tstring GetSubnetForIpAddr(int nIp = 0) const;
	tstring	GetDnsAddr(int nDns = 0) const;
	tstring GetCurrentIpAddress() const;

	// dhcp function
	BOOL	IsDhcpUsed() const;
	tstring	GetDchpAddr() const;

	// wins function
	BOOL	IsWinsUsed() const;
	tstring GetPrimaryWinsServer() const;
	tstring GetSecondaryWinsServer() const;

	tstring	GetGatewayAddr(int nGateway = DEFAULT_GATEWAY_ADDR) const;
	SIZE_T	GetNumGatewayAddrs() const;

	static	tstring GetAdapterTypeString(UINT nType);
	UINT	GetAdapterType() const;

	DWORD	GetAdapterIndex() const;

protected:
	tstring	GetStringFromArray(const StringArray* pPtr, int nIndex) const;

private:
	tstring			m_sName;		// adapter name with the computer.  For human readable name use m_sDesc.
	tstring			m_sDesc;
	tstring			m_sPriWins;
	tstring			m_sSecWins;
	tstring			m_sDefGateway;
	tstring			m_sDhcpAddr;
	_IPINFO			m_sCurIpAddr;	// this is also in the ip address list but this is the address currently active.
	DWORD			m_dwIndex;		// machine index of the adapter.
	UINT			m_nAdapterType;
	BOOL			m_bDhcpUsed;
	BOOL			m_bWinsUsed;
	StringArray		m_DnsAddresses;
	CIpInfoArray	m_IpAddresses;
	StringArray		m_GatewayList;
	time_t			m_tLeaseObtained;
	time_t			m_tLeaseExpires;

	struct UNNAMED {
		BYTE	ucAddress[MAX_ADAPTER_ADDRESS_LENGTH];
		UINT	nLen;
	} m_ucAddress;
public:
	// returns formatted MAC address in HEX punctuated with "::"
	tstring GetAdapterAddress(void);
};

#endif //_ADAPTER_INFO_H____