#include "StdAfx.h"
#include "PythonNetworkStream.h"
#include "Packet.h"
#include "AccountConnector.h"
#ifdef USE_DISCORD_RPC_MODULE
#include "DiscordManager.h"
#endif // USE_DISCORD_RPC_MODULE
#ifdef USE_LOADING_DLG_OPTIMIZATION
#include "PythonPlayerSettingsModule.h"
#endif // USE_LOADING_DLG_OPTIMIZATION

// Login ---------------------------------------------------------------------------
void CPythonNetworkStream::LoginPhase()
{
	TPacketHeader header;
	if (!CheckPacket(&header))
		return;
#if defined(ENABLE_PRINT_RECV_PACKET_DEBUG)
	TraceError("RECV HEADER : %u , phase %s ", header, m_strPhase.c_str());
#endif

	switch (header)
	{
	case HEADER_GC_PHASE:
		if (RecvPhasePacket())
			return;
		break;

	case HEADER_GC_LOGIN_SUCCESS4:
		if (__RecvLoginSuccessPacket4())
			return;
		break;

	case HEADER_GC_LOGIN_FAILURE:
		if (__RecvLoginFailurePacket())
			return;
		break;

	case HEADER_GC_PING:
		if (RecvPingPacket())
			return;
		break;

	case HEADER_GC_EMPIRE:
		if (__RecvEmpirePacket())
			return;
		break;

	default:
		if (RecvDefaultPacket(header))
			return;
		break;
	}

	RecvErrorPacket(header);
}

void CPythonNetworkStream::SetLoginPhase()
{
#ifndef ENABLE_IMPROVED_PACKET_ENCRYPTION
	const char* key = LSS_SECURITY_KEY;
	SetSecurityMode(true, key);
#endif

	if ("Login" != m_strPhase)
		m_phaseLeaveFunc.Run();

	Tracen("");
	Tracen("## Network - Login Phase ##");
	Tracen("");

	m_strPhase = "Login";

	m_phaseProcessFunc.Set(this, &CPythonNetworkStream::LoginPhase);
	m_phaseLeaveFunc.Set(this, &CPythonNetworkStream::__LeaveLoginPhase);

	m_dwChangingPhaseTime = ELTimer_GetMSec();

	if (__DirectEnterMode_IsSet())
	{
		SendLoginPacketNew(m_stID.c_str(), m_stPassword.c_str());
		ClearLoginInfo();
		CAccountConnector& rkAccountConnector = CAccountConnector::Instance();
		rkAccountConnector.ClearLoginInfo();
	}
	else
	{
		SendLoginPacketNew(m_stID.c_str(), m_stPassword.c_str());
		ClearLoginInfo();
		CAccountConnector& rkAccountConnector = CAccountConnector::Instance();
		rkAccountConnector.ClearLoginInfo();

		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_LOGIN], "OnLoginStart", Py_BuildValue("()"));
#ifdef USE_LOADING_DLG_OPTIMIZATION
		CPlayerSettingsModule::Instance().joinLoadThread();
#endif // USE_LOADING_DLG_OPTIMIZATION

		__ClearSelectCharacterData();
	}

#ifdef USE_DISCORD_RPC_MODULE
	CDiscordManager::Instance().UpdatePresence(CDiscordManager::EDiscordPresenceStatus::Discord_Status_Closed, 0);
#endif // USE_DISCORD_RPC_MODULE
}

bool CPythonNetworkStream::__RecvEmpirePacket()
{
	TPacketGCEmpire kPacketEmpire;
	if (!Recv(sizeof(kPacketEmpire), &kPacketEmpire))
		return false;

	m_dwEmpireID = kPacketEmpire.bEmpire;
	return true;
}

bool CPythonNetworkStream::__RecvLoginSuccessPacket4()
{
	TPacketGCLoginSuccess4 kPacketLoginSuccess;

	if (!Recv(sizeof(kPacketLoginSuccess), &kPacketLoginSuccess))
		return false;

	for (int i = 0; i < PLAYER_PER_ACCOUNT4; ++i)
	{
		m_akSimplePlayerInfo[i] = kPacketLoginSuccess.akSimplePlayerInformation[i];
		m_adwGuildID[i] = kPacketLoginSuccess.guild_id[i];
		m_astrGuildName[i] = kPacketLoginSuccess.guild_name[i];
	}

	m_kMarkAuth.m_dwHandle = kPacketLoginSuccess.handle;
	m_kMarkAuth.m_dwRandomKey = kPacketLoginSuccess.random_key;

	if (__DirectEnterMode_IsSet())
	{
	}
	else
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_SELECT], "Refresh", Py_BuildValue("()"));
	}

	return true;
}

void CPythonNetworkStream::OnConnectFailure()
{
	if (__DirectEnterMode_IsSet())
	{
		ClosePhase();
	}
	else
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_LOGIN], "OnConnectFailure", Py_BuildValue("()"));
	}
}

bool CPythonNetworkStream::__RecvLoginFailurePacket()
{
	TPacketGCLoginFailure packet_failure;
	if (!Recv(sizeof(TPacketGCLoginFailure), &packet_failure))
		return false;

#ifdef ENABLE_BAN_REASON_UTILITY
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_LOGIN], "OnLoginFailure", Py_BuildValue("(sis)", packet_failure.szStatus, packet_failure.availDate, packet_failure.szBanWebLink));
#else
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_LOGIN], "OnLoginFailure", Py_BuildValue("(s)", packet_failure.szStatus));
#endif
#ifdef _DEBUG
	Tracef(" RecvLoginFailurePacket : [%s]\n", packet_failure.szStatus);
#endif
	return true;
}

#ifdef ENABLE_HWID_SECURITY_UTILITY
#include "HWIDManager.h"
#include "cryptopp/sha.h"
#include "cryptopp/filters.h"
#include "cryptopp/hex.h"
#include "cryptopp/base64.h"
#include <fstream>
#endif

bool CPythonNetworkStream::SendLoginPacketNew(const char* c_szName, const char* c_szPassword)
{
	TPacketCGLogin2 LoginPacket;
	LoginPacket.header = HEADER_CG_LOGIN2;
	LoginPacket.login_key = m_dwLoginKey;

	strncpy(LoginPacket.name, c_szName, sizeof(LoginPacket.name) - 1);
	LoginPacket.name[ID_MAX_NUM] = '\0';

	extern DWORD g_adwEncryptKey[4];
	extern DWORD g_adwDecryptKey[4];
	for (DWORD i = 0; i < 4; ++i)
		LoginPacket.adwClientKey[i] = g_adwEncryptKey[i];

	if (!Send(sizeof(LoginPacket), &LoginPacket))
	{
		Tracen("SendLogin Error");
		return false;
	}

	__SendInternalBuffer();

#ifndef ENABLE_IMPROVED_PACKET_ENCRYPTION
	SetSecurityMode(true, (const char*)g_adwEncryptKey, (const char*)g_adwDecryptKey);
#endif
	return true;
}