#include "stdafx.h"
#include "constants.h"
#include "config.h"
#include "input.h"
#include "desc_client.h"
#include "desc_manager.h"
#include "protocol.h"
#include "locale_service.h"
#include "db.h"

#include "utils.h"

bool FN_IS_VALID_LOGIN_STRING(const char* str)
{
	const char* tmp;

	if (!str || !*str)
		return false;

	if (strlen(str) < 2)
		return false;

	for (tmp = str; *tmp; ++tmp)
	{
		if (isdigit(*tmp) || isalpha(*tmp))
			continue;

		return false;
	}

	return true;
}

CInputAuth::CInputAuth()
{
}

void CInputAuth::Login(LPDESC d, const char* c_pData)
{
	TPacketCGLogin3* pinfo = (TPacketCGLogin3*)c_pData;

	if (!g_bAuthServer)
	{
		sys_err("CInputAuth class is not for game server. IP %s might be a hacker.",
			inet_ntoa(d->GetAddr().sin_addr));
		d->DelayedDisconnect(5);
		return;
	}

	char login[LOGIN_MAX_LEN + 1];
	trim_and_lower(pinfo->login, login, sizeof(login));

	char passwd[PASSWD_MAX_LEN + 1];
	strlcpy(passwd, pinfo->passwd, sizeof(passwd));

	sys_log(0, "InputAuth::Login : %s(%d) desc %p",
		login, strlen(login), get_pointer(d));

	// check login string
	if (false == FN_IS_VALID_LOGIN_STRING(login))
	{
		sys_log(0, "InputAuth::Login : IS_NOT_VALID_LOGIN_STRING(%s) desc %p",
			login, get_pointer(d));
		LoginFailure(d, "NOID");
		return;
	}

	if (g_bNoMoreClient)
	{
		TPacketGCLoginFailure failurePacket;

		failurePacket.header = HEADER_GC_LOGIN_FAILURE;
		strlcpy(failurePacket.szStatus, "SHUTDOWN", sizeof(failurePacket.szStatus));
#ifdef __BAN_REASON_UTILITY__
		failurePacket.availDate = 0;
		strlcpy(failurePacket.szBanWebLink, "", sizeof(failurePacket.szBanWebLink));
#endif // __BAN_REASON_UTILITY__

		d->Packet(&failurePacket, sizeof(failurePacket));
		return;
	}

	if (DESC_MANAGER::instance().FindByLoginName(login))
	{
		LoginFailure(d, "ALREADY");
		return;
	}

	DWORD dwKey = DESC_MANAGER::instance().CreateLoginKey(d);

	sys_log(0, "InputAuth::Login: login %s", login);

	TPacketCGLogin3* p = M2_NEW TPacketCGLogin3;
	thecore_memcpy(p, pinfo, sizeof(TPacketCGLogin3));

	char szPasswd[PASSWD_MAX_LEN * 2 + 1];
	DBManager::instance().EscapeString(szPasswd, sizeof(szPasswd), passwd, strlen(passwd));

	char szLogin[LOGIN_MAX_LEN * 2 + 1];
	DBManager::instance().EscapeString(szLogin, sizeof(szLogin), login, strlen(login));

#ifdef __WIN32__
	DBManager::instance().ReturnQuery(QID_AUTH_LOGIN, dwKey, p,
		"SELECT PASSWORD('%s'), password, social_id, id, status,"
#ifdef __BAN_REASON_UTILITY__
		"UNIX_TIMESTAMP(banavailDt),"
		"ban_weblink,"
#endif
#ifdef __HWID_SECURITY_UTILITY__
		"pin,"
		"cpu_id,"
		"hdd_model,"
		"machine_guid,"
		"mac_addr,"
		"hdd_serial,"
		"bios_id,"
		"password_whash,"
#endif
		"availDt - NOW() > 0,"
		"UNIX_TIMESTAMP(silver_expire),"
		"UNIX_TIMESTAMP(gold_expire),"
		"UNIX_TIMESTAMP(safebox_expire),"
		"UNIX_TIMESTAMP(autoloot_expire),"
		"UNIX_TIMESTAMP(fish_mind_expire),"
		"UNIX_TIMESTAMP(money_drop_rate_expire),"
		"UNIX_TIMESTAMP(create_time)"
		" FROM account WHERE login='%s'", szLogin);
#else
	DBManager::instance().ReturnQuery(QID_AUTH_LOGIN, dwKey, p,
		"SELECT PASSWORD('%s'), password, social_id, id, status,"
#ifdef __ACC_STATUS_CHECHKUP__
		"checkup_id,"
#endif // __ACC_STATUS_CHECHKUP__
#ifdef __BAN_REASON_UTILITY__
		"UNIX_TIMESTAMP(banavailDt),"
		"ban_weblink,"
#endif
#ifdef __HWID_SECURITY_UTILITY__
		"pin,"
		"cpu_id,"
		"hdd_model,"
		"machine_guid,"
		"mac_addr,"
		"hdd_serial,"
		"bios_id,"
		"password_whash,"
#endif
		"availDt - NOW() > 0,"
		"UNIX_TIMESTAMP(silver_expire),"
		"UNIX_TIMESTAMP(gold_expire),"
		"UNIX_TIMESTAMP(safebox_expire),"
		"UNIX_TIMESTAMP(autoloot_expire),"
		"UNIX_TIMESTAMP(fish_mind_expire),"
		"UNIX_TIMESTAMP(money_drop_rate_expire),"
		"UNIX_TIMESTAMP(create_time)"
		" FROM account WHERE login='%s'",
		szPasswd, szLogin);
#endif
}

int CInputAuth::Analyze(LPDESC d, BYTE bHeader, const char* c_pData)
{
	if (!g_bAuthServer)
	{
		sys_err("CInputAuth class is not for game server. IP %s might be a hacker.",
			inet_ntoa(d->GetAddr().sin_addr));
		d->DelayedDisconnect(5);
		return 0;
	}

	int iExtraLen = 0;

	if (test_server)
		sys_log(0, " InputAuth Analyze Header[%d] ", bHeader);

	switch (bHeader)
	{
	case HEADER_CG_PONG:
		Pong(d);
		break;

	case HEADER_CG_LOGIN3:
		Login(d, c_pData);
		break;

	case HEADER_CG_HANDSHAKE:
	case HEADER_CG_CHAT:
	case HEADER_CG_WHISPER:
		break;

	default:
		sys_err("This phase does not handle this header %d (0x%x)(phase: AUTH)", bHeader, bHeader);
		break;
	}

	return iExtraLen;
}


