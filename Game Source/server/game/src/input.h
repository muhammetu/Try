#pragma once
#include "packet_info.h"

enum
{
	INPROC_CLOSE,
	INPROC_HANDSHAKE,
	INPROC_LOGIN,
	INPROC_MAIN,
	INPROC_DEAD,
	INPROC_DB,
	INPROC_P2P,
	INPROC_AUTH,
};

#ifdef __BAN_REASON_UTILITY__
void LoginFailure(LPDESC d, const char* c_pszStatus, DWORD availDate = 0, const char* c_pszBanWebLink = NULL);
#else
void LoginFailure(LPDESC d, const char* c_pszStatus);
#endif
extern void SendShout(const char* szText, BYTE bEmpire);

class CInputProcessor
{
public:
	CInputProcessor();
	virtual ~CInputProcessor() {};

	virtual bool Process(LPDESC d, const void* c_pvOrig, int iBytes, int& r_iBytesProceed);
	virtual BYTE GetType() = 0;

	void BindPacketInfo(CPacketInfo* pPacketInfo);
	void Pong(LPDESC d);
	void Handshake(LPDESC d, const char* c_pData);
#ifdef __GLOBAL_MESSAGE_UTILITY__
	void SendBulkWhisper(const char* c_pszText);
#endif

protected:
	virtual int	Analyze(LPDESC d, BYTE bHeader, const char* c_pData) = 0;

	CPacketInfo* m_pPacketInfo;
	int	m_iBufferLeft;

	CPacketInfoCG 	m_packetInfoCG;
};

class CInputClose : public CInputProcessor
{
public:
	virtual BYTE	GetType() { return INPROC_CLOSE; }

protected:
	virtual int	Analyze(LPDESC d, BYTE bHeader, const char* c_pData) { return m_iBufferLeft; }
};

class CInputHandshake : public CInputProcessor
{
public:
	CInputHandshake();
	virtual ~CInputHandshake();

	virtual BYTE	GetType() { return INPROC_HANDSHAKE; }

protected:
	virtual int	Analyze(LPDESC d, BYTE bHeader, const char* c_pData);

protected:
	CPacketInfo* m_pMainPacketInfo;
};

class CInputLogin : public CInputProcessor
{
public:
	virtual BYTE	GetType() { return INPROC_LOGIN; }

protected:
	virtual int	Analyze(LPDESC d, BYTE bHeader, const char* c_pData);

protected:
	void		LoginByKey(LPDESC d, const char* data);

	void		CharacterSelect(LPDESC d, const char* data);
	void		CharacterCreate(LPDESC d, const char* data);
	void		CharacterDelete(LPDESC d, const char* data);
	void		Entergame(LPDESC d, const char* data);
	void		Empire(LPDESC d, const char* c_pData);
	void		GuildMarkCRCList(LPDESC d, const char* c_pData);
	// MARK_BUG_FIX
	void		GuildMarkIDXList(LPDESC d, const char* c_pData);
	// END_OF_MARK_BUG_FIX
	void		GuildMarkUpload(LPDESC d, const char* c_pData);
	void		ChangeName(LPDESC d, const char* data);
};

class CInputMain : public CInputProcessor
{
public:
	virtual BYTE	GetType() { return INPROC_MAIN; }

protected:
	virtual int	Analyze(LPDESC d, BYTE bHeader, const char* c_pData);

protected:
	void		Attack(LPCHARACTER ch, const BYTE header, const char* data);

	int			Whisper(LPCHARACTER ch, const char* data, size_t uiBytes);
#ifdef __GROWTH_PET_SYSTEM__
	void		BraveRequestPetName(LPCHARACTER ch, const char* c_pData);
#endif
#ifdef __GROWTH_PET_SYSTEM__
	void		BraveRequestMountName(LPCHARACTER ch, const char* c_pData);
#endif
	int			Chat(LPCHARACTER ch, const char* data, size_t uiBytes);
	void		ItemUse(LPCHARACTER ch, const char* data);
#ifdef __DROP_DIALOG__
	void		ItemDelete(LPCHARACTER ch, const char* data);
	void		ItemSell(LPCHARACTER ch, const char* data);
#endif
	void		ItemMove(LPCHARACTER ch, const char* data);
	void		ItemPickup(LPCHARACTER ch, const char* data);
	void		ItemToItem(LPCHARACTER ch, const char* pcData);
	void		QuickslotAdd(LPCHARACTER ch, const char* data);
	void		QuickslotDelete(LPCHARACTER ch, const char* data);
	void		QuickslotSwap(LPCHARACTER ch, const char* data);
	int			Shop(LPCHARACTER ch, const char* data, size_t uiBytes);
	void		OnClick(LPCHARACTER ch, const char* data);
	void		Exchange(LPCHARACTER ch, const char* data);
	void		Position(LPCHARACTER ch, const char* data);
	void		Move(LPCHARACTER ch, const char* data);
	int			SyncPosition(LPCHARACTER ch, const char* data, size_t uiBytes);
	void		FlyTarget(LPCHARACTER ch, const char* pcData, BYTE bHeader);
	void		UseSkill(LPCHARACTER ch, const char* pcData);

	void		ScriptAnswer(LPCHARACTER ch, const void* pvData);
	void		ScriptButton(LPCHARACTER ch, const void* pvData);
	void		ScriptSelectItem(LPCHARACTER ch, const void* pvData);

	void		QuestInputString(LPCHARACTER ch, const void* pvData);
	void		QuestConfirm(LPCHARACTER ch, const void* pvData);
	void		Target(LPCHARACTER ch, const char* pcData);
	void		SafeboxCheckin(LPCHARACTER ch, const char* c_pData);
	void		SafeboxCheckout(LPCHARACTER ch, const char* c_pData, bool bMall);
	void		SafeboxItemMove(LPCHARACTER ch, const char* data);
	int			Messenger(LPCHARACTER ch, const char* c_pData, size_t uiBytes);

	void 		PartyInvite(LPCHARACTER ch, const char* c_pData);
	void 		PartyInviteAnswer(LPCHARACTER ch, const char* c_pData);
	void		PartyRemove(LPCHARACTER ch, const char* c_pData);
	void		PartySetState(LPCHARACTER ch, const char* c_pData);
	void		PartyParameter(LPCHARACTER ch, const char* c_pData);

	int			Guild(LPCHARACTER ch, const char* data, size_t uiBytes);
	void		AnswerMakeGuild(LPCHARACTER ch, const char* c_pData);

	void		Fishing(LPCHARACTER ch, const char* c_pData);
	void		ItemGive(LPCHARACTER ch, const char* c_pData);
	void		Hack(LPCHARACTER ch, const char* c_pData);
	int			MyShop(LPCHARACTER ch, const char* c_pData, size_t uiBytes);

	void		Refine(LPCHARACTER ch, const char* c_pData);
#ifdef __ACCE_SYSTEM__
	void		Acce(LPCHARACTER pkChar, const char* c_pData);
#endif
#ifdef __ANCIENT_ATTR_ITEM__
	void		ItemNewAttributes(LPCHARACTER ch, const char* pcData);
#endif
#ifdef __CHEST_INFO_SYSTEM__
	void		ChestDropInfo(LPCHARACTER ch, const char* c_pData);
#endif
#ifdef __GLOBAL_MESSAGE_UTILITY__
	void		BulkWhisperManager(LPCHARACTER ch, const char* c_pData);
#endif
#ifdef __SKILL_CHOOSE_WINDOW__
	void		SkillChoosePacket(LPCHARACTER ch, const char* c_pData);
#endif
#ifdef __ITEM_CHANGELOOK__
	void		ChangeLook(LPCHARACTER pkChar, const char* c_pData);
#endif
#ifdef __SKILL_COLOR__
	void		SetSkillColor(LPCHARACTER ch, const char* pcData);
#endif
#ifdef __DUNGEON_INFORMATION__
	void		DungeonTeleport(LPCHARACTER ch, const char* c_pData);
#endif
#ifdef __SWITCHBOT__
	int			Switchbot(LPCHARACTER ch, const char* data, size_t uiBytes);
#endif
#ifdef __CUBE_WINDOW__
	void 		CubeRenewalSend(LPCHARACTER ch, const char* data);
#endif
#ifdef __FAST_CHEQUE_TRANSFER__
	void		WonExchange(LPCHARACTER ch, const char* pcData);
#endif
#ifdef __TARGET_BOARD_RENEWAL__
	void		SendMobInformation(LPCHARACTER ch, LPCHARACTER pkTarget, BYTE bType);
	void		MobInformation(LPCHARACTER ch, const char* c_pData);
#endif
#ifdef __BATTLE_PASS_SYSTEM__
	int BattlePass(LPCHARACTER ch, const char* data, size_t uiBytes);
#endif
};

class CInputDead : public CInputMain
{
public:
	virtual BYTE	GetType() { return INPROC_DEAD; }

protected:
	virtual int	Analyze(LPDESC d, BYTE bHeader, const char* c_pData);
};

class CInputDB : public CInputProcessor
{
public:
	virtual bool Process(LPDESC d, const void* c_pvOrig, int iBytes, int& r_iBytesProceed);
	virtual BYTE GetType() { return INPROC_DB; }

protected:
	virtual int	Analyze(LPDESC d, BYTE bHeader, const char* c_pData);

protected:
	void		MapLocations(const char* c_pData);
	void		LoginSuccess(DWORD dwHandle, const char* data);
	void		PlayerCreateFailure(LPDESC d, BYTE bType);
	void		PlayerDeleteSuccess(LPDESC d, const char* data);
	void		PlayerDeleteFail(LPDESC d);
	void		PlayerLoad(LPDESC d, const char* data);
	void		PlayerCreateSuccess(LPDESC d, const char* data);
	void		Boot(const char* data);
	void		QuestLoad(LPDESC d, const char* c_pData);
	void		SafeboxLoad(LPDESC d, const char* c_pData);
	void		SafeboxChangeSize(LPDESC d, const char* c_pData);
	void		SafeboxWrongPassword(LPDESC d);
	void		SafeboxChangePasswordAnswer(LPDESC d, const char* c_pData);
	void		MallLoad(LPDESC d, const char* c_pData);
	void		EmpireSelect(LPDESC d, const char* c_pData);
	void		P2P(const char* c_pData);
	void		ItemLoad(LPDESC d, const char* c_pData);
	void		AffectLoad(LPDESC d, const char* c_pData);
#ifdef __SKILL_COLOR__
	void		SkillColorLoad(LPDESC d, const char* c_pData);
#endif
	void		GuildLoad(const char* c_pData);
	void		GuildSkillUpdate(const char* c_pData);
	void		GuildSkillRecharge();
	void		GuildExpUpdate(const char* c_pData);
	void		GuildAddMember(const char* c_pData);
	void		GuildRemoveMember(const char* c_pData);
	void		GuildChangeGrade(const char* c_pData);
	void		GuildChangeMemberData(const char* c_pData);
	void		GuildDisband(const char* c_pData);
	void		GuildLadder(const char* c_pData);
	void		GuildWar(const char* c_pData);
	void		GuildWarScore(const char* c_pData);
	void		GuildSkillUsableChange(const char* c_pData);
	void		GuildChangeMaster(TPacketChangeGuildMaster* p);

	void		LoginAlready(LPDESC d, const char* c_pData);

	void		PartyCreate(const char* c_pData);
	void		PartyDelete(const char* c_pData);
	void		PartyAdd(const char* c_pData);
	void		PartyRemove(const char* c_pData);
	void		PartyStateChange(const char* c_pData);
	void		PartySetMemberLevel(const char* c_pData);

	void		Time(const char* c_pData);

	void		ReloadProto(const char* c_pData);
#ifdef ENABLE_RELOAD_SHOP_COMMAND
	void		ReloadShopProto(const char* c_pData);
#endif
#ifdef ENABLE_RELOAD_REFINE_COMMAND
	void		ReloadRefineProto(const char* c_pData);
#endif
	void		ChangeName(LPDESC d, const char* data);

	void		AuthLogin(LPDESC d, const char* c_pData);

	void		ChangeEmpirePriv(const char* c_pData);

	void		SetEventFlag(const char* c_pData);

	void		Notice(const char* c_pData);

	//RELOAD_ADMIN
	void ReloadAdmin(const char* c_pData);
	//END_RELOAD_ADMIN

	void		DetailLog(const TPacketNeedLoginLogInfo* info);

	void		RespondChannelStatus(LPDESC desc, const char* pcData);
#ifdef __CHANNEL_CHANGER__
	void		ChangeChannel(LPDESC desc, const char* pcData);
#endif
#ifdef __BATTLE_PASS_SYSTEM__
	void BattlePassLoad(LPDESC d, const char* c_pData);
	void BattlePassLoadRanking(LPDESC d, const char* c_pData);
#endif
protected:
	DWORD		m_dwHandle;
};

class CInputP2P : public CInputProcessor
{
public:
	CInputP2P();
	virtual BYTE	GetType() { return INPROC_P2P; }

protected:
	virtual int	Analyze(LPDESC d, BYTE bHeader, const char* c_pData);

public:
	void		Setup(LPDESC d, const char* c_pData);
	void		Login(LPDESC d, const char* c_pData);
	void		Logout(LPDESC d, const char* c_pData);
	int			Relay(LPDESC d, const char* c_pData, size_t uiBytes);
#ifdef __FULL_NOTICE__
	int			Notice(LPDESC d, const char* c_pData, size_t uiBytes, bool bBigFont = false);
#else
	int			Notice(LPDESC d, const char* c_pData, size_t uiBytes);
#endif
#ifdef __GLOBAL_MESSAGE_UTILITY__
	int			BulkWhisperSend(LPDESC d, const char* c_pData, size_t uiBytes);
#endif
	int			Guild(LPDESC d, const char* c_pData, size_t uiBytes);
	void		Shout(const char* c_pData);
	void		Disconnect(const char* c_pData);
	void		MessengerAdd(const char* c_pData);
	void		MessengerRemove(const char* c_pData);
	void		FindPosition(LPDESC d, const char* c_pData);
#ifdef __WARP_WITH_CHANNEL__
	void		WarpCharacter(LPDESC d, const char* c_pData);
#else
	void		WarpCharacter(const char* c_pData);
#endif // __WARP_WITH_CHANNEL__
	void		GuildWarZoneMapIndex(const char* c_pData);
#ifdef __WARP_WITH_CHANNEL__
	void		Transfer(LPDESC d, const char* c_pData);
#else
	void		Transfer(const char* c_pData);
#endif // __WARP_WITH_CHANNEL__
	void		BlockChat(const char* c_pData);
#ifdef __EVENT_SYSTEM__
	void		EventTime(const char* c_pData);
#endif
#ifdef __SWITCHBOT__
	void		Switchbot(LPDESC d, const char* c_pData);
#endif
#ifdef __CHAT_FILTER__
	int			NoticeImproving(LPDESC d, const char* c_pData, size_t uiBytes);
#endif
#ifdef ENABLE_BOSS_TRACKING
	void		BossTracking(LPDESC d, const char * c_pData);
#endif
	void		LoginPing(LPDESC d, const char * c_pData);
#ifdef ENABLE_MULTI_FARM_BLOCK
	void MultiFarm(const char* c_pData);
	void MultiFarmQF(const char* c_pData);
#endif
protected:
	CPacketInfoGG 	m_packetInfoGG;
};

class CInputAuth : public CInputProcessor
{
public:
	CInputAuth();
	virtual BYTE GetType() { return INPROC_AUTH; }

protected:
	virtual int	Analyze(LPDESC d, BYTE bHeader, const char* c_pData);

public:
	void		Login(LPDESC d, const char* c_pData);
};