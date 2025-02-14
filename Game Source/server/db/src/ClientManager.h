#pragma once

#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

#include "../../common/stl.h"

#include "Peer.h"
#include "DBManager.h"
#include "LoginData.h"

class CPlayerTableCache;
class CItemCache;
class CQuestCache;
class CAffectCache;
#ifdef __SKILL_COLOR__
class CSKillColorCache;
#endif
#ifdef __BATTLE_PASS_SYSTEM__
class CBattlePassCache;
#endif // __BATTLE_PASS_SYSTEM__

#ifdef __OFFLINE_SHOP__
#include "OfflineshopCache.h"
#endif

class CPacketInfo
{
public:
	void Add(int header);
	void Reset();

	std::map<int, int> m_map_info;
};

size_t CreatePlayerSaveQuery(char* pszQuery, size_t querySize, TPlayerTable* pkTab);

class CClientManager : public CNetBase, public singleton<CClientManager>
{
public:
	typedef std::list<CPeer*>			TPeerList;
	typedef boost::unordered_map<DWORD, CPlayerTableCache*> TPlayerTableCacheMap;
	typedef boost::unordered_map<DWORD, CItemCache*> TItemCacheMap;
	typedef boost::unordered_set<CItemCache*, boost::hash<CItemCache*> > TItemCacheSet;
	typedef boost::unordered_map<DWORD, TItemCacheSet*> TItemCacheSetPtrMap;
	typedef boost::unordered_set<CQuestCache*, boost::hash<CQuestCache*> > TQuestCacheSet;
	typedef boost::unordered_map<DWORD, TQuestCacheSet*> TQuestCacheSetPtrMap;
	typedef boost::unordered_set<CAffectCache*, boost::hash<CAffectCache*> > TAffectCacheSet;
	typedef boost::unordered_map<DWORD, TAffectCacheSet*> TAffectCacheSetPtrMap;
#ifdef __SKILL_COLOR__
	typedef boost::unordered_map<DWORD, CSKillColorCache*> TSkillColorCacheMap;
#endif
#ifdef __BATTLE_PASS_SYSTEM__
	typedef boost::unordered_set<CBattlePassCache*, boost::hash<CBattlePassCache*> > TBattlePassCacheSet;
	typedef boost::unordered_map<DWORD, TBattlePassCacheSet*> TBattlePassCacheSetPtrMap;
#endif // __BATTLE_PASS_SYSTEM__

	typedef boost::unordered_map<short, BYTE> TChannelStatusMap;

	class ClientHandleInfo
	{
	public:
		DWORD	dwHandle;
		DWORD	account_id;
		DWORD	player_id;
		BYTE	account_index;
		char	login[LOGIN_MAX_LEN + 1];
		char	safebox_password[SAFEBOX_PASSWORD_MAX_LEN + 1];
		char	ip[MAX_HOST_LENGTH + 1];

		TAccountTable* pAccountTable;
		TSafeboxTable* pSafebox;

		ClientHandleInfo(DWORD argHandle, DWORD dwPID = 0)
		{
			dwHandle = argHandle;
			pSafebox = NULL;
			pAccountTable = NULL;
			player_id = dwPID;
		};

		ClientHandleInfo(DWORD argHandle, DWORD dwPID, DWORD accountId)
		{
			dwHandle = argHandle;
			pSafebox = NULL;
			pAccountTable = NULL;
			player_id = dwPID;
			account_id = accountId;
		};

		~ClientHandleInfo()
		{
			if (pSafebox)
			{
				delete pSafebox;
				pSafebox = NULL;
			}
		}
	};

public:
	CClientManager();
	~CClientManager();

	bool	Initialize();
	time_t	GetCurrentTime();

	void	MainLoop();
	void	Quit();

	void	SetPlayerIDStart(int iIDStart);
	int	GetPlayerIDStart() { return m_iPlayerIDStart; }

	int	GetPlayerDeleteLevelLimit() { return m_iPlayerDeleteLevelLimit; }

	DWORD	GetUserCount();

	void	SendAllGuildSkillRechargePacket();
	void	SendTime();

	CPlayerTableCache* GetPlayerCache(DWORD id);
	void			PutPlayerCache(TPlayerTable* pNew);

	void			CreateItemCacheSet(DWORD dwID);
	TItemCacheSet* GetItemCacheSet(DWORD dwID);
	void			FlushItemCacheSet(DWORD dwID);

	CItemCache* GetItemCache(DWORD id);
	void			PutItemCache(TPlayerItem* pNew, bool bSkipQuery = false);
	bool			DeleteItemCache(DWORD id);

	TQuestCacheSet*	CreateQuestCacheSet(DWORD dwID);
	TQuestCacheSet* GetQuestCacheSet(DWORD dwID);
	void			FlushQuestCacheSet(DWORD dwID);

	TAffectCacheSet*	CreateAffectCacheSet(DWORD dwID);
	TAffectCacheSet* GetAffectCacheSet(DWORD dwID);
	void			FlushAffectCacheSet(DWORD dwID);

#ifdef __SKILL_COLOR__
	CSKillColorCache* GetSkillColorCache(DWORD id);
	void				PutSkillColorCache(const TSkillColor* pNew);
	void				UpdateSkillColorCache();
#endif

#ifdef __BATTLE_PASS_SYSTEM__
	TBattlePassCacheSet* CreateBattlePassCacheSet(DWORD dwID);
	TBattlePassCacheSet* GetBattlePassCacheSet(DWORD dwID);
	void			FlushBattlePassCacheSet(DWORD dwID);
#endif // __BATTLE_PASS_SYSTEM__

	void			UpdatePlayerCache();
	void			UpdateItemCache();

	void			SendGuildSkillUsable(DWORD guild_id, DWORD dwSkillVnum, bool bUsable);

	void			SetCacheFlushCountLimit(int iLimit);

	template <class Func>
	Func		for_each_peer(Func f);

	CPeer* GetAnyPeer();

	void			ForwardPacket(BYTE header, const void* data, int size, BYTE bChannel = 0, CPeer* except = NULL);

	void			SendNotice(const char* c_pszFormat, ...);
protected:
	void	Destroy();

private:
	bool		InitializeTables();
	bool		InitializeShopTable();
	bool		InitializeMobTable();
	bool		InitializeItemTable();
	bool		InitializeSkillTable();
	bool		InitializeRefineTable();
	bool		InitializeItemAttrTable();
	bool		InitializeItemRareTable();

#ifdef __SCP1453_EXTENSIONS__
	bool		DeleteDungeonRejoinTimes();
#endif

	void		AddPeer(socket_t fd);
	void		RemovePeer(CPeer* pPeer);
	CPeer* GetPeer(IDENT ident);

	int		AnalyzeQueryResult(SQLMsg* msg);

	int		Process();

	void            ProcessPackets(CPeer* peer);

	CLoginData* GetLoginData(DWORD dwKey);
	CLoginData* GetLoginDataByLogin(const char* c_pszLogin);
	CLoginData* GetLoginDataByAID(DWORD dwAID);

	void		InsertLoginData(CLoginData* pkLD);
	void		DeleteLoginData(CLoginData* pkLD);

	bool		InsertLogonAccount(const char* c_pszLogin, DWORD dwHandle, const char* c_pszIP);
	bool		DeleteLogonAccount(const char* c_pszLogin, DWORD dwHandle);
	bool		FindLogonAccount(const char* c_pszLogin);

	void		GuildCreate(CPeer* peer, DWORD dwGuildID);
	void		GuildSkillUpdate(CPeer* peer, TPacketGuildSkillUpdate* p);
	void		GuildExpUpdate(CPeer* peer, TPacketGuildExpUpdate* p);
	void		GuildAddMember(CPeer* peer, TPacketGDGuildAddMember* p);
	void		GuildChangeGrade(CPeer* peer, TPacketGuild* p);
	void		GuildRemoveMember(CPeer* peer, TPacketGuild* p);
	void		GuildChangeMemberData(CPeer* peer, TPacketGuildChangeMemberData* p);
	void		GuildDisband(CPeer* peer, TPacketGuild* p);
	void		GuildWar(CPeer* peer, TPacketGuildWar* p);
	void		GuildWarScore(CPeer* peer, TPacketGuildWarScore* p);
	void		GuildChangeLadderPoint(TPacketGuildLadderPoint* p);
	void		GuildUseSkill(TPacketGuildUseSkill* p);
	void		GuildChangeMaster(TPacketChangeGuildMaster* p);

	void		QUERY_BOOT(CPeer* peer, TPacketGDBoot* p);

	void		QUERY_LOGOUT(CPeer* peer, DWORD dwHandle, const char*);

	void		RESULT_LOGIN(CPeer* peer, SQLMsg* msg);

	void		QUERY_PLAYER_LOAD(CPeer* peer, DWORD dwHandle, TPlayerLoadPacket*);
	void		RESULT_COMPOSITE_PLAYER(CPeer* peer, SQLMsg* pMsg, DWORD dwQID);
	void		RESULT_PLAYER_LOAD(CPeer* peer, MYSQL_RES* pRes, ClientHandleInfo* pkInfo);
	void		RESULT_ITEM_LOAD(CPeer* peer, MYSQL_RES* pRes, DWORD dwHandle, DWORD dwPID);
	void		RESULT_QUEST_LOAD(CPeer* pkPeer, MYSQL_RES* pRes, DWORD dwHandle, DWORD dwPID);
	void		RESULT_AFFECT_LOAD(CPeer* pkPeer, MYSQL_RES* pRes, DWORD dwHandle, DWORD dwRealPID);

#ifdef __SKILL_COLOR__
	void		QUERY_SKILL_COLOR_LOAD(CPeer* peer, DWORD dwHandle, TPlayerLoadPacket* packet);
	void		RESULT_SKILL_COLOR_LOAD(CPeer* peer, MYSQL_RES* pRes, DWORD dwHandle);
#endif

#ifdef __BATTLE_PASS_SYSTEM__
	void RESULT_BATTLE_PASS_LOAD(CPeer* peer, MYSQL_RES* pRes, DWORD dwHandle, DWORD dwRealPID);
	void QUERY_SAVE_BATTLE_PASS(CPeer* pkPeer, const char* c_pData, DWORD dwLen);
	void QUERY_BATTLE_PASS_ADD(CPeer* pkPeer, const char* c_pData, DWORD dwLen);
	void QUERY_REGISTER_RANKING(CPeer* peer, DWORD dwHandle, TBattlePassRegisterRanking* pRanking);

	bool LoadBattlePassRanking();
	void RequestLoadBattlePassRanking(CPeer* peer, DWORD dwHandle, const char* data);
#endif

	// PLAYER_INDEX_CREATE_BUG_FIX
	void		RESULT_PLAYER_INDEX_CREATE(CPeer* pkPeer, SQLMsg* msg);
	// END_PLAYER_INDEX_CREATE_BUG_FIX

	void		QUERY_PLAYER_SAVE(CPeer* peer, DWORD dwHandle, TPlayerTable*);

	void		__QUERY_PLAYER_CREATE(CPeer* peer, DWORD dwHandle, TPlayerCreatePacket*);
	void		__QUERY_PLAYER_DELETE(CPeer* peer, DWORD dwHandle, TPlayerDeletePacket*);
	void		__RESULT_PLAYER_DELETE(CPeer* peer, SQLMsg* msg);

	void		QUERY_PLAYER_COUNT(CPeer* pkPeer, TPlayerCountPacket*);

#ifdef __SKILL_COLOR__
	void		QUERY_SKILL_COLOR_SAVE(const char* c_pData);
#endif
	void		QUERY_ITEM_SAVE(CPeer* pkPeer, const char* c_pData);
	void		QUERY_ITEM_DESTROY(CPeer* pkPeer, const char* c_pData);
	void		QUERY_ITEM_FLUSH(CPeer* pkPeer, const char* c_pData);

	void		QUERY_QUEST_SAVE(CPeer* pkPeer, const char* c_pData, DWORD dwLen);

	void		QUERY_AFFECT_SAVE(CPeer* pkPeer, const char* c_pData, DWORD dwLen);
	void		QUERY_AFFECT_ADD(CPeer* pkPeer, const char* c_pData, DWORD dwLen);
	void		QUERY_AFFECT_CHANGE(CPeer* pkPeer, const char* c_pData, DWORD dwLen);
	void		QUERY_AFFECT_REMOVE(CPeer* pkPeer, const char* c_pData, DWORD dwLen);

	void		QUERY_SAFEBOX_LOAD(CPeer* pkPeer, DWORD dwHandle, TSafeboxLoadPacket*, bool bMall);
	void		QUERY_SAFEBOX_SAVE(CPeer* pkPeer, TSafeboxTable* pTable);
	void		QUERY_SAFEBOX_CHANGE_SIZE(CPeer* pkPeer, DWORD dwHandle, TSafeboxChangeSizePacket* p);
	void		QUERY_SAFEBOX_CHANGE_PASSWORD(CPeer* pkPeer, DWORD dwHandle, TSafeboxChangePasswordPacket* p);

	void		RESULT_SAFEBOX_LOAD(CPeer* pkPeer, SQLMsg* msg);
	void		RESULT_SAFEBOX_CHANGE_SIZE(CPeer* pkPeer, SQLMsg* msg);
	void		RESULT_SAFEBOX_CHANGE_PASSWORD(CPeer* pkPeer, SQLMsg* msg);
	void		RESULT_SAFEBOX_CHANGE_PASSWORD_SECOND(CPeer* pkPeer, SQLMsg* msg);

	void		QUERY_EMPIRE_SELECT(CPeer* pkPeer, DWORD dwHandle, TEmpireSelectPacket* p);
	void		QUERY_SETUP(CPeer* pkPeer, DWORD dwHandle, const char* c_pData);

	void		SendPartyOnSetup(CPeer* peer);

	void		QUERY_FLUSH_CACHE(CPeer* pkPeer, const char* c_pData);

	void		QUERY_PARTY_CREATE(CPeer* peer, TPacketPartyCreate* p);
	void		QUERY_PARTY_DELETE(CPeer* peer, TPacketPartyDelete* p);
	void		QUERY_PARTY_ADD(CPeer* peer, TPacketPartyAdd* p);
	void		QUERY_PARTY_REMOVE(CPeer* peer, TPacketPartyRemove* p);
	void		QUERY_PARTY_STATE_CHANGE(CPeer* peer, TPacketPartyStateChange* p);
	void		QUERY_PARTY_SET_MEMBER_LEVEL(CPeer* peer, TPacketPartySetMemberLevel* p);

	void		QUERY_RELOAD_PROTO();

#ifdef ENABLE_RELOAD_SHOP_COMMAND
	void		QUERY_RELOAD_SHOP();
#endif
#ifdef ENABLE_RELOAD_REFINE_COMMAND
	void		QUERY_RELOAD_REFINE();
#endif
	void		QUERY_CHANGE_NAME(CPeer* peer, DWORD dwHandle, TPacketGDChangeName* p);

	void		AddEmpirePriv(TPacketGiveEmpirePriv* p);
	void		QUERY_AUTH_LOGIN(CPeer* pkPeer, DWORD dwHandle, TPacketGDAuthLogin* p);
	void		QUERY_LOGIN_BY_KEY(CPeer* pkPeer, DWORD dwHandle, TPacketGDLoginByKey* p);
	void		RESULT_LOGIN_BY_KEY(CPeer* peer, SQLMsg* msg);

	void		ChargeCash(const TRequestChargeCash* p);

	void		LoadEventFlag();

	void		SetEventFlag(TPacketSetEventFlag* p);
	void		SendEventFlagsOnSetup(CPeer* peer);

#ifdef __CHANNEL_CHANGER__
	void		FindChannel(CPeer* pkPeer, DWORD dwHandle, TPacketChangeChannel* p);
#endif

private:
	int					m_looping;
	socket_t				m_fdAccept;
	TPeerList				m_peerList;

	CPeer* m_pkAuthPeer;

	// LoginKey, LoginData pair
	typedef boost::unordered_map<DWORD, CLoginData*> TLoginDataByLoginKey;
	TLoginDataByLoginKey			m_map_pkLoginData;

	// Login LoginData pair
	typedef boost::unordered_map<std::string, CLoginData*> TLoginDataByLogin;
	TLoginDataByLogin			m_map_pkLoginDataByLogin;

	// AccountID LoginData pair
	typedef boost::unordered_map<DWORD, CLoginData*> TLoginDataByAID;
	TLoginDataByAID				m_map_pkLoginDataByAID;

	typedef boost::unordered_map<std::string, CLoginData*> TLogonAccountMap;
	TLogonAccountMap			m_map_kLogonAccount;

	int					m_iPlayerIDStart;
	int					m_iPlayerDeleteLevelLimit;
	int					m_iPlayerDeleteLevelLimitLower;

	std::vector<TMobTable>			m_vec_mobTable;
	std::vector<TItemTable>			m_vec_itemTable;
	std::map<DWORD, TItemTable*>		m_map_itemTableByVnum;

	int					m_iShopTableSize;
	TShopTable* m_pShopTable;

	int					m_iRefineTableSize;
	TRefineTable* m_pRefineTable;

	std::vector<TSkillTable>		m_vec_skillTable;
	std::vector<TItemAttrTable>		m_vec_itemAttrTable;
	std::vector<TItemAttrTable>		m_vec_itemRareTable;

	bool					m_bShutdowned;

	TPlayerTableCacheMap			m_map_playerCache;

	TItemCacheMap				m_map_itemCache;
	TItemCacheSetPtrMap			m_map_pkItemCacheSetPtr;

	TQuestCacheSetPtrMap		m_map_pkQuestCacheSetPtr;

	TAffectCacheSetPtrMap		m_map_pkAffectCacheSetPtr;

#ifdef __SKILL_COLOR__
	TSkillColorCacheMap			m_map_SkillColorCache;
#endif
#ifdef __BATTLE_PASS_SYSTEM__
	TBattlePassCacheSetPtrMap	m_map_pkBattlePassCacheSetPtr;
	std::vector<TBattlePassRanking*> m_vec_battlePassRanking;
#endif
	TChannelStatusMap m_mChannelStatus;

	struct TPartyInfo
	{
		BYTE bRole;
		BYTE bLevel;

		TPartyInfo() :bRole(0), bLevel(0)
		{
		}
	};

	typedef std::map<DWORD, TPartyInfo>	TPartyMember;
	typedef std::map<DWORD, TPartyMember>	TPartyMap;
	typedef std::map<BYTE, TPartyMap>	TPartyChannelMap;
	TPartyChannelMap m_map_pkChannelParty;

	typedef std::map<std::string, long>	TEventFlagMap;
	TEventFlagMap m_map_lEventFlag;

	BYTE					m_bLastHeader;
	int					m_iCacheFlushCount;
	int					m_iCacheFlushCountLimit;

#ifdef __OFFLINE_SHOP__
public:
	//booting
	bool	InitializeOfflineshopTable();
	void	SendOfflineshopTable(CPeer* peer);

	//packets exchanging
	void	RecvOfflineShopPacket(CPeer* peer, const char* data);
	bool	RecvOfflineShopBuyItemPacket(const char* data);
	bool	RecvOfflineShopLockBuyItem(CPeer* peer, const char* data);
	bool	RecvOfflineShopCannotBuyLockItem(const char* data); //topatch
	bool	RecvOfflineShopRemoveItemPacket(const char* data);
	bool	RecvOfflineShopAddItemPacket(const char* data);

	bool	RecvOfflineShopForceClose(const char* data);
	bool	RecvOfflineShopCreateNew(const char* data);

	bool	RecvOfflineShopSafeboxGetItem(const char* data);
	bool	RecvOfflineShopSafeboxGetValutes(const char* data);
	bool	RecvOfflineShopSafeboxAddItem(const char* data);

	bool	SendOfflineShopBuyItemPacket(DWORD dwOwner, DWORD dwGuest, DWORD dwItem, bool isSearch);
	bool	SendOfflineShopBuyLockedItemPacket(CPeer* peer, DWORD dwOwner, DWORD dwGuest, DWORD dwItem, bool isSearch);
	bool	SendOfflineShopRemoveItemPacket(DWORD dwOwner, DWORD dwItem);
	bool	SendOfflineShopAddItemPacket(DWORD dwOwner, DWORD dwItemID, const offlineshop::TItemInfo& rInfo);

	bool	SendOfflineShopForceClose(DWORD dwOwnerID);
	bool	SendOfflineShopCreateNew(const offlineshop::TShopInfo& shop, const std::vector<offlineshop::TItemInfo>& vec);
	bool	SendOfflineshopShopExpired(DWORD dwOwnerID);
	
	void	SendOfflineshopSafeboxGetItem(DWORD dwOwnerID, DWORD dwItemID);
	void	SendOfflineshopSafeboxGetValutes(DWORD dwOwnerID, const offlineshop::TValutesInfo& valute);
	void	SendOfflineshopSafeboxAddItem(DWORD dwOwnerID, DWORD dwItem, const offlineshop::TItemInfoEx& item);
	void	SendOfflineshopSafeboxAddValutes(DWORD dwOwnerID, const offlineshop::TValutesInfo& valute);
	void	SendOfflineshopSafeboxLoad(CPeer* peer, DWORD dwOwnerID, const offlineshop::TValutesInfo& valute, const std::vector<offlineshop::TItemInfoEx>& items, const std::vector<DWORD>& ids);
	//patch 08-03-2020
	void	SendOfflineshopSafeboxExpiredItem(DWORD dwOwnerID, DWORD itemID);

	void	OfflineShopResultQuery(CPeer* peer, SQLMsg* msg, CQueryInfo* pQueryInfo);
	void	OfflineShopResultAddItemQuery(CPeer* peer, SQLMsg* msg, CQueryInfo* pQueryInfo);
	void	OfflineShopResultCreateShopQuery(CPeer* peer, SQLMsg* msg, CQueryInfo* pQueryInfo);
	void	OfflineShopResultCreateShopAddItemQuery(CPeer* peer, SQLMsg* msg, CQueryInfo* pQueryInfo);
	void	OfflineShopResultSafeboxAddItemQuery(CPeer* peer, SQLMsg* msg, CQueryInfo* pQueryInfo);

	void	OfflineshopDurationProcess();
	void	OfflineshopExpiredShop(DWORD dwID);
	void	OfflineshopLoadShopSafebox(CPeer* peer, DWORD dwID);

	// @@ memory consumption
	void	OfflineshopDelShopSafebox(uint32_t dwID);

	bool	IsUsingOfflineshopSystem(DWORD dwID);

private:
	offlineshop::CShopCache			m_offlineshopShopCache;
	offlineshop::CSafeboxCache		m_offlineshopSafeboxCache;
#endif

	//BOOT_LOCALIZATION
public:

	bool InitializeLocalization();

private:
	std::vector<tLocale> m_vec_Locale;
	//END_BOOT_LOCALIZATION
	//ADMIN_MANAGER

	bool __GetAdminInfo(const char* szIP, std::vector<tAdminInfo>& rAdminVec);
	//END_ADMIN_MANAGER

	//RELOAD_ADMIN
	void ReloadAdmin(CPeer* peer, TPacketReloadAdmin* p);
	//END_RELOAD_ADMIN
	void ReloadOffShopCount(int count);

	struct TLogoutPlayer
	{
		DWORD	pid;
		time_t	time;

		bool operator < (const TLogoutPlayer& r)
		{
			return (pid < r.pid);
		}
	};

	typedef boost::unordered_map<DWORD, TLogoutPlayer*> TLogoutPlayerMap;
	TLogoutPlayerMap m_map_logout;

	void InsertLogoutPlayer(DWORD pid);
	void DeleteLogoutPlayer(DWORD pid);
	void UpdateLogoutPlayer();
	void UpdateItemCacheSet(DWORD pid);
	void UpdateQuestCacheSet(DWORD pid);

	void FlushPlayerCacheSet(DWORD pid);

	void SendSpareItemIDRange(CPeer* peer);

	void UpdateHorseName(TPacketUpdateHorseName* data, CPeer* peer);
	void AckHorseName(DWORD dwPID, CPeer* peer);
	void DeleteLoginKey(TPacketDC* data);
	void ResetLastPlayerID(const TPacketNeedLoginLogInfo* data);
	//delete gift notify icon
	void UpdateChannelStatus(TChannelStatus* pData);
	void RequestChannelStatus(CPeer* peer, DWORD dwHandle);
};

template<class Func>
Func CClientManager::for_each_peer(Func f)
{
	TPeerList::iterator it;
	for (it = m_peerList.begin(); it != m_peerList.end(); ++it)
	{
		f(*it);
	}
	return f;
}