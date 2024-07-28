/*
	Author : PACHI
	Date : 02.08.2020
	Information : Player ban system
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
#include "char_manager.h"
#include "ban_manager.h"

bool CBanManager::PrepareBan(BYTE bBanType, BYTE bStrType, std::string selectedStr, DWORD availDt, std::string banReason, std::string banEvidence)
{
	int realAvailDt = availDt == E_AVAIL_DT_PERMA ? 99 : availDt;

	switch (bBanType)
	{
		case E_BAN_TYPE_ACCOUNT:
		{
			switch (bStrType)
			{
			case E_STR_TYPE_ACCOUNT:
			{
				bool result = BanByAccountName(selectedStr, realAvailDt, banReason, banEvidence);
				return result;
			}
			break;

			case E_STR_TYPE_PLAYER:
			{
				bool result = BanByPlayerName(selectedStr, realAvailDt, banReason, banEvidence);
				return result;
			}
			break;
			}
		}
		break;

		case E_BAN_TYPE_CHAT:
		{
			bool result = ChatBanByPlayerName(selectedStr, realAvailDt);
			return result;
		}
		break;
	}

	return false;
}

std::string CBanManager::GetAccountNameByAccountID(DWORD dwAccountID)
{
	std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("SELECT login FROM account.account WHERE id=%d;", dwAccountID));
	if (pMsg->Get()->uiNumRows == 0)
	{
		MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);
		if (row != NULL)
			return std::string(row[0]);
	}

	return "";
}

bool CBanManager::BanByAccountName(std::string account_name, DWORD availDt, std::string banReason, std::string banEvidence)
{
	availDt = get_global_time() + availDt;
	std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("UPDATE `account`.`account` SET `status`='%s',`banavailDt`=FROM_UNIXTIME(%d),`ban_weblink`='%s' WHERE login='%s';", banReason.c_str(), availDt, banEvidence.c_str(), account_name.c_str()));
	SQLResult* resSelect = pMsg->Get();

	if (resSelect)
	{
		LPDESC d = DESC_MANAGER::instance().FindByLoginName(account_name);
		if (d)
		{
			d->ChatPacket(CHAT_TYPE_COMMAND, "ExitApplication");
		}
		else
		{
			TPacketGGDisconnect pgg;

			pgg.bHeader = HEADER_GG_DISCONNECT;
			strlcpy(pgg.szLogin, account_name.c_str(), sizeof(pgg.szLogin));
			pgg.bBanned = true;
			P2P_MANAGER::instance().Send(&pgg, sizeof(TPacketGGDisconnect));
		}
		return true;
	}
	return false;
}

bool CBanManager::BanByAccountID(int account_id, DWORD availDt, std::string banReason, std::string player_name, std::string banEvidence)
{
	availDt = get_global_time() + availDt;
	std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("UPDATE `account`.`account` SET `status`='%s',`banavailDt`=FROM_UNIXTIME(%d),`ban_weblink`='%s' WHERE id=%d;", banReason.c_str(), availDt, banEvidence.c_str(), account_id));
	SQLResult* resSelect = pMsg->Get();

	if (resSelect)
	{
		LPCHARACTER ch = CHARACTER_MANAGER::instance().FindPC(player_name.c_str());
		if (ch && ch->GetDesc())
		{
			ch->ChatPacket(CHAT_TYPE_COMMAND, "ExitApplication");
		}
		else
		{
			TPacketGGDisconnect pgg;

			pgg.bHeader = HEADER_GG_DISCONNECT;
			strlcpy(pgg.szLogin, player_name.c_str(), sizeof(pgg.szLogin));
			pgg.bBanned = true;
			P2P_MANAGER::instance().Send(&pgg, sizeof(TPacketGGDisconnect));
		}
		return true;
	}
	return false;
}

bool CBanManager::BanByPlayerName(std::string player_name, DWORD availDt, std::string banReason, std::string banEvidence)
{
	std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("SELECT account_id FROM player.player WHERE name='%s';", player_name.c_str()));
	SQLResult* resSelect = pMsg->Get();

	if (resSelect)
	{
		MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);
		if (row != NULL)
		{
			int accID = 0;
			str_to_number(accID, row[0]);
			bool result = BanByAccountID(accID, availDt, banReason, player_name, banEvidence);
			return result;
		}
	}

	return false;
}

bool CBanManager::ChatBanByPlayerName(std::string player_name, DWORD availDt)
{
	LPCHARACTER tch = CHARACTER_MANAGER::instance().FindPC(player_name.c_str());

	if (!tch)
	{
		CCI* pkCCI = P2P_MANAGER::instance().Find(player_name.c_str());

		if (pkCCI)
		{
			TPacketGGBlockChat p;

			p.bHeader = HEADER_GG_BLOCK_CHAT;
			strlcpy(p.szName, player_name.c_str(), sizeof(p.szName));
			p.lBlockDuration = availDt;
			P2P_MANAGER::instance().Send(&p, sizeof(TPacketGGBlockChat));
			return true;
		}
		return false;
	}

	if (tch)
		tch->AddAffect(AFFECT_BLOCK_CHAT, POINT_NONE, 0, AFF_NONE, availDt, 0, true);

	return true;
}

CBanManager::CBanManager()
{
}

CBanManager::~CBanManager()
{
	Destroy();
}

void CBanManager::Destroy()
{
}