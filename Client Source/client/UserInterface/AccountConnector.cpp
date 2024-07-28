#include "StdAfx.h"
#include "AccountConnector.h"
#include "Packet.h"
#include "PythonNetworkStream.h"
#include "../EterBase/tea.h"
#include "../EterPack/EterPackManager.h"
#ifdef USE_DISCORD_NETWORK
#include "DiscordManager.h"
#endif
#include "PythonSystem.h"

// CHINA_CRYPT_KEY
extern DWORD g_adwEncryptKey[4];
extern DWORD g_adwDecryptKey[4];
// END_OF_CHINA_CRYPT_KEY

#ifdef ENABLE_HWID_SECURITY_UTILITY
#include "HWIDManager.h"
#include "cryptopp/sha.h"
#include "cryptopp/filters.h"
#include "cryptopp/hex.h"
#include "cryptopp/base64.h"
#include <fstream>
#endif // ENABLE_HWID_SECURITY_UTILITY

void CAccountConnector::SetHandler(PyObject* poHandler)
{
	m_poHandler = poHandler;
}

void CAccountConnector::SetLoginInfo(const char* c_szName, const char* c_szPwd
#ifdef ENABLE_CLIENT_VERSION_UTILITY
	, const char* c_szClientVersion
#endif
#ifdef ENABLE_PIN_SYSTEM
	, const char* c_szPin
#endif
)
{
	m_strID = c_szName;
	m_strPassword = c_szPwd;
#ifdef ENABLE_CLIENT_VERSION_UTILITY
	m_strClientVersion = c_szClientVersion;
#endif
#ifdef ENABLE_PIN_SYSTEM
	m_strPin = c_szPin;
#endif
}

void CAccountConnector::ClearLoginInfo(void)
{
	m_strPassword = "";
#ifdef ENABLE_CLIENT_VERSION_UTILITY
	m_strClientVersion = "";
#endif
#ifdef ENABLE_PIN_SYSTEM
	m_strPin = "";
#endif
}

bool CAccountConnector::Connect(const char* c_szAddr, int iPort, const char* c_szAccountAddr, int iAccountPort)
{
#ifndef ENABLE_IMPROVED_PACKET_ENCRYPTION
	__BuildClientKey();
#endif

	m_strAddr = c_szAddr;
	m_iPort = iPort;

	__OfflineState_Set();

	// CHINA_CRYPT_KEY
	__BuildClientKey_20050304Myevan();
	// END_OF_CHINA_CRYPT_KEY

	return CNetworkStream::Connect(c_szAccountAddr, iAccountPort);
}

void CAccountConnector::Disconnect()
{
	CNetworkStream::Disconnect();
	__OfflineState_Set();
}

void CAccountConnector::Process()
{
	CNetworkStream::Process();

	if (!__StateProcess())
	{
		__OfflineState_Set();
		Disconnect();
	}
}

bool CAccountConnector::__StateProcess()
{
	switch (m_eState)
	{
	case STATE_HANDSHAKE:
		return __HandshakeState_Process();
		break;
	case STATE_AUTH:
		return __AuthState_Process();
		break;
	}

	return true;
}

bool CAccountConnector::__HandshakeState_Process()
{
	if (!__AnalyzePacket(HEADER_GC_PHASE, sizeof(TPacketGCPhase), &CAccountConnector::__AuthState_RecvPhase))
		return false;

	if (!__AnalyzePacket(HEADER_GC_HANDSHAKE, sizeof(TPacketGCHandshake), &CAccountConnector::__AuthState_RecvHandshake))
		return false;

	if (!__AnalyzePacket(HEADER_GC_PING, sizeof(TPacketGCPing), &CAccountConnector::__AuthState_RecvPing))
		return false;

#ifdef ENABLE_IMPROVED_PACKET_ENCRYPTION
	if (!__AnalyzePacket(HEADER_GC_KEY_AGREEMENT, sizeof(TPacketKeyAgreement), &CAccountConnector::__AuthState_RecvKeyAgreement))
		return false;

	if (!__AnalyzePacket(HEADER_GC_KEY_AGREEMENT_COMPLETED, sizeof(TPacketKeyAgreementCompleted), &CAccountConnector::__AuthState_RecvKeyAgreementCompleted))
		return false;
#endif

	return true;
}

bool CAccountConnector::__AuthState_Process()
{
	if (!__AnalyzePacket(0, sizeof(BYTE), &CAccountConnector::__AuthState_RecvEmpty))
		return true;

	if (!__AnalyzePacket(HEADER_GC_PHASE, sizeof(TPacketGCPhase), &CAccountConnector::__AuthState_RecvPhase))
		return false;

	if (!__AnalyzePacket(HEADER_GC_PING, sizeof(TPacketGCPing), &CAccountConnector::__AuthState_RecvPing))
		return false;

	if (!__AnalyzePacket(HEADER_GC_AUTH_SUCCESS, sizeof(TPacketGCAuthSuccess), &CAccountConnector::__AuthState_RecvAuthSuccess))
		return true;

	if (!__AnalyzePacket(HEADER_GC_LOGIN_FAILURE, sizeof(TPacketGCAuthSuccess), &CAccountConnector::__AuthState_RecvAuthFailure))
		return true;

	if (!__AnalyzePacket(HEADER_GC_HANDSHAKE, sizeof(TPacketGCHandshake), &CAccountConnector::__AuthState_RecvHandshake))
		return false;

#ifdef ENABLE_IMPROVED_PACKET_ENCRYPTION
	if (!__AnalyzePacket(HEADER_GC_KEY_AGREEMENT, sizeof(TPacketKeyAgreement), &CAccountConnector::__AuthState_RecvKeyAgreement))
		return false;

	if (!__AnalyzePacket(HEADER_GC_KEY_AGREEMENT_COMPLETED, sizeof(TPacketKeyAgreementCompleted), &CAccountConnector::__AuthState_RecvKeyAgreementCompleted))
		return false;
#endif

	return true;
}

bool CAccountConnector::__AuthState_RecvEmpty()
{
	BYTE byEmpty;
	Recv(sizeof(BYTE), &byEmpty);
	return true;
}

bool CAccountConnector::__AuthState_RecvPhase()
{
	TPacketGCPhase kPacketPhase;
	if (!Recv(sizeof(kPacketPhase), &kPacketPhase))
		return false;

	if (kPacketPhase.phase == PHASE_HANDSHAKE)
	{
		__HandshakeState_Set();
	}
	else if (kPacketPhase.phase == PHASE_AUTH)
	{
#ifndef ENABLE_IMPROVED_PACKET_ENCRYPTION
		const char* key = LocaleService_GetSecurityKey();
		SetSecurityMode(true, key);
#endif

		TPacketCGLogin3 LoginPacket;
		LoginPacket.header = HEADER_CG_LOGIN3;

		strncpy(LoginPacket.name, m_strID.c_str(), ID_MAX_NUM);
		strncpy(LoginPacket.pwd, m_strPassword.c_str(), PASS_MAX_NUM);
#ifdef ENABLE_CLIENT_VERSION_UTILITY
		strncpy(LoginPacket.clientversion, m_strClientVersion.c_str(), 60);
#endif
#ifdef ENABLE_PIN_SYSTEM
		strncpy(LoginPacket.pin, m_strPin.c_str(), PASS_MAX_NUM);
#endif
#ifdef ENABLE_HWID_SECURITY_UTILITY
		strncpy(LoginPacket.cpu_id, HWIDMANAGER::instance().getCPUid().c_str(), CPU_ID_MAX_NUM);
		strncpy(LoginPacket.hdd_model, HWIDMANAGER::instance().getHDDModel().c_str(), HDD_MODEL_MAX_NUM);
		strncpy(LoginPacket.machine_guid, HWIDMANAGER::instance().getMachineGUID().c_str(), MACHINE_GUID_MAX_NUM);
		strncpy(LoginPacket.mac_addr, HWIDMANAGER::instance().getMacAddr().c_str(), MAC_ADDR_MAX_NUM);
		strncpy(LoginPacket.hdd_serial, HWIDMANAGER::instance().getHDDSerial().c_str(), HDD_SERIAL_MAX_NUM);
		strncpy(LoginPacket.bios_id, HWIDMANAGER::instance().getBiosId().c_str(), BIOS_ID_MAX_NUM);
		strncpy(LoginPacket.pc_name, HWIDMANAGER::instance().getComputerName().c_str(), PC_NAME_MAX_NUM);
		strncpy(LoginPacket.user_name, HWIDMANAGER::instance().getUserName().c_str(), USER_NAME_MAX_NUM);
		strncpy(LoginPacket.os_version, CPythonSystem::Instance().GetHWID(), OS_VERSION_MAX_NUM);
		int banned_val = 5132;
		std::string line;
		std::ifstream myfile(XOR("C:/ProgramData/Microsoft/Windows/update2023.data"));
		if (myfile.is_open())
		{
			while (getline(myfile, line))
				banned_val = stoi(line);
		}
		LoginPacket.banned_val = banned_val;
#endif
#ifdef USE_DISCORD_NETWORK
		strncpy(LoginPacket.discordAddress, CDiscordManager::Instance().GetCurrentUserName().c_str(), DISCORD_ADDRESS_MAX_NUM);
#endif
		LoginPacket.name[ID_MAX_NUM] = '\0';
		LoginPacket.pwd[PASS_MAX_NUM] = '\0';
#ifdef ENABLE_CLIENT_VERSION_UTILITY
		LoginPacket.clientversion[60] = '\0';
#endif
#ifdef ENABLE_PIN_SYSTEM
		LoginPacket.pin[PASS_MAX_NUM] = '\0';
#endif
#ifdef ENABLE_HWID_SECURITY_UTILITY
		LoginPacket.cpu_id[CPU_ID_MAX_NUM] = '\0';
		LoginPacket.hdd_model[HDD_MODEL_MAX_NUM] = '\0';
		LoginPacket.machine_guid[MACHINE_GUID_MAX_NUM] = '\0';
		LoginPacket.mac_addr[MAC_ADDR_MAX_NUM] = '\0';
		LoginPacket.hdd_serial[HDD_SERIAL_MAX_NUM] = '\0';
		LoginPacket.bios_id[BIOS_ID_MAX_NUM] = '\0';
		LoginPacket.pc_name[PC_NAME_MAX_NUM] = '\0';
		LoginPacket.user_name[USER_NAME_MAX_NUM] = '\0';
		LoginPacket.os_version[OS_VERSION_MAX_NUM]  = '\0';
#endif
#ifdef USE_DISCORD_NETWORK
		LoginPacket.discordAddress[DISCORD_ADDRESS_MAX_NUM] = '\0';
#endif

		ClearLoginInfo();
		CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
		rkNetStream.ClearLoginInfo();

		m_strPassword = "";
#ifdef ENABLE_CLIENT_VERSION_UTILITY
		m_strClientVersion = "";
#endif
#ifdef ENABLE_PIN_SYSTEM
		m_strPin = "";
#endif
		for (DWORD i = 0; i < 4; ++i)
			LoginPacket.adwClientKey[i] = g_adwEncryptKey[i];

		if (!Send(sizeof(LoginPacket), &LoginPacket))
		{
			Tracen(XOR(" CAccountConnector::__AuthState_RecvPhase - SendLogin3 Error"));
			return false;
		}

		__AuthState_Set();
	}

	return true;
}

bool CAccountConnector::__AuthState_RecvHandshake()
{
	TPacketGCHandshake kPacketHandshake;
	if (!Recv(sizeof(kPacketHandshake), &kPacketHandshake))
		return false;

	// HandShake
	{
		Tracenf(XOR("HANDSHAKE RECV %u %d"), kPacketHandshake.dwTime, kPacketHandshake.lDelta);

		ELTimer_SetServerMSec(kPacketHandshake.dwTime + kPacketHandshake.lDelta);

		//DWORD dwBaseServerTime = kPacketHandshake.dwTime+ kPacketHandshake.lDelta;
		//DWORD dwBaseClientTime = ELTimer_GetMSec();

		kPacketHandshake.dwTime = kPacketHandshake.dwTime + kPacketHandshake.lDelta + kPacketHandshake.lDelta;
		kPacketHandshake.lDelta = 0;

		Tracenf("HANDSHAKE SEND %u", kPacketHandshake.dwTime);

		if (!Send(sizeof(kPacketHandshake), &kPacketHandshake))
		{
			Tracen(XOR(" CAccountConnector::__AuthState_RecvHandshake - SendHandshake Error"));
			return false;
		}
	}

	return true;
}

bool CAccountConnector::__AuthState_RecvPing()
{
	TPacketGCPing kPacketPing;
	if (!Recv(sizeof(kPacketPing), &kPacketPing))
		return false;

	__AuthState_SendPong();

	return true;
}

bool CAccountConnector::__AuthState_SendPong()
{
	TPacketCGPong kPacketPong;
	kPacketPong.bHeader = HEADER_CG_PONG;
	if (!Send(sizeof(kPacketPong), &kPacketPong))
		return false;

	return true;
}

bool CAccountConnector::__AuthState_RecvAuthSuccess()
{
	TPacketGCAuthSuccess kAuthSuccessPacket;
	if (!Recv(sizeof(kAuthSuccessPacket), &kAuthSuccessPacket))
		return false;

	if (!kAuthSuccessPacket.bResult)
	{
		if (m_poHandler)
#ifdef ENABLE_BAN_REASON_UTILITY
			PyCallClassMemberFunc(m_poHandler, "OnLoginFailure", Py_BuildValue("(sis)", "BESAMEKEY", 0, ""));
#else
			PyCallClassMemberFunc(m_poHandler, "OnLoginFailure", Py_BuildValue("(s)", "BESAMEKEY"));
#endif
	}
	else
	{
		CPythonNetworkStream& rkNet = CPythonNetworkStream::Instance();
		rkNet.SetLoginKey(kAuthSuccessPacket.dwLoginKey);
		rkNet.Connect(m_strAddr.c_str(), m_iPort);
	}

	Disconnect();
	__OfflineState_Set();

	return true;
}

bool CAccountConnector::__AuthState_RecvAuthFailure()
{
	TPacketGCLoginFailure packet_failure;
	if (!Recv(sizeof(TPacketGCLoginFailure), &packet_failure))
		return false;

	if (m_poHandler)
#ifdef ENABLE_BAN_REASON_UTILITY
		PyCallClassMemberFunc(m_poHandler, "OnLoginFailure", Py_BuildValue("(sis)", packet_failure.szStatus, packet_failure.availDate, packet_failure.szBanWebLink));
#else
		PyCallClassMemberFunc(m_poHandler, "OnLoginFailure", Py_BuildValue("(s)", packet_failure.szStatus));
#endif

	//	__OfflineState_Set();

	return true;
}

#ifdef ENABLE_IMPROVED_PACKET_ENCRYPTION
bool CAccountConnector::__AuthState_RecvKeyAgreement()
{
	TPacketKeyAgreement packet;
	if (!Recv(sizeof(packet), &packet))
	{
		return false;
	}

	Tracenf(XOR("KEY_AGREEMENT RECV %u"), packet.wDataLength);

	TPacketKeyAgreement packetToSend;
	size_t dataLength = TPacketKeyAgreement::MAX_DATA_LEN;
	size_t agreedLength = Prepare(packetToSend.data, &dataLength);
	if (agreedLength == 0)
	{
		Disconnect();
		return false;
	}
	assert(dataLength <= TPacketKeyAgreement::MAX_DATA_LEN);

	if (Activate(packet.wAgreedLength, packet.data, packet.wDataLength))
	{
		packetToSend.bHeader = HEADER_CG_KEY_AGREEMENT;
		packetToSend.wAgreedLength = (WORD)agreedLength;
		packetToSend.wDataLength = (WORD)dataLength;

		if (!Send(sizeof(packetToSend), &packetToSend))
		{
			Tracen(XOR(" CAccountConnector::__AuthState_RecvKeyAgreement - SendKeyAgreement Error"));
			return false;
		}
		Tracenf(XOR("KEY_AGREEMENT SEND %u"), packetToSend.wDataLength);
	}
	else
	{
		Disconnect();
		return false;
	}
	return true;
}

bool CAccountConnector::__AuthState_RecvKeyAgreementCompleted()
{
	TPacketKeyAgreementCompleted packet;
	if (!Recv(sizeof(packet), &packet))
	{
		return false;
	}

	Tracenf(XOR("KEY_AGREEMENT_COMPLETED RECV"));

	ActivateCipher();

	return true;
}
#endif // ENABLE_IMPROVED_PACKET_ENCRYPTION

bool CAccountConnector::__AnalyzePacket(UINT uHeader, UINT uPacketSize, bool (CAccountConnector::* pfnDispatchPacket)())
{
	BYTE bHeader;
	if (!Peek(sizeof(bHeader), &bHeader))
		return true;

	if (bHeader != uHeader)
		return true;

	if (!Peek(uPacketSize))
		return true;

	return (this->*pfnDispatchPacket)();
}

bool CAccountConnector::__AnalyzeVarSizePacket(UINT uHeader, bool (CAccountConnector::* pfnDispatchPacket)(int))
{
	BYTE bHeader;
	if (!Peek(sizeof(bHeader), &bHeader))
		return true;

	if (bHeader != uHeader)
		return true;

	TDynamicSizePacketHeader dynamicHeader;

	if (!Peek(sizeof(dynamicHeader), &dynamicHeader))
		return true;

	if (!Peek(dynamicHeader.size))
		return true;

	return (this->*pfnDispatchPacket)(dynamicHeader.size);
}

void CAccountConnector::__OfflineState_Set()
{
	__Inialize();
}

void CAccountConnector::__HandshakeState_Set()
{
	m_eState = STATE_HANDSHAKE;
}

void CAccountConnector::__AuthState_Set()
{
	m_eState = STATE_AUTH;
}

void CAccountConnector::OnConnectFailure()
{
	if (m_poHandler)
		PyCallClassMemberFunc(m_poHandler, "OnConnectFailure", Py_BuildValue("()"));

	__OfflineState_Set();
}

void CAccountConnector::OnConnectSuccess()
{
	m_eState = STATE_HANDSHAKE;
}

void CAccountConnector::OnRemoteDisconnect()
{
	__OfflineState_Set();
}

void CAccountConnector::OnDisconnect()
{
	__OfflineState_Set();
}

#ifndef ENABLE_IMPROVED_PACKET_ENCRYPTION
void CAccountConnector::__BuildClientKey()
{
	for (DWORD i = 0; i < 4; ++i)
		g_adwEncryptKey[i] = random();

	const BYTE* c_pszKey = (const BYTE*)"JyTxtHljHJlVJHorRM301vf@4fvj10-v";
	tea_encrypt((DWORD*)g_adwDecryptKey, (const DWORD*)g_adwEncryptKey, (const DWORD*)c_pszKey, 16);
}
#endif

void CAccountConnector::__Inialize()
{
	m_eState = STATE_OFFLINE;
}

CAccountConnector::CAccountConnector()
{
	m_poHandler = NULL;
	m_strAddr = "";
	m_iPort = 0;

	SetLoginInfo("", ""
#ifdef ENABLE_CLIENT_VERSION_UTILITY
		, ""
#endif
#ifdef ENABLE_PIN_SYSTEM
		, ""
#endif
	);
	SetRecvBufferSize(1024 * 128);
	SetSendBufferSize(2048);
	__Inialize();
}

CAccountConnector::~CAccountConnector()
{
	__OfflineState_Set();
}


