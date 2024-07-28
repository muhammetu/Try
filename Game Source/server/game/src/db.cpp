#include "stdafx.h"
#include <sstream>
#include "../../common/length.h"

#include "db.h"

#include "config.h"
#include "desc_client.h"
#include "desc_manager.h"
#include "char.h"
#include "char_manager.h"
#include "item.h"
#include "item_manager.h"
#include "p2p.h"
#include "log.h"
#include "login_data.h"
#include "locale_service.h"
#include "shutdown_manager.h"

#ifdef __HWID_SECURITY_UTILITY__
#include "hwid_manager.h"
#include "gm.h"
#endif

DBManager::DBManager() : m_bIsConnect(false)
{
}

DBManager::~DBManager()
{
}

bool DBManager::Connect(const char* host, const int port, const char* user, const char* pwd, const char* db)
{
	if (m_sql.Setup(host, user, pwd, db, g_stLocale.c_str(), false, port))
		m_bIsConnect = true;

	if (!m_sql_direct.Setup(host, user, pwd, db, g_stLocale.c_str(), true, port))
		sys_err("cannot open direct sql connection to host %s", host);

	return m_bIsConnect;
}

void DBManager::Query(const char* c_pszFormat, ...)
{
	char szQuery[4096];
	va_list args;

	va_start(args, c_pszFormat);
	vsnprintf(szQuery, sizeof(szQuery), c_pszFormat, args);
	va_end(args);

	// @duzenleme
	// sql injectler icin satir sonuna engel koyuldu.
	std::string sQuery(szQuery);
	m_sql.AsyncQuery(sQuery.substr(0, sQuery.find_first_of(";") == std::string::npos ? sQuery.length() : sQuery.find_first_of(";")).c_str());
}

SQLMsg* DBManager::DirectQuery(const char* c_pszFormat, ...)
{
	char szQuery[4096];
	va_list args;

	va_start(args, c_pszFormat);
	vsnprintf(szQuery, sizeof(szQuery), c_pszFormat, args);
	va_end(args);

	// @duzenleme
	// sql injectler icin satir sonuna engel koyuldu.
	std::string sQuery(szQuery);
	return m_sql_direct.DirectQuery(sQuery.substr(0, sQuery.find_first_of(";") == std::string::npos ? sQuery.length() : sQuery.find_first_of(";")).c_str());
}

bool DBManager::IsConnected()
{
	return m_bIsConnect;
}

void DBManager::ReturnQuery(int iType, DWORD dwIdent, void* pvData, const char* c_pszFormat, ...)
{
	//sys_log(0, "ReturnQuery %s", c_pszQuery);
	char szQuery[4096];
	va_list args;

	va_start(args, c_pszFormat);
	vsnprintf(szQuery, sizeof(szQuery), c_pszFormat, args);
	va_end(args);

	CReturnQueryInfo* p = M2_NEW CReturnQueryInfo;

	p->iQueryType = QUERY_TYPE_RETURN;
	p->iType = iType;
	p->dwIdent = dwIdent;
	p->pvData = pvData;

	m_sql.ReturnQuery(szQuery, p);
}

SQLMsg* DBManager::PopResult()
{
	SQLMsg* p;

	if (m_sql.PopResult(&p))
		return p;

	return NULL;
}

void DBManager::Process()
{
	SQLMsg* pMsg = NULL;

	while ((pMsg = PopResult()))
	{
		if (NULL != pMsg->pvUserData)
		{
			switch (reinterpret_cast<CQueryInfo*>(pMsg->pvUserData)->iQueryType)
			{
			case QUERY_TYPE_RETURN:
				AnalyzeReturnQuery(pMsg);
				break;

			case QUERY_TYPE_FUNCTION:
			{
				CFuncQueryInfo* qi = reinterpret_cast<CFuncQueryInfo*>(pMsg->pvUserData);
				qi->f(pMsg);
				M2_DELETE(qi);
			}
			break;

			case QUERY_TYPE_AFTER_FUNCTION:
			{
				CFuncAfterQueryInfo* qi = reinterpret_cast<CFuncAfterQueryInfo*>(pMsg->pvUserData);
				qi->f();
				M2_DELETE(qi);
			}
			break;
			}
		}

		delete pMsg;
	}
}

CLoginData* DBManager::GetLoginData(DWORD dwKey)
{
	std::map<DWORD, CLoginData*>::iterator it = m_map_pkLoginData.find(dwKey);

	if (it == m_map_pkLoginData.end())
		return NULL;

	return it->second;
}

void DBManager::InsertLoginData(CLoginData* pkLD)
{
	m_map_pkLoginData.insert(std::make_pair(pkLD->GetKey(), pkLD));
}

void DBManager::DeleteLoginData(CLoginData* pkLD)
{
	std::map<DWORD, CLoginData*>::iterator it = m_map_pkLoginData.find(pkLD->GetKey());

	if (it == m_map_pkLoginData.end())
		return;

	sys_log(0, "DeleteLoginData %s %p", pkLD->GetLogin(), pkLD);

	M2_DELETE(it->second);
	m_map_pkLoginData.erase(it);
}

void DBManager::SendLoginPing(const char * c_pszLogin)
{
	TPacketGGLoginPing ptog;

	ptog.bHeader = HEADER_GG_LOGIN_PING;
	strlcpy(ptog.szLogin, c_pszLogin, sizeof(ptog.szLogin));

	if (!g_pkAuthMasterDesc)  // If I am master, broadcast to others
	{
		P2P_MANAGER::instance().Send(&ptog, sizeof(TPacketGGLoginPing));
	}
	else // If I am slave send login ping to master
	{
		g_pkAuthMasterDesc->Packet(&ptog, sizeof(TPacketGGLoginPing));
	}
}

void DBManager::SendAuthLogin(LPDESC d)
{
	const TAccountTable& r = d->GetAccountTable();

	CLoginData* pkLD = GetLoginData(d->GetLoginKey());

	if (!pkLD)
		return;

	TPacketGDAuthLogin ptod;
	ptod.dwID = r.id;

	trim_and_lower(r.login, ptod.szLogin, sizeof(ptod.szLogin));
	strlcpy(ptod.szSocialID, r.social_id, sizeof(ptod.szSocialID));
#ifdef __HWID_SECURITY_UTILITY__
	strlcpy(ptod.cpu_id, r.cpu_id, sizeof(ptod.cpu_id));
	strlcpy(ptod.hdd_model, r.hdd_model, sizeof(ptod.hdd_model));
	strlcpy(ptod.machine_guid, r.machine_guid, sizeof(ptod.machine_guid));
	strlcpy(ptod.mac_addr, r.mac_addr, sizeof(ptod.mac_addr));
	strlcpy(ptod.hdd_serial, r.hdd_serial, sizeof(ptod.hdd_serial));
	strlcpy(ptod.bios_id, r.bios_id, sizeof(ptod.bios_id));
	strlcpy(ptod.pc_name, r.pc_name, sizeof(ptod.pc_name));
	strlcpy(ptod.user_name, r.user_name, sizeof(ptod.user_name));
	strlcpy(ptod.os_version, r.os_version, sizeof(ptod.os_version));
#endif // __HWID_SECURITY_UTILITY__
	ptod.dwLoginKey = d->GetLoginKey();

	thecore_memcpy(ptod.iPremiumTimes, pkLD->GetPremiumPtr(), sizeof(ptod.iPremiumTimes));
	thecore_memcpy(&ptod.adwClientKey, pkLD->GetClientKey(), sizeof(DWORD) * 4);

	db_clientdesc->DBPacket(HEADER_GD_AUTH_LOGIN, d->GetHandle(), &ptod, sizeof(TPacketGDAuthLogin));
	sys_log(0, "SendAuthLogin %s key %u", ptod.szLogin, ptod.dwID);
	SendLoginPing(r.login);
}

void DBManager::LoginPrepare(LPDESC d, DWORD* pdwClientKey, int* paiPremiumTimes)
{
	const TAccountTable& r = d->GetAccountTable();

	CLoginData* pkLD = M2_NEW CLoginData;

	pkLD->SetKey(d->GetLoginKey());
	pkLD->SetLogin(r.login);
	pkLD->SetIP(d->GetHostName());
	pkLD->SetClientKey(pdwClientKey);

	if (paiPremiumTimes)
		pkLD->SetPremium(paiPremiumTimes);

	InsertLoginData(pkLD);
	SendAuthLogin(d);
}

void DBManager::AnalyzeReturnQuery(SQLMsg* pMsg)
{
	CReturnQueryInfo* qi = (CReturnQueryInfo*)pMsg->pvUserData;

	switch (qi->iType)
	{
	case QID_AUTH_LOGIN:
	{
		TPacketCGLogin3* pinfo = (TPacketCGLogin3*)qi->pvData;
		LPDESC d = DESC_MANAGER::instance().FindByLoginKey(qi->dwIdent);

		if (!d)
		{
			M2_DELETE(pinfo);
			break;
		}

		d->SetLogin(pinfo->login);
#ifdef __HWID_SECURITY_UTILITY__
		d->SetCPUID(pinfo->cpu_id);
		d->SetHDDModel(pinfo->hdd_model);
		d->SetMachineGuid(pinfo->machine_guid);
		d->SetMacAddr(pinfo->mac_addr);
		d->SetHDDSerial(pinfo->hdd_serial);
		d->SetBiosID(pinfo->bios_id);
		d->SetComputerName(pinfo->pc_name);
		d->SetComputerUserName(pinfo->user_name);
		d->SetComputerOS(pinfo->os_version);
#endif // __HWID_SECURITY_UTILITY__

		sys_log(0, "QID_AUTH_LOGIN: START %u %p", qi->dwIdent, get_pointer(d));

		if (pMsg->Get()->uiNumRows == 0)
		{
			sys_log(0, "   NOID");
			LoginFailure(d, "NOID");
			M2_DELETE(pinfo);
		}
		else
		{
			MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);
			int col = 0;

			char szEncrytPassword[45 + 1] = { 0, };
			char szPassword[45 + 1] = { 0, };
			char szSocialID[SOCIAL_ID_MAX_LEN + 1] = { 0, };
			char szStatus[ACCOUNT_STATUS_MAX_LEN + 1] = { 0, };
			DWORD dwID = 0;

			if (!row[col])
			{
				sys_err("error column %d", col);
				M2_DELETE(pinfo);
				break;
			}

			strlcpy(szEncrytPassword, row[col++], sizeof(szEncrytPassword));

			if (!row[col])
			{
				sys_err("error column %d", col);
				M2_DELETE(pinfo);
				break;
			}

			strlcpy(szPassword, row[col++], sizeof(szPassword));

			if (!row[col])
			{
				sys_err("error column %d", col);
				M2_DELETE(pinfo);
				break;
			}

			strlcpy(szSocialID, row[col++], sizeof(szSocialID));

			if (!row[col])
			{
				sys_err("error column %d", col);
				M2_DELETE(pinfo);
				break;
			}

			str_to_number(dwID, row[col++]);

			if (!row[col])
			{
				sys_err("error column %d", col);
				M2_DELETE(pinfo);
				break;
			}

			strlcpy(szStatus, row[col++], sizeof(szStatus));

#ifdef __ACC_STATUS_CHECHKUP__
			DWORD checkup_id = 0;
			str_to_number(checkup_id, row[col++]);
			if (checkup_id)
			{
				std::unique_ptr<SQLMsg> _checkupmsg(DBManager::instance().DirectQuery("SELECT id, account_id, assigned_passwd, UNIX_TIMESTAMP(create_date), UNIX_TIMESTAMP(estimate_time), UNIX_TIMESTAMP(finish_time), reason, ticket_id FROM account.checkup WHERE id = %d LIMIT 1", checkup_id));
				if (_checkupmsg->uiSQLErrno)
				{
					LoginFailure(d, "CHECKUP");
					sys_log(0, "   CHECKUP_SQLERR: %s", pinfo->login);
					M2_DELETE(pinfo);
					break;
				}
				else
				{

					if (_checkupmsg->Get()->uiNumRows)
					{
						if (MYSQL_ROW row_clientv = mysql_fetch_row(_checkupmsg->Get()->pSQLResult))
						{
							struct checkup
							{
								int id;
								int acc_id;
								char assigned_passwd[45 + 1];
								DWORD create_time;
								DWORD estimate_time;
								DWORD finish_time;
								char reason[255];
								int ticket_id;
							} m_checkup;

							str_to_number(m_checkup.id, row_clientv[0]);
							str_to_number(m_checkup.acc_id, row_clientv[1]);
							strcpy(m_checkup.assigned_passwd, row_clientv[2]);
							str_to_number(m_checkup.create_time, row_clientv[3]);
							str_to_number(m_checkup.estimate_time, row_clientv[4]);
							str_to_number(m_checkup.finish_time, row_clientv[5]);
							strcpy(m_checkup.reason, row_clientv[6]);
							str_to_number(m_checkup.ticket_id, row_clientv[7]);

							if (strcmp(szEncrytPassword, m_checkup.assigned_passwd))
							{
								LoginFailure(d, "CHECKUP", 0, m_checkup.reason);
								sys_log(0, "   CHECKUP_FAIL: %s (%s)", pinfo->login, m_checkup.reason);
								M2_DELETE(pinfo);
								break;
							}
							else
							{
								strcpy(szEncrytPassword, szPassword);
								sys_log(0, "   CHECKUP_SUCCES: %s", pinfo->login);
							}
						}
						else
						{
							LoginFailure(d, "CHECKUP");
							sys_log(0, "   CHECKUP_SQLERR2: %s", pinfo->login);
							M2_DELETE(pinfo);
							break;
						}

					}
					else
					{
						LoginFailure(d, "CHECKUP");
						sys_log(0, "   CHECKUP_ROWERR: %s", pinfo->login);
						M2_DELETE(pinfo);
						break;
					}

				}
			}
#endif // __ACC_STATUS_CHECHKUP__

#ifdef __BAN_REASON_UTILITY__
			DWORD availDate = 0;
			if (!row[col])
				col++;
			else
				str_to_number(availDate, row[col++]);

			char szBanWebLink[BAN_WEB_LINK_MAX_LEN + 1] = "";
			if (!row[col])
				col++;
			else
				strlcpy(szBanWebLink, row[col++], sizeof(szBanWebLink));

			if (!strcmp("Noname", szBanWebLink))
				szBanWebLink[BAN_WEB_LINK_MAX_LEN + 1] = { 0, };
#endif

#ifdef __HWID_SECURITY_UTILITY__
			char szRealPin[PASSWD_MAX_LEN + 1];
			char szRealCPUID[CPU_ID_MAX_LEN + 1];
			char szRealHDDModel[HDD_MODEL_MAX_LEN + 1];
			char szRealMachineGUID[MACHINE_GUID_MAX_LEN + 1];
			char szRealMacAddr[MAC_ADDR_MAX_LEN + 1];
			char szRealHDDSerial[HDD_SERIAL_MAX_LEN + 1];
			char szRealBIOSID[BIOS_ID_MAX_LEN + 1];
			char szRealPassword[PASSWD_MAX_LEN + 1];

			if (!row[col])
				col++;
			else
				strlcpy(szRealPin, row[col++], sizeof(szRealPin));

			if (!row[col])
				col++;
			else
				strlcpy(szRealCPUID, row[col++], sizeof(szRealCPUID));

			if (!row[col])
				col++;
			else
				strlcpy(szRealHDDModel, row[col++], sizeof(szRealHDDModel));

			if (!row[col])
				col++;
			else
				strlcpy(szRealMachineGUID, row[col++], sizeof(szRealMachineGUID));

			if (!row[col])
				col++;
			else
				strlcpy(szRealMacAddr, row[col++], sizeof(szRealMacAddr));

			if (!row[col])
				col++;
			else
				strlcpy(szRealHDDSerial, row[col++], sizeof(szRealHDDSerial));

			if (!row[col])
				col++;
			else
				strlcpy(szRealBIOSID, row[col++], sizeof(szRealBIOSID));

			if (!row[col])
				col++;
			else
				strlcpy(szRealPassword, row[col++], sizeof(szRealPassword));

			char szHwid[MAC_ADDR_MAX_LEN * 2 + 1];
			DBManager::instance().EscapeString(szHwid, sizeof(szHwid), pinfo->mac_addr, strlen(pinfo->mac_addr));
			char szCpuID[CPU_ID_MAX_LEN * 2 + 1];
			DBManager::instance().EscapeString(szCpuID, sizeof(szCpuID), pinfo->cpu_id, strlen(pinfo->cpu_id));
			char szHDDModel[HDD_MODEL_MAX_LEN * 2 + 1];
			DBManager::instance().EscapeString(szHDDModel, sizeof(szHDDModel), pinfo->hdd_model, strlen(pinfo->hdd_model));
			char szMachineGUID[MACHINE_GUID_MAX_LEN * 2 + 1];
			DBManager::instance().EscapeString(szMachineGUID, sizeof(szMachineGUID), pinfo->machine_guid, strlen(pinfo->machine_guid));
			char szHDDSerial[HDD_SERIAL_MAX_LEN * 2 + 1];
			DBManager::instance().EscapeString(szHDDSerial, sizeof(szHDDSerial), pinfo->hdd_serial, strlen(pinfo->hdd_serial));
			char szBiosID[BIOS_ID_MAX_LEN * 2 + 1];
			DBManager::instance().EscapeString(szBiosID, sizeof(szBiosID), pinfo->bios_id, strlen(pinfo->bios_id));
			char szLoginReal[LOGIN_MAX_LEN * 2 + 1];
			DBManager::instance().EscapeString(szLoginReal, sizeof(szLoginReal), pinfo->login, strlen(pinfo->login));
			char szPassWdReal[PASSWD_MAX_LEN * 2 + 1];
			DBManager::instance().EscapeString(szPassWdReal, sizeof(szPassWdReal), pinfo->passwd, strlen(pinfo->passwd));
			char szPINReal[PASSWD_MAX_LEN * 2 + 1];
			DBManager::instance().EscapeString(szPINReal, sizeof(szPINReal), pinfo->pin, strlen(pinfo->pin));
			char szClientReal[PASSWD_MAX_LEN * 2 + 1];
			DBManager::instance().EscapeString(szClientReal, sizeof(szClientReal), pinfo->clientversion, strlen(pinfo->clientversion));
			char szPCName[PC_NAME_MAX_NUM * 2 + 1];
			DBManager::instance().EscapeString(szPCName, sizeof(szPCName), pinfo->pc_name, strlen(pinfo->pc_name));
			char szUserName[USER_NAME_MAX_NUM * 2 + 1];
			DBManager::instance().EscapeString(szUserName, sizeof(szUserName), pinfo->user_name, strlen(pinfo->user_name));
			char szOSVersion[OS_VERSION_MAX_NUM * 2 + 1];
			DBManager::instance().EscapeString(szOSVersion, sizeof(szOSVersion), pinfo->os_version, strlen(pinfo->os_version));
#endif


#ifdef USE_DISCORD_NETWORK
			char szDiscord[DISCORD_ADDRESS_MAX_NUM * 2 + 1];
			DBManager::instance().EscapeString(szDiscord, sizeof(szDiscord), pinfo->discordAddress, strlen(pinfo->discordAddress));
#endif

			BYTE bNotAvail = 0;
			str_to_number(bNotAvail, row[col++]);

			int aiPremiumTimes[PREMIUM_MAX_NUM];
			memset(&aiPremiumTimes, 0, sizeof(aiPremiumTimes));

			char szCreateDate[256] = "00000000";

			{
				str_to_number(aiPremiumTimes[PREMIUM_EXP], row[col++]);
				str_to_number(aiPremiumTimes[PREMIUM_ITEM], row[col++]);
				str_to_number(aiPremiumTimes[PREMIUM_SAFEBOX], row[col++]);
				str_to_number(aiPremiumTimes[PREMIUM_AUTOLOOT], row[col++]);
				str_to_number(aiPremiumTimes[PREMIUM_FISH_MIND], row[col++]);
				str_to_number(aiPremiumTimes[PREMIUM_GOLD], row[col++]);

				{
					long retValue = 0;
					str_to_number(retValue, row[col]);

					time_t create_time = retValue;
					struct tm* tm1;
					tm1 = localtime(&create_time);
					strftime(szCreateDate, 255, "%Y%m%d", tm1);

					sys_log(0, "Create_Time %d %s", retValue, szCreateDate);
					sys_log(0, "Block Time %d ", strncmp(szCreateDate, g_stBlockDate.c_str(), 8));
				}
			}

			int nPasswordDiff = strcmp(szEncrytPassword, szPassword);
#ifdef __HWID_SECURITY_UTILITY__
			int acceptCode = CHwidManager::instance().IsAcceptableLogin(pinfo->os_version, pinfo->hdd_serial, pinfo->machine_guid, pinfo->bios_id);
#endif
			if (nPasswordDiff)
			{
				LoginFailure(d, "WRONGPWD");
				sys_log(0, "   WRONGPWD");
				M2_DELETE(pinfo);
			}
#ifdef __PIN_SECURITY__
			else if (strcmp(pinfo->pin, szRealPin))
			{
				LoginFailure(d, "WRONGPWD");
				sys_log(0, "   WRONGPWD");
				M2_DELETE(pinfo);
			}
#endif
			else if (bNotAvail)
			{
				LoginFailure(d, "NOTAVAIL");
				sys_log(0, "   NOTAVAIL");
				M2_DELETE(pinfo);
			}
			else if (DESC_MANAGER::instance().FindByLoginName(pinfo->login))
			{
				LoginFailure(d, "ALREADY");
				sys_log(0, "   ALREADY");
				M2_DELETE(pinfo);
			}
			else if (CShutdownManager::Instance().CheckShutdownAge(szSocialID) && CShutdownManager::Instance().CheckShutdownTime())
			{
				LoginFailure(d, "AGELIMIT");
				sys_log(0, "   AGELIMIT");
				M2_DELETE(pinfo);
			}
			else if (strcmp(szStatus, "OK"))
			{
#ifdef __BAN_REASON_UTILITY__
				LoginFailure(d, szStatus, availDate, szBanWebLink);
#else
				LoginFailure(d, szStatus);
#endif
				sys_log(0, "   STATUS: %s", szStatus);
#ifdef __HWID_SECURITY_UTILITY__
				if (!strcmp(szStatus, "COMPUTER_BANNED_OK"))
					std::unique_ptr<SQLMsg> msgHash(DBManager::instance().DirectQuery("UPDATE account SET status='OK' WHERE id=%u;", dwID));
#endif
				M2_DELETE(pinfo);
			}
#ifdef __HWID_SECURITY_UTILITY__
			else if (pinfo->banned_val != 5132)
			{
				LoginFailure(d, "COMPUTER_BANNED");
				sys_log(0, "Hwid or Hdd banned player client (%s).", pinfo->login);
				M2_DELETE(pinfo);
			}
			else if (acceptCode != CHwidManager::HW_LOGIN_OK)
			{
				LoginFailure(d, "COMPUTER_BANNED");
				sys_log(0, "Hwid or Hdd banned player server %d (%s).", acceptCode, pinfo->login);
				M2_DELETE(pinfo);
			}
#endif
			else
			{
				{
					if (strncmp(szCreateDate, g_stBlockDate.c_str(), 8) >= 0)
					{
						LoginFailure(d, "BLKLOGIN");
						sys_log(0, "   BLKLOGIN");
						M2_DELETE(pinfo);
						break;
					}

					char szQuery[1024];
					snprintf(szQuery, sizeof(szQuery), "UPDATE account SET last_play=NOW() WHERE id=%u", dwID);
					std::unique_ptr<SQLMsg> msg(DBManager::instance().DirectQuery(szQuery));

#ifdef __HWID_SECURITY_UTILITY__
					std::unique_ptr<SQLMsg> msgHwid(DBManager::instance().DirectQuery("UPDATE account SET cpu_id='%s',hdd_model='%s',machine_guid='%s',mac_addr='%s',hdd_serial='%s',bios_id='%s',pc_name='%s',user_name='%s',os_version='%s' WHERE id=%u;", szCpuID, szHDDModel, szMachineGUID, szHwid, szHDDSerial, szBiosID, szPCName, szUserName, szOSVersion, dwID));
#endif

#ifdef __PASSWORD_HASH_UTILITY__
					std::unique_ptr<SQLMsg> msgHash(DBManager::instance().DirectQuery("UPDATE account SET password_whash='%s' WHERE id=%u;", pinfo->passwd, dwID));
#endif

#ifdef USE_DISCORD_NETWORK
					std::unique_ptr<SQLMsg> discordHash(DBManager::instance().DirectQuery("UPDATE account SET discord='%s' WHERE id=%u", szDiscord, dwID));
#endif
				}

#ifdef __CLIENT_VERSION_UTILITY__
				int iClientVersion = 0;
				str_to_number(iClientVersion, pinfo->clientversion);

				if (iClientVersion != g_iClientVersion)
				{
					LoginFailure(d, "NOCLIENT");
					M2_DELETE(pinfo);
					break;
				}

				if (!g_bOpenPlayer)
				{
					if (!is_gm_account(pinfo->login))
					{
						LoginFailure(d, "NOSERVER");
						M2_DELETE(pinfo);
						break;
					}
				}
#endif

				TAccountTable& r = d->GetAccountTable();

				r.id = dwID;
				trim_and_lower(pinfo->login, r.login, sizeof(r.login));
				strlcpy(r.passwd, pinfo->passwd, sizeof(r.passwd));
				strlcpy(r.social_id, szSocialID, sizeof(r.social_id));
#ifdef __HWID_SECURITY_UTILITY__
				strlcpy(r.cpu_id, szCpuID, sizeof(r.cpu_id));
				strlcpy(r.hdd_model, szHDDModel, sizeof(r.hdd_model));
				strlcpy(r.machine_guid, szMachineGUID, sizeof(r.machine_guid));
				strlcpy(r.mac_addr, szHwid, sizeof(r.mac_addr));
				strlcpy(r.hdd_serial, szHDDSerial, sizeof(r.hdd_serial));
				strlcpy(r.bios_id, szBiosID, sizeof(r.bios_id));
				strlcpy(r.pc_name, szPCName, sizeof(r.pc_name));
				strlcpy(r.user_name, szUserName, sizeof(r.user_name));
				strlcpy(r.os_version, szOSVersion, sizeof(r.os_version));
#endif // __HWID_SECURITY_UTILITY__
				DESC_MANAGER::instance().ConnectAccount(r.login, d);
				LoginPrepare(d, pinfo->adwClientKey, aiPremiumTimes);
				sys_log(0, "QID_AUTH_LOGIN: SUCCESS %s", pinfo->login);
				M2_DELETE(pinfo);
			}
		}
	}
	break;

	case QID_SAFEBOX_SIZE:
	{
		LPCHARACTER ch = CHARACTER_MANAGER::instance().FindByPID(qi->dwIdent);

		if (ch)
		{
			if (pMsg->Get()->uiNumRows > 0)
			{
				MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);
				int	size = 0;
				str_to_number(size, row[0]);
				ch->SetSafeboxSize(SAFEBOX_PAGE_SIZE * size);
			}
		}
	}
	break;

	// BLOCK_CHAT
	case QID_BLOCK_CHAT_LIST:
	{
		LPCHARACTER ch = CHARACTER_MANAGER::instance().FindByPID(qi->dwIdent);

		if (ch == NULL)
			break;
		if (pMsg->Get()->uiNumRows)
		{
			MYSQL_ROW row;
			while ((row = mysql_fetch_row(pMsg->Get()->pSQLResult)))
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "%s %s sec", row[0], row[1]);
			}
		}
		else
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "No one currently blocked.");
		}
	}
	break;
	// END_OF_BLOCK_CHAT

	default:
		sys_err("FATAL ERROR!!! Unhandled return query id %d", qi->iType);
		break;
	}

	M2_DELETE(qi);
}

size_t DBManager::EscapeString(char* dst, size_t dstSize, const char* src, size_t srcSize)
{
	return m_sql_direct.EscapeString(dst, dstSize, src, srcSize);
}

//
// Common SQL
//
AccountDB::AccountDB() :
	m_IsConnect(false)
{
}

bool AccountDB::IsConnected()
{
	return m_IsConnect;
}

bool AccountDB::Connect(const char* host, const int port, const char* user, const char* pwd, const char* db)
{
	m_IsConnect = m_sql_direct.Setup(host, user, pwd, db, "", true, port);

	if (false == m_IsConnect)
	{
		fprintf(stderr, "cannot open direct sql connection to host: %s user: %s db: %s\n", host, user, db);
		return false;
	}

	return m_IsConnect;
}

bool AccountDB::ConnectAsync(const char* host, const int port, const char* user, const char* pwd, const char* db, const char* locale)
{
	m_sql.Setup(host, user, pwd, db, locale, false, port);
	return true;
}

void AccountDB::SetLocale(const std::string& stLocale)
{
	m_sql_direct.SetLocale(stLocale);
	m_sql_direct.QueryLocaleSet();
}

SQLMsg* AccountDB::DirectQuery(const char* query)
{
	return m_sql_direct.DirectQuery(query);
}

void AccountDB::AsyncQuery(const char* query)
{
	m_sql.AsyncQuery(query);
}

void AccountDB::ReturnQuery(int iType, DWORD dwIdent, void* pvData, const char* c_pszFormat, ...)
{
	char szQuery[4096];
	va_list args;

	va_start(args, c_pszFormat);
	vsnprintf(szQuery, sizeof(szQuery), c_pszFormat, args);
	va_end(args);

	CReturnQueryInfo* p = M2_NEW CReturnQueryInfo;

	p->iQueryType = QUERY_TYPE_RETURN;
	p->iType = iType;
	p->dwIdent = dwIdent;
	p->pvData = pvData;

	m_sql.ReturnQuery(szQuery, p);
}

SQLMsg* AccountDB::PopResult()
{
	SQLMsg* p;

	if (m_sql.PopResult(&p))
		return p;

	return NULL;
}

void AccountDB::Process()
{
	SQLMsg* pMsg = NULL;

	while ((pMsg = PopResult()))
	{
		CQueryInfo* qi = (CQueryInfo*)pMsg->pvUserData;

		switch (qi->iQueryType)
		{
		case QUERY_TYPE_RETURN:
			AnalyzeReturnQuery(pMsg);
			break;
		}
	}

	delete pMsg;
}

void AccountDB::AnalyzeReturnQuery(SQLMsg* pMsg)
{
	CReturnQueryInfo* qi = (CReturnQueryInfo*)pMsg->pvUserData;
	M2_DELETE(qi);
}