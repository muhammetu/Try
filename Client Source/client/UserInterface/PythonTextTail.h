#pragma once

#include "../eterBase/Singleton.h"
#ifdef ENABLE_GRAPHIC_OPTIMIZATION
#include "../GameLib/ActorInstance.h"
#endif

#ifdef ENABLE_GRAPHIC_OPTIMIZATION
const float c_fEnemyTextTailDistance = 1250.0f;
const float c_fNPCTextTailDistance = 1250.0f;
const float c_fStoneTextTailDistance = 3500.0f;
const float c_fWarpTextTailDistance = 3500.0f;
const float c_fDoorTextTailDistance = 3500.0f;
const float c_fBuildingTextTailDistance = 3500.0f;
const float c_fPlayerTextTailDistance = 2750.0f;
const float c_fPolyTextTailDistance = 2750.0f;
const float c_fHorseTextTailDistance = 500.0f;
const float c_fGotoTextTailDistance = 3500.0f;
const float c_fBossTextTailDistance = 3500.0f;
const float c_fPetTextTailDistance = 500.0f;
const float c_fMountTextTailDistance = 500.0f;
#endif

class CPythonTextTail : public CSingleton<CPythonTextTail>
{
public:
	typedef struct STextTail
	{
		CGraphicTextInstance* pTextInstance;
		CGraphicTextInstance* pOwnerTextInstance;

		CGraphicMarkInstance* pMarkInstance;
		CGraphicTextInstance* pGuildNameTextInstance;

		CGraphicTextInstance* pTitleTextInstance;
		CGraphicTextInstance* pLevelTextInstance;
#ifdef ENABLE_RANK_SYSTEM
		CGraphicImageInstance* pRankImageInstance;
		CGraphicImageInstance* pRankImageInstance2;
#endif
#ifdef ENABLE_LANDRANK_SYSTEM
		CGraphicTextInstance* pLandRankTextInstance;
#endif
#ifdef ENABLE_REBORN_SYSTEM
		CGraphicTextInstance* pRebornTextInstance;
#endif
#ifdef ENABLE_TEAM_SYSTEM
		CGraphicImageInstance* pTeamImageInstance;
		CGraphicTextInstance* pTeamTextInstance;
#endif
#ifdef ENABLE_MONIKER_SYSTEM
		CGraphicTextInstance* pMonikerTextInstance;
#endif
#ifdef ENABLE_LOVE_SYSTEM
		CGraphicImageInstance* pLoveImageInstance;
		CGraphicTextInstance* pLove1TextInstance;
		CGraphicTextInstance* pLove2TextInstance;
#endif // ENABLE_LOVE_SYSTEM
#ifdef ENABLE_WORD_SYSTEM
		CGraphicTextInstance* pWordTextInstance;
#endif

#if defined(ENABLE_MOB_AGGR_LVL_INFO)
		CGraphicTextInstance* pAIFlagTextInstance;
#endif
		CGraphicObjectInstance* pOwner;

		DWORD							dwVirtualID;

		float							x, y, z;
		float							fDistanceFromPlayer;
		D3DXCOLOR						Color;
		BOOL							bNameFlag;

		float							xStart, yStart;
		float							xEnd, yEnd;

		DWORD							LivingTime;

		float							fHeight;

#if defined(ENABLE_MOB_AGGR_LVL_INFO)
		BOOL							bIsPC;
#endif
#ifdef ENABLE_OFFLINE_SHOP
#ifdef ENABLE_OFFLINE_SHOP_CITIES
		bool							bIsShop;
		bool							bRender;
#endif
#endif
#ifdef ENABLE_GRAPHIC_OPTIMIZATION
		/*mutable*/BYTE							instanceType;

		/*const*/ const bool IsEnemy() { return instanceType == CActorInstance::TYPE_ENEMY; }
		/*const*/ const bool IsNPC() { return instanceType == CActorInstance::TYPE_NPC; }
		/*const*/ const bool IsStone() { return instanceType == CActorInstance::TYPE_STONE; }
		/*const*/ const bool IsWarp() { return instanceType == CActorInstance::TYPE_WARP; }
		/*const*/ const bool IsDoor() { return instanceType == CActorInstance::TYPE_DOOR; }
		/*const*/ const bool IsPC() { return instanceType == CActorInstance::TYPE_PC; }
		/*const*/ const bool IsPoly() { return instanceType == CActorInstance::TYPE_POLY; }
		/*const*/ const bool IsHorse() { return instanceType == CActorInstance::TYPE_HORSE; }
		/*const*/ const bool IsGoto() { return instanceType == CActorInstance::TYPE_GOTO; }

		/*const*/ const bool GetInstanceType() { return instanceType; }

		/*const*/ const float GetDistanceLimit()
		{
			if (IsEnemy()) { return c_fEnemyTextTailDistance; }
			else if (IsNPC()) { return c_fNPCTextTailDistance; }
			else if (IsStone()) { return c_fStoneTextTailDistance; }
			else if (IsWarp()) { return c_fWarpTextTailDistance; }
			else if (IsDoor()) { return c_fDoorTextTailDistance; }
			else if (IsPC()) { return c_fPlayerTextTailDistance; }
			else if (IsPoly()) { return c_fPolyTextTailDistance; }
			else if (IsHorse()) { return c_fHorseTextTailDistance; }
			else if (IsGoto()) { return c_fGotoTextTailDistance; }
			else return 3500.0f;
		}
#endif
		STextTail() {}
		virtual ~STextTail() {}
	} TTextTail;

	typedef std::map<DWORD, TTextTail*>		TTextTailMap;
	typedef std::list<TTextTail*>			TTextTailList;
	typedef TTextTailMap					TChatTailMap;

public:
	CPythonTextTail(void);
	virtual ~CPythonTextTail(void);

	void GetInfo(std::string* pstInfo);

	void Initialize();
	void Destroy();
	void Clear();

	void UpdateAllTextTail();
	void UpdateShowingTextTail();
	void Render();

	void ArrangeTextTail();
	void HideAllTextTail();
#ifdef ENABLE_GRAPHIC_OPTIMIZATION
	bool CanShowCharacterTextTail(TTextTail * pTextTail);
	bool CanShowItemTextTail(TTextTail * pTextTail, bool picked);
#endif
	void ShowAllTextTail();
	void ShowCharacterTextTail(DWORD VirtualID);
	void ShowItemTextTail(DWORD VirtualID);

	void RegisterCharacterTextTail(DWORD dwGuildID, DWORD dwVirtualID, const D3DXCOLOR& c_rColor, float fAddHeight = 10.0f);
	void RegisterItemTextTail(DWORD VirtualID, const char* c_szText, CGraphicObjectInstance* pOwner);
	void RegisterChatTail(DWORD VirtualID, const char* c_szChat);
	void RegisterInfoTail(DWORD VirtualID, const char* c_szChat);
	void SetCharacterTextTailColor(DWORD VirtualID, const D3DXCOLOR& c_rColor);
	void SetItemTextTailOwner(DWORD dwVID, const char* c_szName);
	void DeleteCharacterTextTail(DWORD VirtualID);
	void DeleteItemTextTail(DWORD VirtualID);

#ifdef ENABLE_OFFLINE_SHOP_CITIES
	void RegisterShopInstanceTextTail(DWORD dwVirtualID, const char* c_szName, CGraphicObjectInstance* pOwner);
	void DeleteShopTextTail(DWORD VirtualID);
	TTextTail* RegisterShopTextTail(DWORD dwVirtualID, const char* c_szText, CGraphicObjectInstance* pOwner);
	bool GetPickedNewShop(DWORD* pdwVID);
#endif
	int Pick(int ixMouse, int iyMouse);
	void SelectItemName(DWORD dwVirtualID);

	bool GetTextTailPosition(DWORD dwVID, float* px, float* py, float* pz);
	bool IsChatTextTail(DWORD dwVID);
#ifdef ENABLE_GRAPHIC_OPTIMIZATION
	bool IsCharacterTextTail(DWORD dwVID);
	bool IsItemTextTail(DWORD dwVID);
	int GetInstanceType(DWORD dwVID);
#endif

	void EnablePKTitle(BOOL bFlag);
	void AttachTitle(DWORD dwVID, const char* c_szName, const D3DXCOLOR& c_rColor);
	void DetachTitle(DWORD dwVID);

	void AttachLevel(DWORD dwVID, const char* c_szText, const D3DXCOLOR& c_rColor);
	void DetachLevel(DWORD dwVID);
#ifdef ENABLE_RANK_SYSTEM
	void AttachRank(DWORD dwVID, short sRank);
	void DeattachRank(DWORD dwVID);
#endif
#ifdef ENABLE_LANDRANK_SYSTEM
	void AttachLandRank(DWORD dwVID, const char* c_szName);
	void DetachLandRank(DWORD dwVID);
#endif
#ifdef ENABLE_REBORN_SYSTEM
	void AttachReborn(DWORD dwVID, int reborn);
	void DetachReborn(DWORD dwVID);
#endif
#ifdef ENABLE_TEAM_SYSTEM
	void AttachTeam(DWORD dwVID, short sTeam);
	void DeattachTeam(DWORD dwVID);
#endif
#ifdef ENABLE_MONIKER_SYSTEM
	void AttachMoniker(DWORD dwVID, const char* moniker);
	void DetachMoniker(DWORD dwVID);
#endif
#ifdef ENABLE_LOVE_SYSTEM
	void AttachLove(DWORD dwVID, const char* love1, const char* love2);
	void DetachLove(DWORD dwVID);
#endif // ENABLE_LOVE_SYSTEM
#ifdef ENABLE_WORD_SYSTEM
	void AttachWord(DWORD dwVID, const char* word);
	void DetachWord(DWORD dwVID);
#endif // ENABLE_WORD_SYSTEM

protected:
	TTextTail* RegisterTextTail(DWORD dwVirtualID, const char* c_szText, CGraphicObjectInstance* pOwner, float fHeight, const D3DXCOLOR& c_rColor);
	void DeleteTextTail(TTextTail* pTextTail);

	void UpdateTextTail(TTextTail* pTextTail);
	void RenderTextTailBox(TTextTail* pTextTail);
	void RenderTextTailName(TTextTail* pTextTail);
	void UpdateDistance(const TPixelPosition& c_rCenterPosition, TTextTail* pTextTail);

	bool isIn(TTextTail* pSource, TTextTail* pTarget);

protected:
	TTextTailMap				m_CharacterTextTailMap;
	TTextTailMap				m_ItemTextTailMap;
	TChatTailMap				m_ChatTailMap;
#ifdef ENABLE_OFFLINE_SHOP_CITIES
	TTextTailMap				m_ShopTextTailMap;
#endif

	TTextTailList				m_CharacterTextTailList;
	TTextTailList				m_ItemTextTailList;

private:
	CDynamicPool<STextTail>		m_TextTailPool;
};
