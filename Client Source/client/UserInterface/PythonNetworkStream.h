#pragma once

#include "../eterLib/FuncObject.h"
#include "../eterlib/NetStream.h"
#include "../eterLib/NetPacketHeaderMap.h"

#include "packet.h"

#ifdef ENABLE_SWITCHBOT_SYSTEM
#include "PythonSwitchbot.h"
#endif

class CInstanceBase;
class CNetworkActorManager;
struct SNetworkActorData;
struct SNetworkUpdateActorData;

class CPythonNetworkStream : public CNetworkStream, public CSingleton<CPythonNetworkStream>
{
public:
	enum
	{
		SERVER_COMMAND_LOG_OUT = 0,
		SERVER_COMMAND_RETURN_TO_SELECT_CHARACTER = 1,
		SERVER_COMMAND_QUIT = 2,

		MAX_ACCOUNT_PLAYER
	};

	enum
	{
		ERROR_NONE,
		ERROR_UNKNOWN,
		ERROR_CONNECT_MARK_SERVER,
		ERROR_LOAD_MARK,
		ERROR_MARK_WIDTH,
		ERROR_MARK_HEIGHT,

		// MARK_BUG_FIX
		ERROR_MARK_UPLOAD_NEED_RECONNECT,
		ERROR_MARK_CHECK_NEED_RECONNECT,
		// END_OF_MARK_BUG_FIX
	};

	enum
	{
		ACCOUNT_CHARACTER_SLOT_ID,
		ACCOUNT_CHARACTER_SLOT_NAME,
		ACCOUNT_CHARACTER_SLOT_RACE,
		ACCOUNT_CHARACTER_SLOT_LEVEL,
		ACCOUNT_CHARACTER_SLOT_STR,
		ACCOUNT_CHARACTER_SLOT_DEX,
		ACCOUNT_CHARACTER_SLOT_HTH,
		ACCOUNT_CHARACTER_SLOT_INT,
		ACCOUNT_CHARACTER_SLOT_PLAYTIME,
		ACCOUNT_CHARACTER_SLOT_FORM,
		ACCOUNT_CHARACTER_SLOT_ADDR,
		ACCOUNT_CHARACTER_SLOT_PORT,
		ACCOUNT_CHARACTER_SLOT_GUILD_ID,
		ACCOUNT_CHARACTER_SLOT_GUILD_NAME,
		ACCOUNT_CHARACTER_SLOT_CHANGE_NAME_FLAG,
		ACCOUNT_CHARACTER_SLOT_HAIR,
#ifdef ENABLE_ACCE_SYSTEM
		ACCOUNT_CHARACTER_SLOT_ACCE,
#endif
#ifdef ENABLE_CHARACTER_WINDOW_RENEWAL
		ACCOUNT_CHARACTER_SLOT_LAST_PLAYTIME,
#endif
	};

	enum
	{
		PHASE_WINDOW_LOGIN,
		PHASE_WINDOW_SELECT,
		PHASE_WINDOW_CREATE,
		PHASE_WINDOW_LOAD,
		PHASE_WINDOW_GAME,
		PHASE_WINDOW_EMPIRE,
		PHASE_WINDOW_NUM,
	};

public:
	CPythonNetworkStream();
	virtual ~CPythonNetworkStream();

	bool SendSpecial(int nLen, void* pvBuf);

	void StartGame();
	void Warp(LONG lGlobalX, LONG lGlobalY);

	void NotifyHack(const char* c_szMsg);
	void SetWaitFlag();

	void SendEmoticon(UINT eEmoticon);
	void ExitApplication();
	void ExitGame();
	void LogOutGame();
	void AbsoluteExitGame();
	void AbsoluteExitApplication();

	DWORD GetGuildID();

	UINT UploadMark(const char* c_szImageFileName);

	bool LoadConvertTable(DWORD dwEmpireID, const char* c_szFileName);

	UINT		GetAccountCharacterSlotDatau(UINT iSlot, UINT eType);
	const char* GetAccountCharacterSlotDataz(UINT iSlot, UINT eType);

	// SUPPORT_BGM
	const char* GetFieldMusicFileName();
	float			GetFieldMusicVolume();
	// END_OF_SUPPORT_BGM

	bool IsSelectedEmpire();

	void ToggleGameDebugInfo();

	void SetMarkServer(const char* c_szAddr, UINT uPort);
	void ConnectLoginServer(const char* c_szAddr, UINT uPort);
	void ConnectGameServer(UINT iChrSlot);

	void SetLoginInfo(const char* c_szID, const char* c_szPassword
#ifdef ENABLE_CLIENT_VERSION_UTILITY
		, const char* c_szClientVersion
#endif
#ifdef ENABLE_PIN_SYSTEM
		, const char* c_szPin
#endif
	);
#ifdef ENABLE_CHARACTER_WINDOW_RENEWAL
	std::string GetLoginID() const { return m_stID; }
#endif
	void SetLoginKey(DWORD dwLoginKey);
	void ClearLoginInfo(void);

	void SetHandler(PyObject* poHandler);
	void SetPhaseWindow(UINT ePhaseWnd, PyObject* poPhaseWnd);
	void ClearPhaseWindow(UINT ePhaseWnd, PyObject* poPhaseWnd);
	void SetServerCommandParserWindow(PyObject* poPhaseWnd);

	bool SendSyncPositionElementPacket(DWORD dwVictimVID, DWORD dwVictimX, DWORD dwVictimY);
	bool SendAttackPacket(UINT uMotAttack, DWORD dwVIDVictim);
	bool SendCharacterStatePacket(const TPixelPosition& c_rkPPosDst, float fDstRot, UINT eFunc, UINT uArg);
	bool SendUseSkillPacket(DWORD dwSkillIndex, DWORD dwTargetVID = 0);
	bool SendTargetPacket(DWORD dwVID);
#ifdef ENABLE_SKILL_COLOR_SYSTEM
	bool SendSkillColorPacket(BYTE skill, DWORD col1, DWORD col2, DWORD col3, DWORD col4, DWORD col5);
#endif

	bool SendCharacterPositionPacket(BYTE iPosition);
#ifdef ENABLE_USE_ITEM_COUNT
	bool SendItemUsePacket(TItemPos pos, DWORD count = 1);
#else // ENABLE_USE_ITEM_COUNT
	bool SendItemUsePacket(TItemPos pos);
#endif // ENABLE_USE_ITEM_COUNT
	bool SendItemUseToItemPacket(TItemPos source_pos, TItemPos target_pos);
#ifdef ENABLE_DROP_DIALOG_SYSTEM
	bool SendItemDeletePacket(TItemPos item_pos);
	bool SendItemSellPacket(TItemPos item_pos);
#endif
#ifdef ENABLE_ITEM_COUNT_LIMIT
	bool SendItemMovePacket(TItemPos pos, TItemPos change_pos, DWORD num);
#else
	bool SendItemMovePacket(TItemPos pos, TItemPos change_pos, BYTE num);
#endif
	bool SendItemPickUpPacket(DWORD vid);

	bool SendQuickSlotAddPacket(BYTE wpos, BYTE type, BYTE pos);
	bool SendQuickSlotDelPacket(BYTE wpos);
	bool SendQuickSlotMovePacket(BYTE wpos, BYTE change_pos);

	bool SendPointResetPacket();

	// Shop
	bool SendShopEndPacket();
	bool SendShopBuyPacket(BYTE byCount);
	bool SendShopSellPacket(BYTE bySlot);
#ifdef ENABLE_ADDITIONAL_INVENTORY
#ifdef ENABLE_ITEM_COUNT_LIMIT
	bool SendShopSellPacketNew(WORD wSlot, DWORD byCount, BYTE byType);
#else
	bool SendShopSellPacketNew(WORD wSlot, BYTE byCount, BYTE byType);
#endif
#else
#ifdef ENABLE_ITEM_COUNT_LIMIT
	bool SendShopSellPacketNew(WORD bySlot, DWORD byCount);
#else
	bool SendShopSellPacketNew(WORD bySlot, BYTE byCount);
#endif
#endif

	// Exchange
	bool SendExchangeStartPacket(DWORD vid);
	bool SendExchangeItemAddPacket(TItemPos ItemPos, BYTE byDisplayPos);
#ifdef ENABLE_GOLD_LIMIT_REWORK
	bool SendExchangeElkAddPacket(long long elk);
#else
	bool SendExchangeElkAddPacket(DWORD elk);
#endif
#ifdef ENABLE_CHEQUE_SYSTEM
	bool SendExchangeChequeAddPacket(DWORD cheque);
#endif
	bool SendExchangeItemDelPacket(BYTE pos);
	bool SendExchangeAcceptPacket();
	bool SendExchangeExitPacket();

	// Quest
	bool SendScriptAnswerPacket(int iAnswer);
	bool SendScriptButtonPacket(unsigned int iIndex);
	bool SendAnswerMakeGuildPacket(const char* c_szName);
	bool SendQuestInputStringPacket(const char* c_szString);
	bool SendQuestConfirmPacket(BYTE byAnswer, DWORD dwPID);

	// Event
	bool SendOnClickPacket(DWORD vid);

	// Fly
	bool SendFlyTargetingPacket(DWORD dwTargetVID, const TPixelPosition& kPPosTarget);
	bool SendAddFlyTargetingPacket(DWORD dwTargetVID, const TPixelPosition& kPPosTarget);
	bool SendShootPacket(UINT uSkill);

	// Command
	bool ClientCommand(const char* c_szCommand);
	void ServerCommand(char* c_szCommand);

	// Emoticon
	void RegisterEmoticonString(const char* pcEmoticonString);

	// Party
	bool SendPartyInvitePacket(DWORD dwVID);
	bool SendPartyInviteAnswerPacket(DWORD dwLeaderVID, BYTE byAccept);
	bool SendPartyRemovePacket(DWORD dwPID);
	bool SendPartySetStatePacket(DWORD dwVID, BYTE byState, BYTE byFlag);
	bool SendPartyParameterPacket(BYTE byDistributeMode);

	// SafeBox
	bool SendSafeBoxCheckinPacket(TItemPos InventoryPos, BYTE bySafeBoxPos);
	bool SendSafeBoxCheckoutPacket(BYTE bySafeBoxPos, TItemPos InventoryPos);
#ifdef ENABLE_ITEM_COUNT_LIMIT
	bool SendSafeBoxItemMovePacket(BYTE bySourcePos, BYTE byTargetPos, DWORD byCount);
#else
	bool SendSafeBoxItemMovePacket(BYTE bySourcePos, BYTE byTargetPos, BYTE byCount);
#endif

	// Mall
	bool SendMallCheckoutPacket(BYTE byMallPos, TItemPos InventoryPos);

	// Guild
	bool SendGuildAddMemberPacket(DWORD dwVID);
	bool SendGuildRemoveMemberPacket(DWORD dwPID);
	bool SendGuildChangeGradeNamePacket(BYTE byGradeNumber, const char* c_szName);
	bool SendGuildChangeGradeAuthorityPacket(BYTE byGradeNumber, BYTE byAuthority);
	bool SendGuildOfferPacket(DWORD dwExperience);
	bool SendGuildPostCommentPacket(const char* c_szMessage);
	bool SendGuildDeleteCommentPacket(DWORD dwIndex);
	bool SendGuildRefreshCommentsPacket(DWORD dwHighestIndex);
	bool SendGuildChangeMemberGradePacket(DWORD dwPID, BYTE byGrade);
	bool SendGuildUseSkillPacket(DWORD dwSkillID, DWORD dwTargetVID);
	bool SendGuildChangeMemberGeneralPacket(DWORD dwPID, BYTE byFlag);
	bool SendGuildInviteAnswerPacket(DWORD dwGuildID, BYTE byAnswer);
#ifdef ENABLE_GOLD_LIMIT_REWORK
	bool SendGuildChargeGSPPacket(long long llMoney);
	bool SendGuildDepositMoneyPacket(long long llMoney);
	bool SendGuildWithdrawMoneyPacket(long long llMoney);
#else
	bool SendGuildChargeGSPPacket(DWORD dwMoney);
	bool SendGuildDepositMoneyPacket(DWORD dwMoney);
	bool SendGuildWithdrawMoneyPacket(DWORD dwMoney);
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
	bool PetSetNamePacket(const char* petname);
#endif
#ifdef ENABLE_GROWTH_MOUNT_SYSTEM
	bool MountSetNamePacket(const char* mountName);
#endif
	// Mall
	bool RecvMallOpenPacket();
	bool RecvMallItemSetPacket();
	bool RecvMallItemDelPacket();
#ifdef ENABLE_OFFLINE_SHOP
	bool RecvOfflineshopPacket();

	bool RecvOfflineshopShopOpen();
	bool RecvOfflineshopShopOpenOwner();
	bool RecvOfflineshopShopOpenOwnerNoShop();
	bool RecvOfflineshopShopClose();
	bool RecvOfflineshopShopFilterResult();
	bool RecvOfflineshopShopSafeboxRefresh();
	bool RecvOfflineshopShopBuyItemFromSearch();

#ifdef ENABLE_OFFLINE_SHOP_CITIES
	bool RecvOfflineshopInsertEntity();
	bool RecvOfflineshopRemoveEntity();

	void SendOfflineshopOnClickShopEntity(DWORD dwPickedShopVID);
#endif

	bool RecvOfflineshopBuyResult();
	bool RecvOfflineshopCreateSuccess();
	bool RecvOfflineshopPopup();

	void SendOfflineshopShopCreate(const offlineshop::TShopInfo& shopInfo, const std::vector<offlineshop::TShopItemInfo>& items);
	void SendOfflineshopForceCloseShop();

	void SendOfflineshopOpenShop(DWORD dwOwnerID);
	void SendOfflineshopOpenShopOwner();

	void SendOfflineshopBuyItem(DWORD dwOwnerID, DWORD dwItemID, bool isSearch, const offlineshop::TPriceInfo& price); //fix-edit-price

	void SendOfflineshopAddItem(offlineshop::TShopItemInfo& itemInfo);
	void SendOfflineshopRemoveItem(DWORD dwItemID);

	void SendOfflineshopFilterRequest(const offlineshop::TFilterInfo& filter);

	void SendOfflineshopSafeboxOpen();
	void SendOfflineshopSafeboxGetItem(DWORD dwItemID);
	void SendOfflineshopSafeboxGetValutes(const offlineshop::TValutesInfo& valutes);
	void SendOfflineshopSafeboxClose();

	void SendOfflineshopCloseBoard();
	void SendOfflineShopTeleport();
#endif

	// Dig
	bool RecvDigMotionPacket();

	// Fishing
	bool SendFishingPacket(int iRotation);
	bool SendGiveItemPacket(DWORD dwTargetVID, TItemPos ItemPos, int iItemCount);

	// Private Shop
	bool SendBuildPrivateShopPacket(const char* c_szName, const std::vector<TShopItemTable>& c_rSellingItemStock);

	// Refine
	bool SendRefinePacket(BYTE byPos, BYTE byType);
	bool SendSelectItemPacket(DWORD dwItemPos);

	// Client Version
	bool SendDragonSoulRefinePacket(BYTE bRefineType, TItemPos* pos);

	// Handshake
	bool RecvHandshakePacket();
	bool RecvHandshakeOKPacket();

#ifdef ENABLE_IMPROVED_PACKET_ENCRYPTION
	bool RecvKeyAgreementPacket();
	bool RecvKeyAgreementCompletedPacket();
#endif

	// ETC
	DWORD GetMainActorVID();
	DWORD GetMainActorRace();
	DWORD GetMainActorEmpire();
	DWORD GetMainActorSkillGroup();
	void SetEmpireID(DWORD dwEmpireID);
	DWORD GetEmpireID();

#ifdef ENABLE_ACCE_SYSTEM
	bool	SendAcceClosePacket();
	bool	SendAcceAddPacket(TItemPos tPos, BYTE bPos);
	bool	SendAcceRemovePacket(BYTE bPos);
	bool	SendAcceRefinePacket();
#endif
#ifdef ENABLE_CHEST_INFO_SYSTEM
	bool	SendChestDropInfo(DWORD dwChestVnum, TItemPos pos);
	bool	RecvChestDropInfo();
#endif
#ifdef ENABLE_EVENT_SYSTEM
	bool RecvEventInfo();
#endif
#ifdef ENABLE_SKILL_CHOOSE_SYSTEM
protected:
	bool	RecvSkillChoose();
public:
	bool	SendSkillChooseInfo(int data);
#endif
#ifdef ENABLE_CHANGELOOK_SYSTEM
public:
	bool SendClClosePacket();
	bool SendClAddPacket(TItemPos tPos, BYTE bPos);
	bool SendClRemovePacket(BYTE bPos);
	bool SendClRefinePacket();
protected:
	bool RecvChangeLookPacket();
#endif
#ifdef ENABLE_DUNGEON_INFO_SYSTEM
public:
	bool DungeonTeleport(BYTE dungeonIndex);
#endif
#ifdef ENABLE_CUBE_RENEWAL
public:
	bool CubeRenewalMakeItem(int index_item, int count_item, int index_item_improve);
	bool CubeRenewalClose();
	bool RecvCubeRenewalPacket();
#endif
#ifdef ENABLE_FAST_CHEQUE_TRANSFER
public:
	bool SendWonExchangeSellPacket(DWORD wValue);
	bool SendWonExchangeBuyPacket(DWORD wValue);
#endif
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
public:
#ifdef ENABLE_ANCIENT_ATTR_SYSTEM
	bool SendItemNewAttributePacket(TItemPos source_pos, TItemPos target_pos, BYTE* bValues);
#endif
	void SetOffLinePhase();
	void SetHandShakePhase();
	void SetLoginPhase();
	void SetSelectPhase();
	void SetLoadingPhase();
	void SetGamePhase();
	void ClosePhase();

	// Login Phase
	bool SendLoginPacketNew(const char* c_szName, const char* c_szPassword);
	bool SendEnterGame();

	// Select Phase
	bool SendSelectEmpirePacket(DWORD dwEmpireID);
	bool SendSelectCharacterPacket(BYTE account_Index);
	bool SendChangeNamePacket(BYTE index, const char* name);
	bool SendCreateCharacterPacket(BYTE index, const char* name, BYTE job, BYTE shape);
	bool SendDestroyCharacterPacket(BYTE index, const char* szPrivateCode);

	// Main Game Phase
	bool SendChatPacket(const char* c_szChat, BYTE byType = CHAT_TYPE_TALKING);
#ifdef ENABLE_GLOBAL_MESSAGE_UTILITY
	bool SendBulkWhisperPacket(const char* c_szContent);
#endif

	bool SendWhisperPacket(const char* name, const char* c_szChat);
	bool SendMessengerAddByVIDPacket(DWORD vid);
	bool SendMessengerAddByNamePacket(const char* c_szName);
	bool SendMessengerRemovePacket(const char* c_szKey, const char* c_szName);

protected:
	bool OnProcess();
	void OffLinePhase();
	void HandShakePhase();
	void LoginPhase();
	void SelectPhase();
	void LoadingPhase();
	void GamePhase();

	void __DownloadMark();

	void __PlayInventoryItemUseSound(TItemPos uSlotPos);
	void __PlayInventoryItemDropSound(TItemPos uSlotPos);
	//void __PlayShopItemDropSound(UINT uSlotPos);
	void __PlaySafeBoxItemDropSound(UINT uSlotPos);
	void __PlayMallItemDropSound(UINT uSlotPos);

	bool __CanActMainInstance();

	enum REFRESH_WINDOW_TYPE
	{
		RefreshStatus = (1 << 0),
		RefreshAlignmentWindow = (1 << 1),
		RefreshCharacterWindow = (1 << 2),
		RefreshEquipmentWindow = (1 << 3),
		RefreshInventoryWindow = (1 << 4),
		RefreshExchangeWindow = (1 << 5),
		RefreshSkillWindow = (1 << 6),
		RefreshSafeboxWindow = (1 << 7),
		RefreshMessengerWindow = (1 << 8),
		RefreshGuildWindowInfoPage = (1 << 9),
		RefreshGuildWindowBoardPage = (1 << 10),
		RefreshGuildWindowMemberPage = (1 << 11),
		RefreshGuildWindowMemberPageGradeComboBox = (1 << 12),
		RefreshGuildWindowSkillPage = (1 << 13),
		RefreshGuildWindowGradePage = (1 << 14),
		RefreshTargetBoard = (1 << 15),
		RefreshMallWindow = (1 << 16),
#ifdef ENABLE_ADDITIONAL_INVENTORY
		RefreshAdditionalInventoryWindow = (1 << 17),
		RefreshOtherInventoryWindow = (1 << 18),
#endif
	};

	void __RefreshStatus();
	void __RefreshAlignmentWindow();
	void __RefreshCharacterWindow();
	void __RefreshEquipmentWindow();
#ifdef ENABLE_ADDITIONAL_INVENTORY
	void __RefreshInventoryWindow(BYTE bWindowType = 0);
#else
	void __RefreshInventoryWindow();
#endif
	void __RefreshExchangeWindow();
	void __RefreshSkillWindow();
	void __RefreshSafeboxWindow();
	void __RefreshMessengerWindow();
	void __RefreshGuildWindowInfoPage();
	void __RefreshGuildWindowBoardPage();
	void __RefreshGuildWindowMemberPage();
	void __RefreshGuildWindowMemberPageGradeComboBox();
	void __RefreshGuildWindowSkillPage();
	void __RefreshGuildWindowGradePage();
	void __RefreshTargetBoardByVID(DWORD dwVID);
	void __RefreshTargetBoardByName(const char* c_szName);
	void __RefreshTargetBoard();
	void __RefreshMallWindow();
	void __RefreshCostumeWindow();
	void __RefreshGameOptionDlg();

	bool __SendHack(const char* c_szMsg);

protected:
	// Common
	bool RecvErrorPacket(int header);
	bool RecvPingPacket();
	bool RecvDefaultPacket(int header);
	bool RecvPhasePacket();

	// Login Phase
	bool __RecvLoginSuccessPacket4();
	bool __RecvLoginFailurePacket();
	bool __RecvEmpirePacket();

	// Select Phase
	bool __RecvPlayerCreateSuccessPacket();
	bool __RecvPlayerCreateFailurePacket();
	bool __RecvPlayerDestroySuccessPacket();
	bool __RecvPlayerDestroyFailurePacket();
	bool __RecvPlayerPoints();
	bool __RecvChangeName();

	// Loading Phase
	bool RecvMainCharacter();

	void __SetFieldMusicFileName(const char* musicName);
	void __SetFieldMusicFileInfo(const char* musicName, float vol);
	// END_OF_SUPPORT_BGM

	// Main Game Phase
	bool RecvWarpPacket();
	bool RecvPVPPacket();
	bool RecvCharacterAppendPacket();
	bool RecvCharacterAdditionalInfo();
	bool RecvCharacterUpdatePacket();
	bool RecvCharacterDeletePacket();
	bool RecvChatPacket();
#ifdef ENABLE_GLOBAL_MESSAGE_UTILITY
	bool RecvBulkWhisperPacket();
#endif
	bool RecvOwnerShipPacket();
	bool RecvSyncPositionPacket();
	bool RecvWhisperPacket();
	bool RecvPointChange();					// Alarm to python

#ifdef ENABLE_INGAME_MALL_SYSTEM
	bool RecvItemShopData();
#endif // ENABLE_INGAME_MALL_SYSTEM

	bool RecvStunPacket();
	bool RecvDeadPacket();
	bool RecvCharacterMovePacket();

	bool RecvItemDelPacket();					// Alarm to python
	bool RecvItemSetPacket();					// Alarm to python
	bool RecvItemUsePacket();					// Alarm to python
	bool RecvItemUpdatePacket();				// Alarm to python
	bool RecvItemGroundAddPacket();
	bool RecvItemGroundDelPacket();
	bool RecvItemOwnership();
	bool RecvQuickSlotAddPacket();				// Alarm to python
	bool RecvQuickSlotDelPacket();				// Alarm to python
	bool RecvQuickSlotMovePacket();				// Alarm to python

	bool RecvCharacterPositionPacket();
	bool RecvMotionPacket();

	bool RecvShopPacket();
	bool RecvShopSignPacket();
	bool RecvExchangePacket();

	// Quest
	bool RecvScriptPacket();
	bool RecvQuestInfoPacket();
	bool RecvQuestConfirmPacket();
	bool RecvRequestMakeGuild();

	// Skill
	bool RecvSkillLevelNew();

	// Target
	bool RecvTargetPacket();
	bool RecvDamageInfoPacket();

	// Fly
	bool RecvCreateFlyPacket();
	bool RecvFlyTargetingPacket();
	bool RecvAddFlyTargetingPacket();

	// Messenger
	bool RecvMessenger();

	// Guild
	bool RecvGuild();

	// Party
	bool RecvPartyInvite();
	bool RecvPartyAdd();
	bool RecvPartyUpdate();
	bool RecvPartyRemove();
	bool RecvPartyLink();
	bool RecvPartyUnlink();
	bool RecvPartyParameter();

	// SafeBox
	bool RecvSafeBoxSetPacket();
	bool RecvSafeBoxDelPacket();
	bool RecvSafeBoxWrongPasswordPacket();
	bool RecvSafeBoxSizePacket();

	// Fishing
	bool RecvFishing();

	// Dungeon
	bool RecvDungeon();

	// Time
	bool RecvTimePacket();

	// WalkMode
	bool RecvWalkModePacket();

	// ChangeSkillGroup
	bool RecvChangeSkillGroupPacket();

	// Refine
	bool RecvRefineInformationPacketNew();

	// Use Potion
	bool RecvSpecialEffect();

	bool RecvSpecificEffect();

	bool RecvDragonSoulRefine();

	// MiniMap Info
	bool RecvTargetCreatePacketNew();
	bool RecvTargetUpdatePacket();
	bool RecvTargetDeletePacket();

	// Affect
	bool RecvAffectAddPacket();
	bool RecvAffectRemovePacket();

	// Channel
	bool RecvChannelPacket();

	bool RecvUnk213();

#ifdef ENABLE_ACCE_SYSTEM
	bool RecvAccePacket(bool bReturn = false);
#endif

#ifdef ENABLE_PLAYER_STATISTICS
	bool RecvPlayerStatistics();
#endif

protected:
	bool ParseEmoticon(const char* pChatMsg, DWORD* pdwEmoticon);

	void OnConnectFailure();
	void OnScriptEventStart(int iSkin, int iIndex);

	void OnRemoteDisconnect();
	void OnDisconnect();

	void SetGameOnline();
	void SetGameOffline();
	BOOL IsGameOnline();

protected:
	bool CheckPacket(TPacketHeader* pRetHeader);

	void __InitializeGamePhase();
	void __InitializeMarkAuth();
	void __GlobalPositionToLocalPosition(LONG& rGlobalX, LONG& rGlobalY);
	void __LocalPositionToGlobalPosition(LONG& rLocalX, LONG& rLocalY);

	bool __IsPlayerAttacking();
	bool __IsEquipItemInSlot(TItemPos Cell);

	void __ShowMapName(LONG lLocalX, LONG lLocalY);

	void __LeaveOfflinePhase() {}
	void __LeaveHandshakePhase() {}
	void __LeaveLoginPhase() {}
	void __LeaveSelectPhase() {}
	void __LeaveLoadingPhase() {}
	void __LeaveGamePhase();

	void __ClearNetworkActorManager();

	void __ClearSelectCharacterData();

	// DELETEME
	//void __SendWarpPacket();

	void __RecvCharacterAppendPacket(SNetworkActorData* pkNetActorData);
	void __RecvCharacterUpdatePacket(SNetworkUpdateActorData* pkNetUpdateActorData);

	void __SetGuildID(DWORD id);

#ifdef ENABLE_CHANNEL_INFO_UPDATE
public:
	void SetChannel(BYTE bChannel) { m_bNetChannel = bChannel; }
	BYTE GetChannel() { return m_bNetChannel; }
protected:
	BYTE m_bNetChannel;
#endif

protected:
	TPacketGCHandshake m_HandshakeData;
	DWORD m_dwChangingPhaseTime;
	DWORD m_dwBindupRetryCount;
	DWORD m_dwMainActorVID;
	DWORD m_dwMainActorRace;
	DWORD m_dwMainActorEmpire;
	DWORD m_dwMainActorSkillGroup;
	BOOL m_isGameOnline;
	BOOL m_isStartGame;

	DWORD m_dwGuildID;
	DWORD m_dwEmpireID;

	struct SServerTimeSync
	{
		DWORD m_dwChangeServerTime;
		DWORD m_dwChangeClientTime;
	} m_kServerTimeSync;

	void __ServerTimeSync_Initialize();
	//DWORD m_dwBaseServerTime;
	//DWORD m_dwBaseClientTime;

	DWORD m_dwLastGamePingTime;

	std::string	m_stID;
	std::string	m_stPassword;
#ifdef ENABLE_CLIENT_VERSION_UTILITY
	std::string	m_stClientVersion;
#endif
#ifdef ENABLE_PIN_SYSTEM
	std::string	m_stPin;
#endif
	std::string	m_strLastCommand;
	std::string	m_strPhase;
	DWORD m_dwLoginKey;
	BOOL m_isWaitLoginKey;

	std::string m_stMarkIP;

	CFuncObject<CPythonNetworkStream>	m_phaseProcessFunc;
	CFuncObject<CPythonNetworkStream>	m_phaseLeaveFunc;

	PyObject* m_poHandler;
	PyObject* m_apoPhaseWnd[PHASE_WINDOW_NUM];
	PyObject* m_poSerCommandParserWnd;

	TSimplePlayerInformation			m_akSimplePlayerInfo[PLAYER_PER_ACCOUNT4];
	DWORD								m_adwGuildID[PLAYER_PER_ACCOUNT4];
	std::string							m_astrGuildName[PLAYER_PER_ACCOUNT4];
	bool m_bSimplePlayerInfo;

	CRef<CNetworkActorManager>			m_rokNetActorMgr;

	bool m_isRefreshStatus;
	bool m_isRefreshCharacterWnd;
	bool m_isRefreshEquipmentWnd;
	bool m_isRefreshInventoryWnd;
	bool m_isRefreshExchangeWnd;
	bool m_isRefreshSkillWnd;
	bool m_isRefreshSafeboxWnd;
	bool m_isRefreshMallWnd;
	bool m_isRefreshMessengerWnd;
	bool m_isRefreshGuildWndInfoPage;
	bool m_isRefreshGuildWndBoardPage;
	bool m_isRefreshGuildWndMemberPage;
	bool m_isRefreshGuildWndMemberPageGradeComboBox;
	bool m_isRefreshGuildWndSkillPage;
	bool m_isRefreshGuildWndGradePage;
#ifdef ENABLE_ADDITIONAL_INVENTORY
	bool m_isRefreshAdditionalInventoryWnd;
	bool m_isRefreshOtherWnd;
#endif

	// Emoticon
	std::vector<std::string> m_EmoticonStringVector;

	struct STextConvertTable
	{
		char acUpper[26];
		char acLower[26];
		BYTE aacHan[5000][2];
	} m_aTextConvTable[3];

	struct SMarkAuth
	{
		CNetworkAddress m_kNetAddr;
		DWORD m_dwHandle;
		DWORD m_dwRandomKey;
	} m_kMarkAuth;

	DWORD m_dwSelectedCharacterIndex;

	bool m_bComboSkillFlag;

	std::deque<std::string> m_kQue_stHack;

private:
	struct SDirectEnterMode
	{
		bool m_isSet;
		DWORD m_dwChrSlotIndex;
	} m_kDirectEnterMode;

	void __DirectEnterMode_Initialize();

	void __DirectEnterMode_Set(UINT uChrSlotIndex);
	bool __DirectEnterMode_IsSet();
	DWORD __DirectEnterMode__GetChrSlotIndex();

public:
	DWORD EXPORT_GetBettingGuildWarValue(const char* c_szValueName);
private:
	struct SBettingGuildWar
	{
#ifdef ENABLE_GOLD_LIMIT_REWORK
		long long m_llBettingMoney;
#else
		DWORD m_dwBettingMoney;
#endif
		DWORD m_dwObserverCount;
	} m_kBettingGuildWar;

	CInstanceBase* m_pInstTarget;

	void __BettingGuildWar_Initialize();
	void __BettingGuildWar_SetObserverCount(UINT uObserverCount);
#ifdef ENABLE_GOLD_LIMIT_REWORK
	void __BettingGuildWar_SetBettingMoney(long long uBettingMoney);
#else
	void __BettingGuildWar_SetBettingMoney(UINT uBettingMoney);
#endif
#ifdef ENABLE_SWITCHBOT_SYSTEM
public:
	bool RecvSwitchbotPacket();

	bool SendSwitchbotStartPacket(BYTE slot, std::vector<CPythonSwitchbot::TSwitchbotAttributeAlternativeTable> alternatives);
	bool SendSwitchbotStopPacket(BYTE slot);
	bool SendSwitchbotChangeSpeed(BYTE slot);
#endif
#ifdef ENABLE_TARGET_BOARD_RENEWAL
public:
	bool			RecvMobInfo();
	void			SendMobInfoPacket(DWORD dwMobVnum, DWORD dwVirtualID, BYTE bType);
#endif

#ifdef ENABLE_CHAT_DISCREDIT
private:
	std::map<std::string, std::string> m_DiscreditMap;
	BYTE m_discreditMode;
public:
	BYTE GetDiscreditMode() { return m_discreditMode; }
	bool LoadDiscreditFile(const char *szFileName);
	void ControlDiscredit(std::string &strchat);
#endif
#ifdef ENABLE_BATTLE_PASS_SYSTEM
public:
	bool SendBattlePassAction(BYTE bAction, BYTE bSubAction);

protected:
	bool RecvBattlePassPacket();
	bool RecvBattlePassRankingPacket();
	bool RecvBattlePassUpdatePacket();
#endif
#ifdef ENABLE_AUTO_HUNT_SYSTEM
public:
	void SetCikiyorumStatus(bool f) { m_bCikiyorum = f; };
	bool IsCikiyorumStatus() const { return m_bCikiyorum; };
protected:
	bool m_bCikiyorum;
#endif
};
