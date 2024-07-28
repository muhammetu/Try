#pragma once

#include "../EterLib/NetStream.h"
#include "../EterLib/FuncObject.h"

class CAccountConnector : public CNetworkStream, public CSingleton<CAccountConnector>
{
public:
	enum
	{
		STATE_OFFLINE,
		STATE_HANDSHAKE,
		STATE_AUTH,
	};

public:
	CAccountConnector();
	virtual ~CAccountConnector();

	void SetHandler(PyObject* poHandler);
	void SetLoginInfo(const char* c_szName, const char* c_szPwd
#ifdef ENABLE_CLIENT_VERSION_UTILITY
		, const char* c_szClientVersion
#endif
#ifdef ENABLE_PIN_SYSTEM
		, const char* c_szPin
#endif
	);
	void ClearLoginInfo(void);

	bool Connect(const char* c_szAddr, int iPort, const char* c_szAccountAddr, int iAccountPort);
	void Disconnect();
	void Process();

protected:
	void OnConnectFailure();
	void OnConnectSuccess();
	void OnRemoteDisconnect();
	void OnDisconnect();

protected:
	void __Inialize();
	bool __StateProcess();

	void __OfflineState_Set();
	void __HandshakeState_Set();
	void __AuthState_Set();

	bool __HandshakeState_Process();
	bool __AuthState_Process();

	bool __AuthState_RecvEmpty();
	bool __AuthState_RecvPhase();
	bool __AuthState_RecvHandshake();
	bool __AuthState_RecvPing();
	bool __AuthState_SendPong();
	bool __AuthState_RecvAuthSuccess();
	bool __AuthState_RecvAuthFailure();
#ifdef ENABLE_IMPROVED_PACKET_ENCRYPTION
	bool __AuthState_RecvKeyAgreement();
	bool __AuthState_RecvKeyAgreementCompleted();
#endif

	bool __AnalyzePacket(UINT uHeader, UINT uPacketSize, bool (CAccountConnector::* pfnDispatchPacket)());
	bool __AnalyzeVarSizePacket(UINT uHeader, bool (CAccountConnector::* pfnDispatchPacket)(int));

#ifndef ENABLE_IMPROVED_PACKET_ENCRYPTION
	void __BuildClientKey();
#endif

protected:
	UINT m_eState;
	std::string m_strID;
	std::string m_strPassword;
#ifdef ENABLE_CLIENT_VERSION_UTILITY
	std::string m_strClientVersion;
#endif
#ifdef ENABLE_PIN_SYSTEM
	std::string m_strPin;
#endif
	std::string m_strAddr;
	int m_iPort;

	PyObject* m_poHandler;

	// CHINA_CRYPT_KEY
	void __BuildClientKey_20050304Myevan();
	// END_OF_CHINA_CRYPT_KEY
};
