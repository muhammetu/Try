/*
	Author : PACHI
	Date : 27.06.2020
	Information : The quieter you become the more you are able to hear -_-
*/
#include "stdafx.h"
#include "char.h"
#include "utils.h"
#include "log.h"
#include "db.h"
#include "config.h"
#include "desc.h"
#include "desc_manager.h"
#include "buffer_manager.h"
#include "packet.h"
#include "desc_client.h"
#include "p2p.h"
#include "hwid_manager.h"
#ifdef __BAN_REASON_UTILITY__
#include "ban_manager.h"
#endif

int CHwidManager::IsAcceptableLogin(const std::string& hwid_key, const std::string& hdd_key, const std::string& machine_guid, const std::string& bios_id)
{
	// std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("SELECT hwid, hdd_serial, machine_guid, bios_id FROM account.banned_computers"));
	// SQLResult* resSelect = pMsg->Get();

	// if (resSelect && resSelect->uiNumRows > 0)
	// {
	// 	MYSQL_ROW row;
	// 	while ((row = mysql_fetch_row(resSelect->pSQLResult)))
	// 	{
	// 		if (!strcmp(row[0], hwid_key.c_str()))
	// 			return HW_LOGIN_HWID_BANNED;

	// 		if (!strcmp(row[1], hdd_key.c_str()))
	// 			return HW_LOGIN_HDD_BANNED;

	// 		if (!strcmp(row[2], machine_guid.c_str()))
	// 			return HW_LOGIN_MACHINE_BANNED;

	// 		if (!strcmp(row[3], bios_id.c_str()))
	// 			return HW_LOGIN_BIOS_BANNED;
	// 	}
	// }

	if (!IsAcceptableHwid(hwid_key))
		return HW_LOGIN_HWID_BANNED;

	if (!IsAcceptableHdd(hdd_key))
		return HW_LOGIN_HDD_BANNED;

	if (!IsAcceptableMachine(machine_guid))
		return HW_LOGIN_MACHINE_BANNED;

	if (!IsAcceptableBios(bios_id))
		return HW_LOGIN_BIOS_BANNED;

	return HW_LOGIN_OK;
}

int CHwidManager::ReloadBanData()
{
	std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("SELECT hwid, hdd_serial, machine_guid, bios_id FROM account.banned_computers"));
	SQLResult* resSelect = pMsg->Get();

	if (!resSelect || resSelect->uiNumRows < 0)
		return -1;

	m_bannedHwids.clear();
	m_bannedHdd.clear();
	m_bannedMachine.clear();
	m_bannedBios.clear();

	int ret = 0;

	MYSQL_ROW row;
	while ((row = mysql_fetch_row(resSelect->pSQLResult)))
	{
		m_bannedHwids.insert(row[0]);
		m_bannedHdd.insert(row[1]);
		m_bannedMachine.insert(row[2]);
		m_bannedBios.insert(row[3]);

		ret++;
	}

	sys_log(0, "ReloadBanData:: m_bannedHwids.size: %d m_bannedHdd.size: %d m_bannedMachine.size: %d m_bannedBios.size: %d ", m_bannedHwids.size(), m_bannedHdd.size(), m_bannedMachine.size(), m_bannedBios.size());

	return ret;
}

bool CHwidManager::BanByAccountData(std::string account)
{
	std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("SELECT os_version,hdd_serial,machine_guid,bios_id FROM account.account WHERE login='%s';", account.c_str()));
	SQLResult* resSelect = pMsg->Get();

	if (resSelect && resSelect->uiNumRows > 0)
	{
		MYSQL_ROW row = mysql_fetch_row(resSelect->pSQLResult);
		bool success = Ban(row[0], row[1], row[2], row[3]);
		return success;
	}

	return false;
}

bool CHwidManager::BanByAccountID(DWORD account_id)
{
	std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("SELECT login FROM account.account WHERE id=%d", account_id));
	SQLResult* resSelect = pMsg->Get();

	if (resSelect && resSelect->uiNumRows > 0)
	{
		MYSQL_ROW row = mysql_fetch_row(resSelect->pSQLResult);
		bool success = BanByAccountData(row[0]);
		return success;
	}

	return false;
}

bool CHwidManager::BanByPlayerData(std::string player)
{
	std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("SELECT account_id FROM player.player WHERE name='%s';", player.c_str()));
	SQLResult* resSelect = pMsg->Get();

	if (resSelect && resSelect->uiNumRows > 0)
	{
		MYSQL_ROW row = mysql_fetch_row(resSelect->pSQLResult);
		bool success = BanByAccountID(atoi(row[0]));
		return success;
	}

	return false;
}

bool CHwidManager::Ban(std::string hwid_key, std::string hdd_serial, std::string machine_guid, std::string bios_id)
{
	std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("INSERT INTO `account`.`banned_computers` (`hwid`,`hdd_serial`,`machine_guid`,`bios_id`) VALUES('%s','Devre Disi...','%s','Devre Disi...')", hwid_key.c_str(), machine_guid.c_str()));
	SQLResult* resInsert = pMsg->Get();

	if (resInsert && resInsert->uiAffectedRows > 0)
		return true;

	return false;
}

CHwidManager::CHwidManager()
{
}

CHwidManager::~CHwidManager()
{
	Destroy();
}

void CHwidManager::Destroy()
{
}
