#pragma once
#include "constants.h"
#include "input.h"
#ifdef __IMPROVED_PACKET_ENCRYPTION__
#include "cipher.h"
#endif

#define MAX_ALLOW_USER                  4096
//#define MAX_INPUT_LEN			2048
#define MAX_INPUT_LEN			65536

#define HANDSHAKE_RETRY_LIMIT		32

class CInputProcessor;

enum EDescType
{
	DESC_TYPE_ACCEPTOR,
	DESC_TYPE_CONNECTOR
};

class CLoginKey
{
public:
	CLoginKey(DWORD dwKey, LPDESC pkDesc) : m_dwKey(dwKey), m_pkDesc(pkDesc)
	{
		m_dwExpireTime = 0;
	}

	void Expire()
	{
		m_dwExpireTime = get_dword_time();
		m_pkDesc = NULL;
	}

	operator DWORD() const
	{
		return m_dwKey;
	}

	DWORD   m_dwKey;
	DWORD   m_dwExpireTime;
	LPDESC  m_pkDesc;
};

class DESC
{
public:
	EVENTINFO(desc_event_info)
	{
		LPDESC desc;

		desc_event_info()
			: desc(0)
		{
		}
	};

public:
	DESC();
	virtual ~DESC();

	virtual BYTE		GetType() { return DESC_TYPE_ACCEPTOR; }
	virtual void		Destroy();
	virtual void		SetPhase(int _phase);

	void			FlushOutput();

	bool			Setup(LPFDWATCH _fdw, socket_t _fd, const struct sockaddr_in& c_rSockAddr, DWORD _handle, DWORD _handshake);

	socket_t		GetSocket() const { return m_sock; }
	const char* GetHostName() { return m_stHost.c_str(); }
	WORD			GetPort() { return m_wPort; }

	void			SetP2P(const char* h, WORD w, BYTE b) { m_stP2PHost = h; m_wP2PPort = w; m_bP2PChannel = b; }
	const char* GetP2PHost() { return m_stP2PHost.c_str(); }
	WORD			GetP2PPort() const { return m_wP2PPort; }
	BYTE			GetP2PChannel() const { return m_bP2PChannel; }

	void			BufferedPacket(const void* c_pvData, int iSize);
	void			Packet(const void* c_pvData, int iSize);
	void			LargePacket(const void* c_pvData, int iSize);

	int				ProcessInput();		// returns -1 if error
	int				ProcessOutput();	// returns -1 if error

	CInputProcessor* GetInputProcessor() { return m_pInputProcessor; }

	DWORD			GetHandle() const { return m_dwHandle; }
	LPBUFFER		GetOutputBuffer() { return m_lpOutputBuffer; }

	void			BindAccountTable(TAccountTable* pTable);
	TAccountTable& GetAccountTable() { return m_accountTable; }

	void			BindCharacter(LPCHARACTER ch);
	LPCHARACTER		GetCharacter() { return m_lpCharacter; }

	bool			IsPhase(int phase) const { return m_iPhase == phase ? true : false; }

	const struct sockaddr_in& GetAddr() { return m_SockAddr; }

	void			Log(const char* format, ...);

	void			StartHandshake(DWORD _dw);
	void			SendHandshake(DWORD dwCurTime, long lNewDelta);
	bool			HandshakeProcess(DWORD dwTime, long lDelta, bool bInfiniteRetry = false);
	bool			IsHandshaking();

	DWORD			GetHandshake() const { return m_dwHandshake; }
	DWORD			GetClientTime();

#ifdef __IMPROVED_PACKET_ENCRYPTION__
	void SendKeyAgreement();
	void SendKeyAgreementCompleted();
	bool FinishHandshake(size_t agreed_length, const void* buffer, size_t length);
	bool IsCipherPrepared();
#else
	// Obsolete encryption stuff here
	void			SetSecurityKey(const DWORD* c_pdwKey);
	const DWORD* GetEncryptionKey() const { return &m_adwEncryptionKey[0]; }
	const DWORD* GetDecryptionKey() const { return &m_adwDecryptionKey[0]; }
#endif

	BYTE			GetEmpire();

	// for p2p
	void			SetRelay(const char* c_pszName);
	bool			DelayedDisconnect(int iSec);
	void			DisconnectOfSameLogin();

	void			SetPong(bool b);
	bool			IsPong();
	void			SendLoginSuccessPacket();

	void			SetLoginKey(DWORD dwKey);
	void			SetLoginKey(CLoginKey* pkKey);
	DWORD			GetLoginKey();

	bool			isChannelStatusRequested() const { return m_bChannelStatusRequested; }
	void			SetChannelStatusRequested(bool bChannelStatusRequested) { m_bChannelStatusRequested = bChannelStatusRequested; }

#ifdef __HWID_SECURITY_UTILITY__
	// Get
	const char* GetCPUID() const { return m_cpuID.c_str(); }
	const char* GetHDDModel() const { return m_hddModel.c_str(); }
	const char* GetMachineGuid() const { return m_machineGuid.c_str(); }
	const char* GetMacAddr() const { return m_macAddr.c_str(); }
	const char* GetHDDSerial() const { return m_hddSerial.c_str(); }
	const char* GetBiosID() const { return m_biosID.c_str(); }
	const char* GetComputerName() const { return m_computerName.c_str(); }
	const char* GetComputerUserName() const { return m_computerUserName.c_str(); }
	const char* GetComputerOS() const { return m_computeros.c_str(); }

	// Set
	void			SetCPUID(std::string val) { m_cpuID = val; }
	void			SetHDDModel(std::string val) { m_hddModel = val; }
	void			SetMachineGuid(std::string val) { m_machineGuid = val; }
	void			SetMacAddr(std::string val) { m_macAddr = val; }
	void			SetHDDSerial(std::string val) { m_hddSerial = val; }
	void			SetBiosID(std::string val) { m_biosID = val; }
	void			SetComputerName(std::string val) { m_computerName = val; }
	void			SetComputerUserName(std::string val) { m_computerUserName = val; }
	void			SetComputerOS(std::string val) { m_computeros = val; }
#endif // __HWID_SECURITY_UTILITY__

protected:
	void			Initialize();

protected:
	CInputProcessor* m_pInputProcessor;
	CInputClose		m_inputClose;
	CInputHandshake	m_inputHandshake;
	CInputLogin		m_inputLogin;
	CInputMain		m_inputMain;
	CInputDead		m_inputDead;
	CInputAuth		m_inputAuth;

	LPFDWATCH		m_lpFdw;
	socket_t		m_sock;
	int				m_iPhase;
	DWORD			m_dwHandle;

	std::string		m_stHost;
	WORD			m_wPort;
	time_t			m_LastTryToConnectTime;

	LPBUFFER		m_lpInputBuffer;
	int				m_iMinInputBufferLen;

	DWORD			m_dwHandshake;
	DWORD			m_dwHandshakeSentTime;
	int				m_iHandshakeRetry;
	DWORD			m_dwClientTime;
	bool			m_bHandshaking;

	LPBUFFER		m_lpBufferedOutputBuffer;
	LPBUFFER		m_lpOutputBuffer;

	LPEVENT			m_pkPingEvent;
	LPCHARACTER		m_lpCharacter;
	TAccountTable		m_accountTable;

	struct sockaddr_in	m_SockAddr;

	FILE* m_pLogFile;
	std::string		m_stRelayName;

	std::string		m_stP2PHost;
	WORD			m_wP2PPort;
#ifdef __HWID_SECURITY_UTILITY__
	std::string		m_cpuID;
	std::string		m_hddModel;
	std::string		m_machineGuid;
	std::string		m_macAddr;
	std::string		m_hddSerial;
	std::string		m_biosID;
	std::string		m_computerName;
	std::string		m_computerUserName;
	std::string		m_computeros;
#endif // __HWID_SECURITY_UTILITY__
	BYTE			m_bP2PChannel;

	bool			m_bPong;

	CLoginKey* m_pkLoginKey;
	DWORD			m_dwLoginKey;

	std::string		m_Login;
	int				m_outtime;
	int				m_playtime;
	int				m_offtime;

	bool			m_bDestroyed;
	bool			m_bChannelStatusRequested;

#ifdef __IMPROVED_PACKET_ENCRYPTION__
	Cipher cipher_;
#else
	// Obsolete encryption stuff here
	bool			m_bEncrypted;
	DWORD			m_adwDecryptionKey[4];
	DWORD			m_adwEncryptionKey[4];
#endif

public:
	LPEVENT			m_pkDisconnectEvent;

	void SetLogin(const std::string& login) { m_Login = login; }
	void SetLogin(const char* login) { m_Login = login; }
	const std::string& GetLogin() { return m_Login; }

	void SetOutTime(int outtime) { m_outtime = outtime; }
	void SetOffTime(int offtime) { m_offtime = offtime; }
	void SetPlayTime(int playtime) { m_playtime = playtime; }

	void ChatPacket(BYTE type, const char* format, ...);
};