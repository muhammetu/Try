//
//
#include "stdafx.h"
#include "InstanceBase.h"
#include "resource.h"
#include "PythonTextTail.h"
#include "PythonCharacterManager.h"
#include "PythonGuild.h"
#include "Locale.h"
#include "MarkManager.h"
#ifdef ENABLE_OFFLINE_SHOP_CITIES
#include "PythonApplication.h"
#endif
#ifdef ENABLE_PB2_PREMIUM_SYSTEM
#include <boost/algorithm/string.hpp>
#endif // ENABLE_PB2_PREMIUM_SYSTEM
const D3DXCOLOR c_TextTail_Player_Color = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
const D3DXCOLOR c_TextTail_Monster_Color = D3DXCOLOR(1.0f, 0.0f, 0.0f, 1.0f);
const D3DXCOLOR c_TextTail_Item_Color = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
#ifdef ENABLE_ITEM_ENTITY_UTILITY
const D3DXCOLOR c_TextTail_SpecialItem_Color = D3DXCOLOR(1.0f, 0.67f, 0.0f, 1.0f); //Golden
const D3DXCOLOR c_TextTail_Item_Red_Color = 0xFFFF6969;
#endif
const D3DXCOLOR c_TextTail_Chat_Color = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
const D3DXCOLOR c_TextTail_Info_Color = D3DXCOLOR(1.0f, 0.785f, 0.785f, 1.0f);
#ifdef ENABLE_GUILD_LEADER_SYSTEM
const D3DXCOLOR c_TextTail_Guild_Name_Color_Member = 0xFFEFD3FF;
const D3DXCOLOR c_TextTail_Guild_Name_Color_Leader = 0xFFFF6969;
const D3DXCOLOR c_TextTail_Guild_Name_Color_General = 0xFFFFAD54;
#else
const D3DXCOLOR c_TextTail_Guild_Name_Color = 0xFFEFD3FF;
#endif
#ifdef ENABLE_WORD_SYSTEM
const D3DXCOLOR c_TextTail_Word_Name_Color = 0xFF1A8CF3;
#endif // ENABLE_WORD_SYSTEM
const float c_TextTail_Name_Position = -10.0f;
const float c_fxMarkPosition = 1.5f;
const float c_fyGuildNamePosition = 15.0f;
const float c_fyMarkPosition = 15.0f + 11.0f;
BOOL bPKTitleEnable = TRUE;

#ifdef ENABLE_TEAM_SYSTEM
const std::vector<std::pair<std::string, std::pair<DWORD, DWORD>>> teamNames = {
	{"Çaykur Rizespor",	{ D3DXCOLOR(0.00f, 0.51f, 0.25f, 1.0f), D3DXCOLOR(0.00f, 0.00f, 0.00f, 1.0f) }},
	{"Fenerbahçe",		{ D3DXCOLOR(1.00f, 0.99f, 0.22f, 1.0f), D3DXCOLOR(0.00f, 0.00f, 0.00f, 1.0f) }},
	{"Göztepe",			{ D3DXCOLOR(0.96f, 0.84f, 0.04f, 1.0f), D3DXCOLOR(0.00f, 0.00f, 0.00f, 1.0f) }},
	{"Ýstanbulspor",	{ D3DXCOLOR(0.00f, 0.00f, 0.00f, 1.0f), D3DXCOLOR(1.00f, 1.00f, 1.00f, 1.0f) }},
	{"Trabzonspor",		{ D3DXCOLOR(0.07f, 0.73f, 0.91f, 1.0f), D3DXCOLOR(0.00f, 0.00f, 0.00f, 1.0f) }},
	{"Bursaspor",		{ D3DXCOLOR(0.01f, 0.51f, 0.22f, 1.0f), D3DXCOLOR(0.00f, 0.00f, 0.00f, 1.0f) }},
	{"Balýkesirspor",	{ D3DXCOLOR(0.96f, 0.00f, 0.00f, 1.0f), D3DXCOLOR(0.00f, 0.00f, 0.00f, 1.0f) }},
	{"Adanaspor",		{ D3DXCOLOR(0.97f, 0.39f, 0.00f, 1.0f), D3DXCOLOR(0.00f, 0.00f, 0.00f, 1.0f) }},
	{"Bandýrmaspor",	{ D3DXCOLOR(0.68f, 0.85f, 0.91f, 1.0f), D3DXCOLOR(0.00f, 0.00f, 0.00f, 1.0f) }},
	{"Adanademirspor",	{ D3DXCOLOR(0.13f, 0.11f, 0.42f, 1.0f), D3DXCOLOR(1.00f, 1.00f, 1.00f, 1.0f) }},
	{"Akhisarspor",		{ D3DXCOLOR(0.16f, 0.57f, 0.24f, 1.0f), D3DXCOLOR(0.00f, 0.00f, 0.00f, 1.0f) }},
	{"Alanyaspor",		{ D3DXCOLOR(0.25f, 0.61f, 0.24f, 1.0f), D3DXCOLOR(0.00f, 0.00f, 0.00f, 1.0f) }},
	{"Altay SK",		{ D3DXCOLOR(0.00f, 0.00f, 0.00f, 1.0f), D3DXCOLOR(1.00f, 1.00f, 1.00f, 1.0f) }},
	{"Antýnordu",		{ D3DXCOLOR(0.11f, 0.21f, 0.51f, 1.0f), D3DXCOLOR(1.00f, 1.00f, 1.00f, 1.0f) }},
	{"Keçiören SK",		{ D3DXCOLOR(0.39f, 0.00f, 0.39f, 1.0f), D3DXCOLOR(1.00f, 1.00f, 1.00f, 1.0f) }},
	{"Ankaraspor",		{ D3DXCOLOR(0.00f, 0.32f, 0.63f, 1.0f), D3DXCOLOR(0.00f, 0.00f, 0.00f, 1.0f) }},
	{"Antalyaspor",		{ D3DXCOLOR(0.82f, 0.27f, 0.23f, 1.0f), D3DXCOLOR(0.00f, 0.00f, 0.00f, 1.0f) }},
	{"Erzurumspor",		{ D3DXCOLOR(0.00f, 0.56f, 0.82f, 1.0f), D3DXCOLOR(0.00f, 0.00f, 0.00f, 1.0f) }},
	{"Beþiktaþ",		{ D3DXCOLOR(0.00f, 0.00f, 0.00f, 1.0f), D3DXCOLOR(1.00f, 1.00f, 1.00f, 1.0f) }},
	{"Boluspor",		{ D3DXCOLOR(0.89f, 0.17f, 0.18f, 1.0f), D3DXCOLOR(0.00f, 0.00f, 0.00f, 1.0f) }},
	{"Denizlispor",		{ D3DXCOLOR(0.00f, 0.63f, 0.30f, 1.0f), D3DXCOLOR(0.00f, 0.00f, 0.00f, 1.0f) }},
	{"Eskiþehirspor",	{ D3DXCOLOR(0.97f, 0.97f, 0.00f, 1.0f), D3DXCOLOR(0.00f, 0.00f, 0.00f, 1.0f) }},
	{"Fatihkaragümrük",	{ D3DXCOLOR(0.91f, 0.18f, 0.13f, 1.0f), D3DXCOLOR(0.00f, 0.00f, 0.00f, 1.0f) }},
	{"Gaziantep FK",	{ D3DXCOLOR(0.84f, 0.12f, 0.12f, 1.0f), D3DXCOLOR(0.00f, 0.00f, 0.00f, 1.0f) }},
	{"Gençlerbirliði",	{ D3DXCOLOR(0.89f, 0.10f, 0.13f, 1.0f), D3DXCOLOR(0.00f, 0.00f, 0.00f, 1.0f) }},
	{"Giresunspor",		{ D3DXCOLOR(0.02f, 0.38f, 0.00f, 1.0f), D3DXCOLOR(0.00f, 0.00f, 0.00f, 1.0f) }},
	{"Galatasaray",		{ D3DXCOLOR(0.64f, 0.02f, 0.19f, 1.0f), D3DXCOLOR(0.00f, 0.00f, 0.00f, 1.0f) }},
	{"Hatayspor",		{ D3DXCOLOR(0.65f, 0.13f, 0.16f, 1.0f), D3DXCOLOR(0.00f, 0.00f, 0.00f, 1.0f) }},
	{"Kasýmpaþa Spor",	{ D3DXCOLOR(0.18f, 0.29f, 0.63f, 1.0f), D3DXCOLOR(0.00f, 0.00f, 0.00f, 1.0f) }},
	{"Kayserispor",		{ D3DXCOLOR(0.96f, 0.72f, 0.08f, 1.0f), D3DXCOLOR(0.00f, 0.00f, 0.00f, 1.0f) }},
	{"Konyaspor",		{ D3DXCOLOR(0.96f, 0.96f, 0.96f, 1.0f), D3DXCOLOR(0.00f, 0.00f, 0.00f, 1.0f) }},
	{"Baþakþehir",		{ D3DXCOLOR(0.00f, 0.16f, 0.32f, 1.0f), D3DXCOLOR(1.00f, 1.00f, 1.00f, 1.0f) }},
	{"Menemenspor",		{ D3DXCOLOR(0.16f, 0.08f, 0.42f, 1.0f), D3DXCOLOR(1.00f, 1.00f, 1.00f, 1.0f) }},
	{"Ankaragücü",		{ D3DXCOLOR(0.96f, 0.88f, 0.07f, 1.0f), D3DXCOLOR(0.00f, 0.00f, 0.00f, 1.0f) }},
	{"Samsunspor",		{ D3DXCOLOR(0.97f, 0.97f, 0.97f, 1.0f), D3DXCOLOR(0.00f, 0.00f, 0.00f, 1.0f) }},
	{"Sivasspor",		{ D3DXCOLOR(0.97f, 0.97f, 0.97f, 1.0f), D3DXCOLOR(0.00f, 0.00f, 0.00f, 1.0f) }},
	{"Tuzlaspor",		{ D3DXCOLOR(0.17f, 0.20f, 0.55f, 1.0f), D3DXCOLOR(1.00f, 1.00f, 1.00f, 1.0f) }},
	{"Ümraniyespor",	{ D3DXCOLOR(0.78f, 0.00f, 0.00f, 1.0f), D3DXCOLOR(0.00f, 0.00f, 0.00f, 1.0f) }},
	{"Malatyaspor",		{ D3DXCOLOR(0.91f, 0.87f, 0.00f, 1.0f), D3DXCOLOR(0.00f, 0.00f, 0.00f, 1.0f) }},
};
#endif

// TEXTTAIL_LIVINGTIME_CONTROL
long gs_TextTail_LivingTime = 5000;

long TextTail_GetLivingTime()
{
	assert(gs_TextTail_LivingTime > 1000);
	return gs_TextTail_LivingTime;
}

void TextTail_SetLivingTime(long livingTime)
{
	gs_TextTail_LivingTime = livingTime;
}
// END_OF_TEXTTAIL_LIVINGTIME_CONTROL

CGraphicText* ms_pFont = NULL;

void CPythonTextTail::GetInfo(std::string* pstInfo)
{
	char szInfo[256];
	sprintf(szInfo, "TextTail: ChatTail %d, ChrTail (Map %d, List %d), ItemTail (Map %d, List %d), Pool %d",
		m_ChatTailMap.size(),
		m_CharacterTextTailMap.size(), m_CharacterTextTailList.size(),
		m_ItemTextTailMap.size(), m_ItemTextTailList.size(),
		m_TextTailPool.GetCapacity());

	pstInfo->append(szInfo);
}

void CPythonTextTail::UpdateAllTextTail()
{
	CInstanceBase* pInstance = CPythonCharacterManager::Instance().GetMainInstancePtr();
	if (pInstance)
	{
		TPixelPosition pixelPos;
		pInstance->NEW_GetPixelPosition(&pixelPos);

		TTextTailMap::iterator itorMap;

		for (itorMap = m_CharacterTextTailMap.begin(); itorMap != m_CharacterTextTailMap.end(); ++itorMap)
		{
			UpdateDistance(pixelPos, itorMap->second);
		}

		for (itorMap = m_ItemTextTailMap.begin(); itorMap != m_ItemTextTailMap.end(); ++itorMap)
		{
			UpdateDistance(pixelPos, itorMap->second);
		}
#ifdef ENABLE_OFFLINE_SHOP_CITIES
		for (itorMap = m_ShopTextTailMap.begin(); itorMap != m_ShopTextTailMap.end(); ++itorMap)
			UpdateDistance(pixelPos, itorMap->second);
#endif

		for (TChatTailMap::iterator itorChat = m_ChatTailMap.begin(); itorChat != m_ChatTailMap.end(); ++itorChat)
		{
			UpdateDistance(pixelPos, itorChat->second);

			if (itorChat->second->bNameFlag)
			{
				DWORD dwVID = itorChat->first;
				ShowCharacterTextTail(dwVID);
			}
		}
	}
}

void CPythonTextTail::UpdateShowingTextTail()
{
	TTextTailList::iterator itor;

	for (itor = m_ItemTextTailList.begin(); itor != m_ItemTextTailList.end(); ++itor)
	{
		UpdateTextTail(*itor);
	}

	for (TChatTailMap::iterator itorChat = m_ChatTailMap.begin(); itorChat != m_ChatTailMap.end(); ++itorChat)
	{
		UpdateTextTail(itorChat->second);
	}

#ifdef ENABLE_OFFLINE_SHOP_CITIES
	for (auto itorMap = m_ShopTextTailMap.begin(); itorMap != m_ShopTextTailMap.end(); ++itorMap)
		if (itorMap->second->bRender)
			UpdateTextTail(itorMap->second);
#endif
	for (itor = m_CharacterTextTailList.begin(); itor != m_CharacterTextTailList.end(); ++itor)
	{
		TTextTail* pTextTail = *itor;
		UpdateTextTail(pTextTail);

		TChatTailMap::iterator itor = m_ChatTailMap.find(pTextTail->dwVirtualID);
		if (m_ChatTailMap.end() != itor)
		{
			TTextTail* pChatTail = itor->second;
			if (pChatTail->bNameFlag)
			{
				pTextTail->y = pChatTail->y - 17.0f;
			}
		}
	}
}

void CPythonTextTail::UpdateTextTail(TTextTail* pTextTail)
{
	if (!pTextTail->pOwner)
		return;

	/////

	CPythonGraphic& rpyGraphic = CPythonGraphic::Instance();
	rpyGraphic.Identity();

	const D3DXVECTOR3& c_rv3Position = pTextTail->pOwner->GetPosition();
	rpyGraphic.ProjectPosition(c_rv3Position.x,
		c_rv3Position.y,
		c_rv3Position.z + pTextTail->fHeight,
		&pTextTail->x,
		&pTextTail->y,
		&pTextTail->z);

	pTextTail->x = floorf(pTextTail->x);
	pTextTail->y = floorf(pTextTail->y);

	if (pTextTail->fDistanceFromPlayer < 1300.0f)
	{
		pTextTail->z = 0.0f;
	}
	else
	{
		pTextTail->z = pTextTail->z * CPythonGraphic::Instance().GetOrthoDepth() * -1.0f;
		pTextTail->z += 10.0f;
	}
}

void CPythonTextTail::ArrangeTextTail()
{
	TTextTailList::iterator itor;
	TTextTailList::iterator itorCompare;

	DWORD dwTime = CTimer::Instance().GetCurrentMillisecond();

	for (itor = m_ItemTextTailList.begin(); itor != m_ItemTextTailList.end(); ++itor)
	{
		TTextTail* pInsertTextTail = *itor;

		int yTemp = 5;
		int LimitCount = 0;

		for (itorCompare = m_ItemTextTailList.begin(); itorCompare != m_ItemTextTailList.end();)
		{
			TTextTail* pCompareTextTail = *itorCompare;

			if (*itorCompare == *itor)
			{
				++itorCompare;
				continue;
			}

			if (LimitCount >= 20)
				break;

			if (isIn(pInsertTextTail, pCompareTextTail))
			{
				pInsertTextTail->y = (pCompareTextTail->y + pCompareTextTail->yEnd + yTemp);

				itorCompare = m_ItemTextTailList.begin();
				++LimitCount;
				continue;
			}

			++itorCompare;
		}

		if (pInsertTextTail->pOwnerTextInstance)
		{
			pInsertTextTail->pOwnerTextInstance->SetPosition(pInsertTextTail->x, pInsertTextTail->y, pInsertTextTail->z);
			pInsertTextTail->pOwnerTextInstance->Update();

			pInsertTextTail->pTextInstance->SetColor(pInsertTextTail->Color.r, pInsertTextTail->Color.g, pInsertTextTail->Color.b);
			pInsertTextTail->pTextInstance->SetPosition(pInsertTextTail->x, pInsertTextTail->y + 15.0f, pInsertTextTail->z);
			pInsertTextTail->pTextInstance->Update();
		}
		else
		{
			pInsertTextTail->pTextInstance->SetColor(pInsertTextTail->Color.r, pInsertTextTail->Color.g, pInsertTextTail->Color.b);
			pInsertTextTail->pTextInstance->SetPosition(pInsertTextTail->x, pInsertTextTail->y, pInsertTextTail->z);
			pInsertTextTail->pTextInstance->Update();
		}
	}
#ifdef ENABLE_OFFLINE_SHOP_CITIES
	for (auto itorMap = m_ShopTextTailMap.begin(); itorMap != m_ShopTextTailMap.end(); ++itorMap)
	{
		if (!itorMap->second->bRender)
			continue;

		TTextTail* pInsertTextTail = itorMap->second;

		if (pInsertTextTail->pOwnerTextInstance)
		{
			pInsertTextTail->pOwnerTextInstance->SetPosition(pInsertTextTail->x, pInsertTextTail->y, pInsertTextTail->z);
			pInsertTextTail->pOwnerTextInstance->Update();

			pInsertTextTail->pTextInstance->SetColor(pInsertTextTail->Color.r, pInsertTextTail->Color.g, pInsertTextTail->Color.b);
			pInsertTextTail->pTextInstance->SetPosition(pInsertTextTail->x, pInsertTextTail->y + 15.0f, pInsertTextTail->z);
			pInsertTextTail->pTextInstance->Update();
		}
		else
		{
			pInsertTextTail->pTextInstance->SetColor(pInsertTextTail->Color.r, pInsertTextTail->Color.g, pInsertTextTail->Color.b);
			pInsertTextTail->pTextInstance->SetPosition(pInsertTextTail->x, pInsertTextTail->y, pInsertTextTail->z);
			pInsertTextTail->pTextInstance->Update();
		}
	}
#endif

	for (itor = m_CharacterTextTailList.begin(); itor != m_CharacterTextTailList.end(); ++itor)
	{
		TTextTail* pTextTail = *itor;

		float fxAdd = 0.0f;

		CGraphicMarkInstance* pMarkInstance = pTextTail->pMarkInstance;
		CGraphicTextInstance* pGuildNameInstance = pTextTail->pGuildNameTextInstance;
#ifdef ENABLE_RANK_SYSTEM
		CGraphicImageInstance* pRankInstance = pTextTail->pRankImageInstance;
		CGraphicImageInstance* pRankInstance2 = pTextTail->pRankImageInstance2;
#endif
#ifdef ENABLE_LANDRANK_SYSTEM
		CGraphicTextInstance* pLandRankInstance = pTextTail->pLandRankTextInstance;
#endif
#ifdef ENABLE_REBORN_SYSTEM
		CGraphicTextInstance* pRebornInstance = pTextTail->pRebornTextInstance;
#endif
#ifdef ENABLE_TEAM_SYSTEM
		auto pTeamInstanceImage = pTextTail->pTeamImageInstance;
		auto pTeamInstanceText = pTextTail->pTeamTextInstance;
#endif
#ifdef ENABLE_MONIKER_SYSTEM
		CGraphicTextInstance* pMonikerInstance = pTextTail->pMonikerTextInstance;
#endif
#ifdef ENABLE_LOVE_SYSTEM
		CGraphicTextInstance* pLove1Instance = pTextTail->pLove1TextInstance;
		CGraphicTextInstance* pLove2Instance = pTextTail->pLove2TextInstance;
		CGraphicImageInstance* pLoveInstance = pTextTail->pLoveImageInstance;
#endif // ENABLE_LOVE_SYSTEM
#ifdef ENABLE_WORD_SYSTEM
		CGraphicTextInstance* pWordInstance = pTextTail->pWordTextInstance;
#endif // ENABLE_WORD_SYSTEM
		if (pMarkInstance && pGuildNameInstance)
		{
			int iWidth, iHeight;
			int iImageHalfSize = pMarkInstance->GetWidth() / 2 + c_fxMarkPosition;
			pGuildNameInstance->GetTextSize(&iWidth, &iHeight);

			pMarkInstance->SetPosition(pTextTail->x - iWidth / 2 - iImageHalfSize, pTextTail->y - c_fyMarkPosition);
			pGuildNameInstance->SetPosition(pTextTail->x + iImageHalfSize, pTextTail->y - c_fyGuildNamePosition, pTextTail->z);
#if defined(ENABLE_REBORN_SYSTEM) && defined(ENABLE_LANDRANK_SYSTEM)
			if ((pRebornInstance || pLandRankInstance))
			{
				pMarkInstance->UpdatePosition(D3DXVECTOR3(0, -15, 0));
				pGuildNameInstance->UpdatePosition(D3DXVECTOR3(0, -15, 0));
			}
#endif
#ifdef ENABLE_TEAM_SYSTEM
			if (pTeamInstanceImage || pTeamInstanceText)
			{
				pMarkInstance->UpdatePosition(D3DXVECTOR3(0, -15, 0));
				pGuildNameInstance->UpdatePosition(D3DXVECTOR3(0, -15, 0));
			}
#endif // ENABLE_MONIKER_SYSTEM
#ifdef ENABLE_MONIKER_SYSTEM
			if (pMonikerInstance)
			{
				pMarkInstance->UpdatePosition(D3DXVECTOR3(0, -15, 0));
				pGuildNameInstance->UpdatePosition(D3DXVECTOR3(0, -15, 0));
			}
#endif // ENABLE_MONIKER_SYSTEM
#ifdef ENABLE_WORD_SYSTEM
			if (pWordInstance)
			{
				pMarkInstance->UpdatePosition(D3DXVECTOR3(0, -15, 0));
				pGuildNameInstance->UpdatePosition(D3DXVECTOR3(0, -15, 0));
			}
#endif // ENABLE_WORD_SYSTEM
#ifdef ENABLE_LOVE_SYSTEM
			if (pLoveInstance)
			{
				pMarkInstance->UpdatePosition(D3DXVECTOR3(0, -15, 0));
				pGuildNameInstance->UpdatePosition(D3DXVECTOR3(0, -15, 0));
			}
#endif // ENABLE_LOVE_SYSTEM
			pGuildNameInstance->Update();
		}

		int iNameWidth, iNameHeight;
		pTextTail->pTextInstance->GetTextSize(&iNameWidth, &iNameHeight);

		CGraphicTextInstance* pTitle = pTextTail->pTitleTextInstance;
		CGraphicTextInstance* pLevel = pTextTail->pLevelTextInstance;

		if (pTitle)
		{
			int iTitleWidth, iTitleHeight;
			pTitle->GetTextSize(&iTitleWidth, &iTitleHeight);

			fxAdd = 8.0f;

			float fNamePosX, fNamePosY;
			pTextTail->pTextInstance->GetPosition(&fNamePosX, &fNamePosY);

			pTitle->SetPosition(fNamePosX - (iNameWidth / 2) - 2, pTextTail->y, pTextTail->z);
			pTitle->Update();

			if (pLevel)
			{
				int iLevelWidth, iLevelHeight;
				pLevel->GetTextSize(&iLevelWidth, &iLevelHeight);
				pLevel->SetPosition(pTextTail->x - (iNameWidth / 2) - iTitleWidth, pTextTail->y, pTextTail->z);
				pLevel->Update();
			}
		}
		else
		{
			fxAdd = 4.0f;

			if (pLevel)
			{
				int iLevelWidth, iLevelHeight;
				pLevel->GetTextSize(&iLevelWidth, &iLevelHeight);
				pLevel->SetPosition(pTextTail->x - (iNameWidth / 2), pTextTail->y, pTextTail->z);
				pLevel->Update();
			}
		}

#ifdef ENABLE_LANDRANK_SYSTEM
		if (pLandRankInstance)
		{
			int iLWidth, iLHeight;
			pLandRankInstance->GetTextSize(&iLWidth, &iLHeight);

			pLandRankInstance->SetPosition(pTextTail->x, pTextTail->y - 5 - 10, pTextTail->z);

			pLandRankInstance->Update();
		}
#endif

#ifdef ENABLE_REBORN_SYSTEM
		if (pRebornInstance)
		{
			int iRWidth, iRHeight;
			pRebornInstance->GetTextSize(&iRWidth, &iRHeight);

			if (pLandRankInstance) {
				int iLWidth, iLHeight;
				float iLPosX, iLPosY;
				pLandRankInstance->GetTextSize(&iLWidth, &iLHeight);
				pLandRankInstance->GetPosition(&iLPosX, &iLPosY);
				pRebornInstance->SetPosition(iLPosX - iRWidth + 5, iLPosY, pTextTail->z);
			}
			else
				pRebornInstance->SetPosition(pTextTail->x, pTextTail->y - 5 - 10, pTextTail->z);

			pRebornInstance->Update();
		}
#endif

#ifdef ENABLE_TEAM_SYSTEM
		if (pTeamInstanceImage && pTeamInstanceText && pLevel)
		{
			auto imagew = pTeamInstanceImage->GetWidth();
			// auto imageh = pTeamInstanceImage->GetHeight();
			int textw, texth;
			pTeamInstanceText->GetTextSize(&textw, &texth);
			auto totalw = imagew + textw + 3;

			pTeamInstanceImage->SetPosition(pTextTail->x - (totalw / 2), pTextTail->y - 15 - 10);

			auto imagePos = pTeamInstanceImage->GetPosition();

			pTeamInstanceText->SetPosition(imagePos.x + imagew + 3, pTextTail->y - 15 - 10, pTextTail->z);

			if (pRebornInstance || pLandRankInstance)
			{
				pTeamInstanceImage->UpdatePosition(0, -15);
				pTeamInstanceText->UpdatePosition(D3DXVECTOR3(0, -15, 0));
			}

			if (pMonikerInstance)
			{
				pTeamInstanceImage->UpdatePosition(0, -15);
				pTeamInstanceText->UpdatePosition(D3DXVECTOR3(0, -15, 0));
			}

			pTeamInstanceText->Update();
		}
#endif

#ifdef ENABLE_MONIKER_SYSTEM
		if (pMonikerInstance)
		{
			pMonikerInstance->SetPosition(pTextTail->x, pTextTail->y - 15, pTextTail->z);

			if (pRebornInstance || pLandRankInstance)
			{
				pMonikerInstance->UpdatePosition(D3DXVECTOR3(0, -15, 0));
			}

			pMonikerInstance->Update();
		}
#endif

#ifdef ENABLE_WORD_SYSTEM
		if (pWordInstance)
		{
			pWordInstance->SetPosition(pTextTail->x, pTextTail->y - 15, pTextTail->z);
			
			if (pRebornInstance || pLandRankInstance)
			{
				pWordInstance->UpdatePosition(D3DXVECTOR3(0, -15, 0));
			}

			if (pMonikerInstance)
			{
				pWordInstance->UpdatePosition(D3DXVECTOR3(0, -15, 0));
			}

#ifdef ENABLE_TEAM_SYSTEM
			if (pTeamInstanceImage)
			{
				pWordInstance->UpdatePosition(D3DXVECTOR3(0, -15, 0));
			}
#endif // ENABLE_TEAM_SYSTEM

			pWordInstance->Update();
		}
#endif // ENABLE_WORD_SYSTEM

#ifdef ENABLE_LOVE_SYSTEM
		if (pLoveInstance && pLove1Instance && pLove2Instance)
		{
			pLove1Instance->SetPosition(pTextTail->x - 12, pTextTail->y - 15, pTextTail->z);
			pLoveInstance->SetPosition(pTextTail->x - 9, pTextTail->y - 25);
			pLove2Instance->SetPosition(pTextTail->x + 11, pTextTail->y - 15, pTextTail->z);

			if (pRebornInstance || pLandRankInstance)
			{
				pLoveInstance->UpdatePosition(0, -15);
				pLove1Instance->UpdatePosition(D3DXVECTOR3(0, -15, 0));
				pLove2Instance->UpdatePosition(D3DXVECTOR3(0, -15, 0));
			}

			if (pMonikerInstance)
			{
				pLoveInstance->UpdatePosition(0, -15);
				pLove1Instance->UpdatePosition(D3DXVECTOR3(0, -15, 0));
				pLove2Instance->UpdatePosition(D3DXVECTOR3(0, -15, 0));
			}

#ifdef ENABLE_TEAM_SYSTEM
			if (pTeamInstanceImage)
			{
				pLoveInstance->UpdatePosition(0, -15);
				pLove1Instance->UpdatePosition(D3DXVECTOR3(0, -15, 0));
				pLove2Instance->UpdatePosition(D3DXVECTOR3(0, -15, 0));
			}
#endif // ENABLE_TEAM_SYSTEM

#ifdef ENABLE_WORD_SYSTEM
			if (pWordInstance)
			{
				pWordInstance->UpdatePosition(D3DXVECTOR3(0, -15, 0));
			}
#endif // ENABLE_WORD_SYSTEM

			pLove1Instance->Update();
			pLove2Instance->Update();
		}
#endif

#if defined(ENABLE_MOB_AGGR_LVL_INFO)
		CGraphicTextInstance* pAIFlag = pTextTail->pAIFlagTextInstance;
		if (pAIFlag)
		{
			pAIFlag->SetColor(pTextTail->Color.r, pTextTail->Color.g, pTextTail->Color.b);
			pAIFlag->SetPosition(pTextTail->x + fxAdd + (iNameWidth / 2) + 1.0f, pTextTail->y, pTextTail->z);//+1.0f is not neccesarry
			pAIFlag->Update();
		}
#endif

#ifdef ENABLE_RANK_SYSTEM
		if (pRankInstance && pRankInstance2 && pLevel)
		{
			int iLevelWidth, iLevelHeight;
			pTextTail->pLevelTextInstance->GetTextSize(&iLevelWidth, &iLevelHeight);
			float fLevelPosX, fLevelPosY;
			pTextTail->pLevelTextInstance->GetPosition(&fLevelPosX, &fLevelPosY);
			pRankInstance->SetPosition(fLevelPosX - (iLevelWidth + pRankInstance->GetWidth() + 2), pTextTail->y - 10);

			float fNamePosX, fNamePosY;
			pTextTail->pTextInstance->GetPosition(&fNamePosX, &fNamePosY);
			pRankInstance2->SetPosition(fNamePosX + iNameWidth / 2 + 2, pTextTail->y - 10);
		}
#endif


		pTextTail->pTextInstance->SetColor(pTextTail->Color.r, pTextTail->Color.g, pTextTail->Color.b);
		pTextTail->pTextInstance->SetPosition(pTextTail->x + fxAdd, pTextTail->y, pTextTail->z);
		pTextTail->pTextInstance->Update();
	}

	for (TChatTailMap::iterator itorChat = m_ChatTailMap.begin(); itorChat != m_ChatTailMap.end();)
	{
		TTextTail* pTextTail = itorChat->second;

		if (pTextTail->LivingTime < dwTime)
		{
			DeleteTextTail(pTextTail);
			itorChat = m_ChatTailMap.erase(itorChat);
			continue;
		}
		else
			++itorChat;

		pTextTail->pTextInstance->SetColor(pTextTail->Color);
		pTextTail->pTextInstance->SetPosition(pTextTail->x, pTextTail->y, pTextTail->z);
		pTextTail->pTextInstance->Update();
	}
}

void CPythonTextTail::Render()
{
	TTextTailList::iterator itor;

	for (itor = m_CharacterTextTailList.begin(); itor != m_CharacterTextTailList.end(); ++itor)
	{
		TTextTail* pTextTail = *itor;
		pTextTail->pTextInstance->Render();
		if (pTextTail->pMarkInstance && pTextTail->pGuildNameTextInstance)
		{
			pTextTail->pMarkInstance->Render();
			pTextTail->pGuildNameTextInstance->Render();
		}
		if (pTextTail->pTitleTextInstance)
		{
			pTextTail->pTitleTextInstance->Render();
		}
#if defined(ENABLE_MOB_AGGR_LVL_INFO)
		if (pTextTail->pLevelTextInstance && (pTextTail->instanceType == CActorInstance::TYPE_PC || CPythonSystem::Instance().IsShowMobLevel()))
#else
		if (pTextTail->pLevelTextInstance)
#endif
		{
			pTextTail->pLevelTextInstance->Render();
		}
#ifdef ENABLE_RANK_SYSTEM
		if (pTextTail->pRankImageInstance && CPythonSystem::Instance().IsShowRank())
		{
			pTextTail->pRankImageInstance->Render();
		}
		if (pTextTail->pRankImageInstance2 && CPythonSystem::Instance().IsShowRank())
		{
			pTextTail->pRankImageInstance2->Render();
		}
#endif
#ifdef ENABLE_LANDRANK_SYSTEM
		if (pTextTail->pLandRankTextInstance && CPythonSystem::Instance().IsShowLandRank())
		{
			pTextTail->pLandRankTextInstance->Render();
		}
#endif
#ifdef ENABLE_REBORN_SYSTEM
		if (pTextTail->pRebornTextInstance && CPythonSystem::Instance().IsShowReborn())
		{
			pTextTail->pRebornTextInstance->Render();
		}
#endif
#ifdef ENABLE_TEAM_SYSTEM
		if (pTextTail->pTeamImageInstance && CPythonSystem::Instance().IsShowTeam())
		{
			pTextTail->pTeamImageInstance->Render();
		}
		if (pTextTail->pTeamTextInstance && CPythonSystem::Instance().IsShowTeam())
		{
			pTextTail->pTeamTextInstance->Render();
		}
#endif
#ifdef ENABLE_MONIKER_SYSTEM
		if (pTextTail->pMonikerTextInstance && CPythonSystem::Instance().IsShowMoniker())
		{
			pTextTail->pMonikerTextInstance->Render();
		}
#endif
#ifdef ENABLE_LOVE_SYSTEM
		if (pTextTail->pLoveImageInstance)
		{
			pTextTail->pLoveImageInstance->Render();
		}
		if (pTextTail->pLove1TextInstance)
		{
			pTextTail->pLove1TextInstance->Render();
		}
		if (pTextTail->pLove2TextInstance)
		{
			pTextTail->pLove2TextInstance->Render();
		}
#endif // ENABLE_LOVE_SYSTEM
#ifdef ENABLE_WORD_SYSTEM
		if (pTextTail->pWordTextInstance)
		{
			pTextTail->pWordTextInstance->Render();
		}
#endif // ENABLE_WORD_SYSTEM
#if defined(ENABLE_MOB_AGGR_LVL_INFO)
		if (pTextTail->pAIFlagTextInstance && CPythonSystem::Instance().IsShowMobAIFlag())
		{
			pTextTail->pAIFlagTextInstance->Render();
		}
#endif
	}

#ifdef ENABLE_OFFLINE_SHOP_CITIES
	for (auto itorMap = m_ShopTextTailMap.begin(); itorMap != m_ShopTextTailMap.end(); ++itorMap)
	{
		if (!itorMap->second->bRender)
			continue;

		TTextTail* pTextTail = itorMap->second;

		RenderTextTailBox(pTextTail);
		pTextTail->pTextInstance->Render();
		if (pTextTail->pOwnerTextInstance)
			pTextTail->pOwnerTextInstance->Render();
	}
#endif

	for (itor = m_ItemTextTailList.begin(); itor != m_ItemTextTailList.end(); ++itor)
	{
		TTextTail* pTextTail = *itor;

		RenderTextTailBox(pTextTail);
		pTextTail->pTextInstance->Render();
		if (pTextTail->pOwnerTextInstance)
			pTextTail->pOwnerTextInstance->Render();
	}

	for (TChatTailMap::iterator itorChat = m_ChatTailMap.begin(); itorChat != m_ChatTailMap.end(); ++itorChat)
	{
		TTextTail* pTextTail = itorChat->second;
		if (pTextTail->pOwner->isShow())
			RenderTextTailName(pTextTail);
	}
}

void CPythonTextTail::RenderTextTailBox(TTextTail* pTextTail)
{
#ifdef ENABLE_OFFLINE_SHOP
#ifdef ENABLE_OFFLINE_SHOP_CITIES
	if (pTextTail->bIsShop)
	{
		CPythonGraphic::Instance().SetDiffuseColor(0.0f, 0.0f, 0.0f, 1.0f);
		CPythonGraphic::Instance().RenderBox2d(pTextTail->x + pTextTail->xStart - 10.f,
			pTextTail->y + pTextTail->yStart - 10.f,
			pTextTail->x + pTextTail->xEnd + 10.f,
			pTextTail->y + pTextTail->yEnd + 10.f,
			pTextTail->z);

		CPythonGraphic::Instance().SetDiffuseColor(0.0f, 0.0f, 0.0f, 0.51f);
		CPythonGraphic::Instance().RenderBar2d(pTextTail->x + pTextTail->xStart - 10.f,
			pTextTail->y + pTextTail->yStart - 10.f,
			pTextTail->x + pTextTail->xEnd + 10.f,
			pTextTail->y + pTextTail->yEnd + 10.f,
			pTextTail->z);

		return;
	}
#endif
#endif
	CPythonGraphic::Instance().SetDiffuseColor(0.0f, 0.0f, 0.0f, 1.0f);
	CPythonGraphic::Instance().RenderBox2d(pTextTail->x + pTextTail->xStart,
		pTextTail->y + pTextTail->yStart,
		pTextTail->x + pTextTail->xEnd,
		pTextTail->y + pTextTail->yEnd,
		pTextTail->z);

	CPythonGraphic::Instance().SetDiffuseColor(0.0f, 0.0f, 0.0f, 0.3f);
	CPythonGraphic::Instance().RenderBar2d(pTextTail->x + pTextTail->xStart,
		pTextTail->y + pTextTail->yStart,
		pTextTail->x + pTextTail->xEnd,
		pTextTail->y + pTextTail->yEnd,
		pTextTail->z);
}

void CPythonTextTail::RenderTextTailName(TTextTail* pTextTail)
{
	pTextTail->pTextInstance->Render();
}

void CPythonTextTail::HideAllTextTail()
{
	m_CharacterTextTailList.clear();
	m_ItemTextTailList.clear();
#ifdef ENABLE_OFFLINE_SHOP_CITIES
	for (auto& iter : m_ShopTextTailMap)
		iter.second->bRender = false;
#endif
}

void CPythonTextTail::UpdateDistance(const TPixelPosition& c_rCenterPosition, TTextTail* pTextTail)
{
	const D3DXVECTOR3& c_rv3Position = pTextTail->pOwner->GetPosition();
	D3DXVECTOR2 v2Distance(c_rv3Position.x - c_rCenterPosition.x, -c_rv3Position.y - c_rCenterPosition.y);
	pTextTail->fDistanceFromPlayer = D3DXVec2Length(&v2Distance);
}

#ifdef ENABLE_GRAPHIC_OPTIMIZATION
constexpr float offlineShopDistance[5] = {25000.0f, 5000.0f, 3500.0f, 2500.0f, 1500.0f};

bool CPythonTextTail::CanShowCharacterTextTail(TTextTail * pTextTail)
{
	if (pTextTail->instanceType == CActorInstance::TYPE_NPC && CPythonSystem::Instance().IsNpcNameStatus() == false)
		return false;

	if (pTextTail->instanceType == CActorInstance::TYPE_SHOP)
	{
		int shopDistanceLevel = CPythonSystem::Instance().GetPrivateShopLevel();

		if (offlineShopDistance[shopDistanceLevel] < pTextTail->fDistanceFromPlayer)
			return false;

		if (CPythonSystem::Instance().IsPrivateShopStatus() == false)
			return false;

		return true;
	}

	return pTextTail->fDistanceFromPlayer < 3500.0f;
}

bool CPythonTextTail::CanShowItemTextTail(TTextTail* pTextTail, bool picked)
{
	if (CPythonSystem::Instance().GetDropItemLevel() >= 2)
		return false;

	return pTextTail->fDistanceFromPlayer < 3500.0f;
}
#endif

void CPythonTextTail::ShowAllTextTail()
{
	TTextTailMap::iterator itor;
	for (itor = m_CharacterTextTailMap.begin(); itor != m_CharacterTextTailMap.end(); ++itor)
	{
		TTextTail* pTextTail = itor->second;
#ifdef ENABLE_GRAPHIC_OPTIMIZATION
		if (CanShowCharacterTextTail(pTextTail))
#else
		if (pTextTail->fDistanceFromPlayer < 3500.0f)
#endif
			ShowCharacterTextTail(itor->first);
	}
	for (itor = m_ItemTextTailMap.begin(); itor != m_ItemTextTailMap.end(); ++itor)
	{
		TTextTail* pTextTail = itor->second;
#ifdef ENABLE_GRAPHIC_OPTIMIZATION
		if (CanShowItemTextTail(pTextTail, false))
#else
		if (pTextTail->fDistanceFromPlayer < 3500.0f)
#endif
			ShowItemTextTail(itor->first);
	}
#ifdef ENABLE_OFFLINE_SHOP_CITIES
	//OFFSHOP_DEBUG("ShopTextTailMap size %u ",m_ShopTextTailMap.size());
	for (itor = m_ShopTextTailMap.begin(); itor != m_ShopTextTailMap.end(); ++itor)
	{
		TTextTail* pTextTail = itor->second;
		if (CanShowCharacterTextTail(pTextTail))
			pTextTail->bRender = pTextTail->fDistanceFromPlayer < 3500.f;
	}
#endif
}

void CPythonTextTail::ShowCharacterTextTail(DWORD VirtualID)
{
	TTextTailMap::iterator itor = m_CharacterTextTailMap.find(VirtualID);

	if (m_CharacterTextTailMap.end() == itor)
		return;

	TTextTail* pTextTail = itor->second;

	if (m_CharacterTextTailList.end() != std::find(m_CharacterTextTailList.begin(), m_CharacterTextTailList.end(), pTextTail))
	{
		return;
	}

	if (!pTextTail->pOwner->isShow())
		return;

	CInstanceBase* pInstance = CPythonCharacterManager::Instance().GetInstancePtr(pTextTail->dwVirtualID);
	if (!pInstance)
		return;

	if (pInstance->IsGuildWall())
		return;

#ifdef ENABLE_EVENT_SYSTEM
	if (pInstance->GetRace() == 20143)
		return;
#endif

	if (pInstance->CanPickInstance())
		m_CharacterTextTailList.push_back(pTextTail);
}

void CPythonTextTail::ShowItemTextTail(DWORD VirtualID)
{
	TTextTailMap::iterator itor = m_ItemTextTailMap.find(VirtualID);

	if (m_ItemTextTailMap.end() == itor)
		return;

	TTextTail* pTextTail = itor->second;

	if (m_ItemTextTailList.end() != std::find(m_ItemTextTailList.begin(), m_ItemTextTailList.end(), pTextTail))
	{
		return;
	}

#ifdef ENABLE_GRAPHIC_OPTIMIZATION
	if (CPythonSystem::Instance().GetDropItemLevel() == 4)
		return;
#endif

	m_ItemTextTailList.push_back(pTextTail);
}

bool CPythonTextTail::isIn(CPythonTextTail::TTextTail* pSource, CPythonTextTail::TTextTail* pTarget)
{
	float x1Source = pSource->x + pSource->xStart;
	float y1Source = pSource->y + pSource->yStart;
	float x2Source = pSource->x + pSource->xEnd;
	float y2Source = pSource->y + pSource->yEnd;
	float x1Target = pTarget->x + pTarget->xStart;
	float y1Target = pTarget->y + pTarget->yStart;
	float x2Target = pTarget->x + pTarget->xEnd;
	float y2Target = pTarget->y + pTarget->yEnd;

	if (x1Source <= x2Target && x2Source >= x1Target &&
		y1Source <= y2Target && y2Source >= y1Target)
	{
		return true;
	}

	return false;
}

void CPythonTextTail::RegisterCharacterTextTail(DWORD dwGuildID, DWORD dwVirtualID, const D3DXCOLOR& c_rColor, float fAddHeight)
{
	CInstanceBase* pCharacterInstance = CPythonCharacterManager::Instance().GetInstancePtr(dwVirtualID);

	if (!pCharacterInstance)
		return;

	TTextTail* pTextTail = RegisterTextTail(dwVirtualID,
		pCharacterInstance->GetNameString(),
		pCharacterInstance->GetGraphicThingInstancePtr(),
		pCharacterInstance->GetGraphicThingInstanceRef().GetHeight() + fAddHeight,
		c_rColor);

	CGraphicTextInstance* pTextInstance = pTextTail->pTextInstance;
	pTextInstance->SetOutline(true);
	pTextInstance->SetVerticalAlign(CGraphicTextInstance::VERTICAL_ALIGN_BOTTOM);

	pTextTail->pMarkInstance = NULL;
	pTextTail->pGuildNameTextInstance = NULL;
	pTextTail->pTitleTextInstance = NULL;
	pTextTail->pLevelTextInstance = NULL;
#ifdef ENABLE_RANK_SYSTEM
	pTextTail->pRankImageInstance = NULL;
	pTextTail->pRankImageInstance2 = NULL;
#endif
#ifdef ENABLE_LANDRANK_SYSTEM
	pTextTail->pLandRankTextInstance = NULL;
#endif
#ifdef ENABLE_REBORN_SYSTEM
	pTextTail->pRebornTextInstance = NULL;
#endif
#ifdef ENABLE_TEAM_SYSTEM
	pTextTail->pTeamImageInstance = NULL;
	pTextTail->pTeamTextInstance = NULL;
#endif
#ifdef ENABLE_MONIKER_SYSTEM
	pTextTail->pMonikerTextInstance = NULL;
#endif
#ifdef ENABLE_LOVE_SYSTEM
	pTextTail->pLoveImageInstance = NULL;
	pTextTail->pLove1TextInstance = NULL;
	pTextTail->pLove2TextInstance = NULL;
#endif // ENABLE_LOVE_SYSTEM
#ifdef ENABLE_WORD_SYSTEM
	pTextTail->pWordTextInstance = NULL;
#endif // ENABLE_WORD_SYSTEM
#if defined(ENABLE_MOB_AGGR_LVL_INFO)
	pTextTail->pAIFlagTextInstance = NULL;
#endif

	if (0 != dwGuildID)
	{
		pTextTail->pMarkInstance = CGraphicMarkInstance::New();

		DWORD dwMarkID = CGuildMarkManager::Instance().GetMarkID(dwGuildID);

		if (dwMarkID != CGuildMarkManager::INVALID_MARK_ID)
		{
			std::string markImagePath;

			if (CGuildMarkManager::Instance().GetMarkImageFilename(dwMarkID / CGuildMarkImage::MARK_TOTAL_COUNT, markImagePath))
			{
				pTextTail->pMarkInstance->SetImageFileName(markImagePath.c_str());
				pTextTail->pMarkInstance->Load();
				pTextTail->pMarkInstance->SetIndex(dwMarkID % CGuildMarkImage::MARK_TOTAL_COUNT);
			}
		}

		std::string strGuildName;
		if (!CPythonGuild::Instance().GetGuildName(dwGuildID, &strGuildName))
			strGuildName = "Noname";

		CGraphicTextInstance*& prGuildNameInstance = pTextTail->pGuildNameTextInstance;
		prGuildNameInstance = CGraphicTextInstance::New();
		prGuildNameInstance->SetTextPointer(ms_pFont);
		prGuildNameInstance->SetOutline(true);
		prGuildNameInstance->SetHorizonalAlign(CGraphicTextInstance::HORIZONTAL_ALIGN_CENTER);
		prGuildNameInstance->SetVerticalAlign(CGraphicTextInstance::VERTICAL_ALIGN_BOTTOM);
		prGuildNameInstance->SetValue(strGuildName.c_str());
#ifdef ENABLE_GUILD_LEADER_SYSTEM
		if (pCharacterInstance->GetGuildMemberType() == GUILD_MEMBER_LEADER)
			prGuildNameInstance->SetColor(c_TextTail_Guild_Name_Color_Leader.r, c_TextTail_Guild_Name_Color_Leader.g, c_TextTail_Guild_Name_Color_Leader.b);
		else if (pCharacterInstance->GetGuildMemberType() == GUILD_MEMBER_GENERAL)
			prGuildNameInstance->SetColor(c_TextTail_Guild_Name_Color_General.r, c_TextTail_Guild_Name_Color_General.g, c_TextTail_Guild_Name_Color_General.b);
		else
			prGuildNameInstance->SetColor(c_TextTail_Guild_Name_Color_Member.r, c_TextTail_Guild_Name_Color_Member.g, c_TextTail_Guild_Name_Color_Member.b);
#else
		prGuildNameInstance->SetColor(c_TextTail_Guild_Name_Color.r, c_TextTail_Guild_Name_Color.g, c_TextTail_Guild_Name_Color.b);
#endif
		prGuildNameInstance->Update();
	}

#if defined(ENABLE_MOB_AGGR_LVL_INFO)
	if (IS_SET(pCharacterInstance->GetAIFlag(), CInstanceBase::AIFLAG_AGGRESSIVE))
	{
		CGraphicTextInstance*& prAIFlagInstance = pTextTail->pAIFlagTextInstance;
		prAIFlagInstance = CGraphicTextInstance::New();
		prAIFlagInstance->SetTextPointer(ms_pFont);
		prAIFlagInstance->SetHorizonalAlign(CGraphicTextInstance::HORIZONTAL_ALIGN_LEFT);
		prAIFlagInstance->SetVerticalAlign(CGraphicTextInstance::VERTICAL_ALIGN_BOTTOM);
		prAIFlagInstance->SetValue("*");
		prAIFlagInstance->SetOutline(true);
		prAIFlagInstance->SetColor(c_rColor.r, c_rColor.g, c_rColor.b);
		prAIFlagInstance->Update();
	}
	pTextTail->instanceType = pCharacterInstance->GetInstanceType();
#endif

#ifdef ENABLE_TEAM_SYSTEM
	if (pCharacterInstance->IsPC() && pCharacterInstance->GetTeam())
	{
		pTextTail->pTeamImageInstance = CGraphicImageInstance::New();
		pTextTail->pTeamTextInstance = CGraphicTextInstance::New();
		pTextTail->pTeamTextInstance->SetTextPointer(ms_pFont);
		pTextTail->pTeamTextInstance->SetOutline(true);

		char szPath[256];
		sprintf(szPath, "d:/ymir work/team/%d.tga", pCharacterInstance->GetTeam());
		pTextTail->pTeamImageInstance->SetImagePointer((CGraphicImage*)CResourceManager::Instance().GetResourcePointer(szPath));

		pTextTail->pTeamTextInstance->SetValueString(teamNames[pCharacterInstance->GetTeam()-1].first);
		pTextTail->pTeamTextInstance->SetColor(teamNames[pCharacterInstance->GetTeam()-1].second.first);
		pTextTail->pTeamTextInstance->SetOutLineColor(teamNames[pCharacterInstance->GetTeam()-1].second.second);
	}
#endif

	m_CharacterTextTailMap.insert(TTextTailMap::value_type(dwVirtualID, pTextTail));
}

void CPythonTextTail::RegisterItemTextTail(DWORD VirtualID, const char* c_szText, CGraphicObjectInstance* pOwner)
{
#ifdef __DEBUG
	char szName[256];
	spritnf(szName, "%s[%d]", c_szText, VirtualID);
	D3DXCOLOR c_d3dColor = c_TextTail_Item_Color;
	TTextTail* pTextTail = RegisterTextTail(VirtualID, c_szText, pOwner, c_TextTail_Name_Position, c_d3dColor);
	m_ItemTextTailMap.insert(TTextTailMap::value_type(VirtualID, pTextTail));
#else
	D3DXCOLOR c_d3dColor = c_TextTail_Item_Color;
	TTextTail* pTextTail = RegisterTextTail(VirtualID, c_szText, pOwner, c_TextTail_Name_Position, c_d3dColor);
	m_ItemTextTailMap.insert(TTextTailMap::value_type(VirtualID, pTextTail));
#endif
}

#ifdef ENABLE_OFFLINE_SHOP_CITIES
void CPythonTextTail::RegisterShopInstanceTextTail(DWORD dwVirtualID, const char* c_szName, CGraphicObjectInstance* pOwner)
{
	TTextTail* pTextTail = RegisterShopTextTail(dwVirtualID, c_szName, pOwner);
	m_ShopTextTailMap.insert(TTextTailMap::value_type(dwVirtualID, pTextTail));
}
#endif
void CPythonTextTail::RegisterChatTail(DWORD VirtualID, const char* c_szChat)
{
	CInstanceBase* pCharacterInstance = CPythonCharacterManager::Instance().GetInstancePtr(VirtualID);

	if (!pCharacterInstance)
		return;

	TChatTailMap::iterator itor = m_ChatTailMap.find(VirtualID);

#ifdef ENABLE_PB2_PREMIUM_SYSTEM
	std::string strContent = c_szChat;

	char text[69][69] = {":**", "O:)", "~X(", "=D>", ">3", "X(", ";;)",
		";))", "\:D/", ":^>", ">:/", "=((", ":-c", ":-@",
		":O)", ":-/", ":((", ":D", "8->", ">:)", ":o3",
		":-<", ":-$", "=P~", "(pow)", ":-L", ">:P", ";)",
		"<3", ":!!", "@-)", "^#(^", ":*", ":))", ":^o",
		":X", ":-SS", ":-B", "%-(", "[-(", "^:)^", ":-j",
		":)]", "<:-P", ":P", ":ar!", "[-o<", "\m/", "8-/",
		"=))", "@};-", ":(", "[-x", ":-&", ":-^", "8-}",
		"/-)", ":)", ":>", ":/", "B-)", ":O", "=;",
		":-?", ":-t", ":-w", ":-h"};
	char emoji[69][69] = {"|Eemoji/yahoo/2love|e", "|Eemoji/yahoo/angel|e", "|Eemoji/yahoo/at-wits-end|e", "|Eemoji/yahoo/applause|e", "|Eemoji/yahoo/arrow|e", "|Eemoji/yahoo/angry-or-grumpy|e", "|Eemoji/yahoo/batting-eyelashes|e",
		"|Eemoji/yahoo/giggle-or-hee-hee|e", "|Eemoji/yahoo/dancing|e", "|Eemoji/yahoo/blushing|e", "|Eemoji/yahoo/bring-it-on|e", "|Eemoji/yahoo/broken-heart|e", "|Eemoji/yahoo/call-me|e", "|Eemoji/yahoo/chatterbox|e",
		"|Eemoji/yahoo/clown|e", "|Eemoji/yahoo/confused|e", "|Eemoji/yahoo/crying|e", "|Eemoji/yahoo/big-grin|e", "|Eemoji/yahoo/daydreaming|e", "|Eemoji/yahoo/devil|e", "|Eemoji/yahoo/dog|e",
		"|Eemoji/yahoo/doh!|e", "|Eemoji/yahoo/do-not-tell-anyone|e", "|Eemoji/yahoo/drooling|e", "|Eemoji/yahoo/fortziki|e", "|Eemoji/yahoo/frustrated|e", "|Eemoji/yahoo/phbbbbt-or-upset|e", "|Eemoji/yahoo/beat-up|e",
		"|Eemoji/yahoo/heart|e", "|Eemoji/yahoo/hurry-up|e", "|Eemoji/yahoo/hypnotized|e", "|Eemoji/yahoo/it-was-not-me|e", "|Eemoji/yahoo/kiss|e", "|Eemoji/yahoo/laughing|e", "|Eemoji/yahoo/liar-liar|e",
		"|Eemoji/yahoo/love-struck|e", "|Eemoji/yahoo/nail-biting|e", "|Eemoji/yahoo/nerd|e", "|Eemoji/yahoo/not-listening|e", "|Eemoji/yahoo/not-talking|e", "|Eemoji/yahoo/not-worthy|e", "|Eemoji/yahoo/oh-go-on|e",
		"|Eemoji/yahoo/on-the-phone|e", "|Eemoji/yahoo/party|e", "|Eemoji/yahoo/frustrated-or-sticking-tongue-out|e", "|Eemoji/yahoo/pirate|e", "|Eemoji/yahoo/praying|e", "|Eemoji/yahoo/rocking|e", "|Eemoji/yahoo/rolling-eyes|e",
		"|Eemoji/yahoo/rolling-on-the-floor-laughing|e", "|Eemoji/yahoo/rose|e", "|Eemoji/yahoo/sad-or-frown-face|e", "|Eemoji/yahoo/shame-on-you|e", "|Eemoji/yahoo/sick|e", "|Eemoji/yahoo/whistling|e", "|Eemoji/yahoo/silly|e",
		"|Eemoji/yahoo/sleepy|e", "|Eemoji/yahoo/smile-or-happy-face|e", "|Eemoji/yahoo/smug|e", "|Eemoji/yahoo/straight-face|e", "|Eemoji/yahoo/sunglasses-or-cool|e", "|Eemoji/yahoo/surprised|e", "|Eemoji/yahoo/talk-to-the-hand|e",
		"|Eemoji/yahoo/thinking|e", "|Eemoji/yahoo/time-out|e", "|Eemoji/yahoo/waiting|e", "|Eemoji/yahoo/wave|e"};
	   
	if (strContent.find("http") == std::string::npos && CPythonPlayer::Instance().IsPB2GlobalChat() == true)
		for (int i = 0; i < 69; i++)
			boost::algorithm::replace_all(strContent, text[i], emoji[i]);
#endif // ENABLE_PB2_PREMIUM_SYSTEM

	if (m_ChatTailMap.end() != itor)
	{
		TTextTail* pTextTail = itor->second;

#ifdef ENABLE_PB2_PREMIUM_SYSTEM
		pTextTail->pTextInstance->SetValue(strContent.c_str());
#else
		pTextTail->pTextInstance->SetValue(c_szChat);
#endif // ENABLE_PB2_PREMIUM_SYSTEM
		pTextTail->pTextInstance->Update();
		pTextTail->Color = c_TextTail_Chat_Color;
		pTextTail->pTextInstance->SetColor(c_TextTail_Chat_Color);

		// TEXTTAIL_LIVINGTIME_CONTROL
		pTextTail->LivingTime = CTimer::Instance().GetCurrentMillisecond() + TextTail_GetLivingTime();
		// END_OF_TEXTTAIL_LIVINGTIME_CONTROL

		pTextTail->bNameFlag = TRUE;

		return;
	}

	TTextTail* pTextTail = RegisterTextTail(VirtualID, strContent.c_str(), pCharacterInstance->GetGraphicThingInstancePtr(), pCharacterInstance->GetGraphicThingInstanceRef().GetHeight() + 10.0f, c_TextTail_Chat_Color);

	// TEXTTAIL_LIVINGTIME_CONTROL
	pTextTail->LivingTime = CTimer::Instance().GetCurrentMillisecond() + TextTail_GetLivingTime();
	// END_OF_TEXTTAIL_LIVINGTIME_CONTROL

	pTextTail->bNameFlag = TRUE;
	pTextTail->pTextInstance->SetOutline(true);
	pTextTail->pTextInstance->SetVerticalAlign(CGraphicTextInstance::VERTICAL_ALIGN_BOTTOM);
	m_ChatTailMap.insert(TTextTailMap::value_type(VirtualID, pTextTail));
}

void CPythonTextTail::RegisterInfoTail(DWORD VirtualID, const char* c_szChat)
{
	CInstanceBase* pCharacterInstance = CPythonCharacterManager::Instance().GetInstancePtr(VirtualID);

	if (!pCharacterInstance)
		return;

	TChatTailMap::iterator itor = m_ChatTailMap.find(VirtualID);

	if (m_ChatTailMap.end() != itor)
	{
		TTextTail* pTextTail = itor->second;

		pTextTail->pTextInstance->SetValue(c_szChat);
		pTextTail->pTextInstance->Update();
		pTextTail->Color = c_TextTail_Info_Color;
		pTextTail->pTextInstance->SetColor(c_TextTail_Info_Color);

		// TEXTTAIL_LIVINGTIME_CONTROL
		pTextTail->LivingTime = CTimer::Instance().GetCurrentMillisecond() + TextTail_GetLivingTime();
		// END_OF_TEXTTAIL_LIVINGTIME_CONTROL

		pTextTail->bNameFlag = FALSE;

		return;
	}

	TTextTail* pTextTail = RegisterTextTail(VirtualID,
		c_szChat,
		pCharacterInstance->GetGraphicThingInstancePtr(),
		pCharacterInstance->GetGraphicThingInstanceRef().GetHeight() + 10.0f,
		c_TextTail_Info_Color);

	// TEXTTAIL_LIVINGTIME_CONTROL
	pTextTail->LivingTime = CTimer::Instance().GetCurrentMillisecond() + TextTail_GetLivingTime();
	// END_OF_TEXTTAIL_LIVINGTIME_CONTROL

	pTextTail->bNameFlag = FALSE;
	pTextTail->pTextInstance->SetOutline(true);
	pTextTail->pTextInstance->SetVerticalAlign(CGraphicTextInstance::VERTICAL_ALIGN_BOTTOM);
	m_ChatTailMap.insert(TTextTailMap::value_type(VirtualID, pTextTail));
}

bool CPythonTextTail::GetTextTailPosition(DWORD dwVID, float* px, float* py, float* pz)
{
	TTextTailMap::iterator itorCharacter = m_CharacterTextTailMap.find(dwVID);

	if (m_CharacterTextTailMap.end() == itorCharacter)
	{
		return false;
	}

	TTextTail* pTextTail = itorCharacter->second;
	*px = pTextTail->x;
	*py = pTextTail->y;
	*pz = pTextTail->z;

	return true;
}

bool CPythonTextTail::IsChatTextTail(DWORD dwVID)
{
	TChatTailMap::iterator itorChat = m_ChatTailMap.find(dwVID);

	if (m_ChatTailMap.end() == itorChat)
		return false;

	return true;
}

#ifdef ENABLE_GRAPHIC_OPTIMIZATION
bool CPythonTextTail::IsCharacterTextTail(DWORD dwVID)
{
	TTextTailMap::iterator itor = m_CharacterTextTailMap.find(dwVID);

	if (m_CharacterTextTailMap.end() == itor)
		return false;

	return true;
}

bool CPythonTextTail::IsItemTextTail(DWORD dwVID)
{
	TTextTailMap::iterator itor = m_ItemTextTailMap.find(dwVID);

	if (m_ItemTextTailMap.end() == itor)
		return false;

	return true;
}

int CPythonTextTail::GetInstanceType(DWORD dwVID)
{
	// eger karakter degil ise -1
	if (!IsCharacterTextTail(dwVID))
		return -1;

	TTextTailMap::iterator itor = m_CharacterTextTailMap.find(dwVID);

	if (m_CharacterTextTailMap.end() == itor)
		return -1;

	// texttail bulunamazsa -1
	return itor->second->GetInstanceType();
}
#endif

void CPythonTextTail::SetCharacterTextTailColor(DWORD VirtualID, const D3DXCOLOR& c_rColor)
{
	TTextTailMap::iterator itorCharacter = m_CharacterTextTailMap.find(VirtualID);

	if (m_CharacterTextTailMap.end() == itorCharacter)
		return;

	TTextTail* pTextTail = itorCharacter->second;
	pTextTail->pTextInstance->SetColor(c_rColor);
	pTextTail->Color = c_rColor;
}

void CPythonTextTail::SetItemTextTailOwner(DWORD dwVID, const char* c_szName)
{
	TTextTailMap::iterator itor = m_ItemTextTailMap.find(dwVID);
	if (m_ItemTextTailMap.end() == itor)
		return;

	TTextTail* pTextTail = itor->second;

	if (strlen(c_szName) > 0)
	{
		if (!pTextTail->pOwnerTextInstance)
		{
			pTextTail->pOwnerTextInstance = CGraphicTextInstance::New();
		}

		std::string strName = c_szName;
		static const string& strOwnership = ApplicationStringTable_GetString(IDS_POSSESSIVE_MORPHENE) == "" ? "'s" : ApplicationStringTable_GetString(IDS_POSSESSIVE_MORPHENE);
		strName += strOwnership;

		pTextTail->pOwnerTextInstance->SetTextPointer(ms_pFont);
		pTextTail->pOwnerTextInstance->SetHorizonalAlign(CGraphicTextInstance::HORIZONTAL_ALIGN_CENTER);
		pTextTail->pOwnerTextInstance->SetValue(strName.c_str());
#ifdef ENABLE_ITEM_ENTITY_UTILITY
		CInstanceBase* pInstance = CPythonCharacterManager::Instance().GetMainInstancePtr();
		if (pInstance)
		{
			if (!strcmp(pInstance->GetNameString(), c_szName))
				pTextTail->pOwnerTextInstance->SetColor(1.0f, 1.0f, 0.0f);
			else
				pTextTail->pOwnerTextInstance->SetColor(c_TextTail_Item_Red_Color.r, c_TextTail_Item_Red_Color.g, c_TextTail_Item_Red_Color.b);
		}
#else
		pTextTail->pOwnerTextInstance->SetColor(1.0f, 1.0f, 0.0f);
#endif
		pTextTail->pOwnerTextInstance->Update();

		int xOwnerSize, yOwnerSize;
		pTextTail->pOwnerTextInstance->GetTextSize(&xOwnerSize, &yOwnerSize);
		pTextTail->yStart = -2.0f;
		pTextTail->yEnd += float(yOwnerSize + 4);
		pTextTail->xStart = fMIN(pTextTail->xStart, float(-xOwnerSize / 2 - 1));
		pTextTail->xEnd = fMAX(pTextTail->xEnd, float(xOwnerSize / 2 + 1));
	}
	else
	{
		if (pTextTail->pOwnerTextInstance)
		{
			CGraphicTextInstance::Delete(pTextTail->pOwnerTextInstance);
			pTextTail->pOwnerTextInstance = NULL;
		}

		int xSize, ySize;
		pTextTail->pTextInstance->GetTextSize(&xSize, &ySize);
		pTextTail->xStart = (float)(-xSize / 2 - 2);
		pTextTail->yStart = -2.0f;
		pTextTail->xEnd = (float)(xSize / 2 + 2);
		pTextTail->yEnd = (float)ySize;
	}
}

void CPythonTextTail::DeleteCharacterTextTail(DWORD VirtualID)
{
	TTextTailMap::iterator itorCharacter = m_CharacterTextTailMap.find(VirtualID);
	TTextTailMap::iterator itorChat = m_ChatTailMap.find(VirtualID);

	if (m_CharacterTextTailMap.end() != itorCharacter)
	{
		DeleteTextTail(itorCharacter->second);
		m_CharacterTextTailMap.erase(itorCharacter);
	}
	else
	{
		Tracenf("CPythonTextTail::DeleteCharacterTextTail - Find VID[%d] Error", VirtualID);
	}

	if (m_ChatTailMap.end() != itorChat)
	{
		DeleteTextTail(itorChat->second);
		m_ChatTailMap.erase(itorChat);
	}
}
#ifdef ENABLE_OFFLINE_SHOP_CITIES
void CPythonTextTail::DeleteShopTextTail(DWORD VirtualID)
{
	TTextTailMap::iterator itor = m_ShopTextTailMap.find(VirtualID);

	if (m_ShopTextTailMap.end() == itor)
	{
		Tracef(" CPythonTextTail::DeleteShopTextTail - None Item Text Tail\n");
		return;
	}

	DeleteTextTail(itor->second);
	m_ShopTextTailMap.erase(itor);
}
#endif

void CPythonTextTail::DeleteItemTextTail(DWORD VirtualID)
{
	TTextTailMap::iterator itor = m_ItemTextTailMap.find(VirtualID);

	if (m_ItemTextTailMap.end() == itor)
	{
		Tracef(" CPythonTextTail::DeleteItemTextTail - None Item Text Tail\n");
		return;
	}

	DeleteTextTail(itor->second);
	m_ItemTextTailMap.erase(itor);
}

CPythonTextTail::TTextTail* CPythonTextTail::RegisterTextTail(DWORD dwVirtualID, const char* c_szText, CGraphicObjectInstance* pOwner, float fHeight, const D3DXCOLOR& c_rColor)
{
	TTextTail* pTextTail = m_TextTailPool.Alloc();

#ifdef ENABLE_OFFLINE_SHOP
#ifdef ENABLE_OFFLINE_SHOP_CITIES
	pTextTail->bIsShop = false;
	pTextTail->bRender = false;
#endif
#endif
	pTextTail->dwVirtualID = dwVirtualID;
	pTextTail->pOwner = pOwner;
	pTextTail->pTextInstance = CGraphicTextInstance::New();
	pTextTail->pOwnerTextInstance = NULL;
	pTextTail->fHeight = fHeight;

	pTextTail->pTextInstance->SetTextPointer(ms_pFont);
	pTextTail->pTextInstance->SetHorizonalAlign(CGraphicTextInstance::HORIZONTAL_ALIGN_CENTER);
	pTextTail->pTextInstance->SetValue(c_szText);
	pTextTail->pTextInstance->SetColor(c_rColor.r, c_rColor.g, c_rColor.b);
	pTextTail->pTextInstance->Update();

	int xSize, ySize;
	pTextTail->pTextInstance->GetTextSize(&xSize, &ySize);
	pTextTail->xStart = (float)(-xSize / 2 - 2);
	pTextTail->yStart = -2.0f;
	pTextTail->xEnd = (float)(xSize / 2 + 2);
	pTextTail->yEnd = (float)ySize;
	pTextTail->Color = c_rColor;
	pTextTail->fDistanceFromPlayer = 0.0f;
	pTextTail->x = -100.0f;
	pTextTail->y = -100.0f;
	pTextTail->z = 0.0f;
	pTextTail->pMarkInstance = NULL;
	pTextTail->pGuildNameTextInstance = NULL;
	pTextTail->pTitleTextInstance = NULL;
	pTextTail->pLevelTextInstance = NULL;
#ifdef ENABLE_RANK_SYSTEM
	pTextTail->pRankImageInstance = NULL;
	pTextTail->pRankImageInstance2 = NULL;
#endif
#ifdef ENABLE_LANDRANK_SYSTEM
	pTextTail->pLandRankTextInstance = NULL;
#endif
#ifdef ENABLE_REBORN_SYSTEM
	pTextTail->pRebornTextInstance = NULL;
#endif
#ifdef ENABLE_TEAM_SYSTEM
	pTextTail->pTeamImageInstance = NULL;
	pTextTail->pTeamTextInstance = NULL;
#endif
#ifdef ENABLE_MONIKER_SYSTEM
	pTextTail->pMonikerTextInstance = NULL;
#endif
#ifdef ENABLE_LOVE_SYSTEM
	pTextTail->pLoveImageInstance = NULL;
	pTextTail->pLove1TextInstance = NULL;
	pTextTail->pLove2TextInstance = NULL;
#endif // ENABLE_LOVE_SYSTEM
#ifdef ENABLE_WORD_SYSTEM
	pTextTail->pWordTextInstance = NULL;
#endif // ENABLE_WORD_SYSTEM
#if defined(ENABLE_MOB_AGGR_LVL_INFO)
	pTextTail->pAIFlagTextInstance = NULL;
#endif
#ifdef ENABLE_GRAPHIC_OPTIMIZATION
	CInstanceBase * ownerInstance = CPythonCharacterManager::Instance().GetInstancePtr(dwVirtualID);
	if (ownerInstance != nullptr)
		pTextTail->instanceType = ownerInstance->GetInstanceType();
#endif
	return pTextTail;
}

#ifdef ENABLE_OFFLINE_SHOP_CITIES
CPythonTextTail::TTextTail* CPythonTextTail::RegisterShopTextTail(DWORD dwVirtualID, const char* c_szText, CGraphicObjectInstance* pOwner)
{
	const D3DXCOLOR& c_rColor = D3DXCOLOR(1.0, 1.0, 0.5, 1.0);

	TTextTail* pTextTail = m_TextTailPool.Alloc();

	pTextTail->bIsShop = true;

	pTextTail->dwVirtualID = dwVirtualID;
	pTextTail->pOwner = pOwner;
	pTextTail->pTextInstance = CGraphicTextInstance::New();
	pTextTail->pOwnerTextInstance = NULL;
	pTextTail->fHeight = 180.f;

	pTextTail->pTextInstance->SetTextPointer(ms_pFont);
	pTextTail->pTextInstance->SetHorizonalAlign(CGraphicTextInstance::HORIZONTAL_ALIGN_CENTER);
	pTextTail->pTextInstance->SetValue(c_szText);
	pTextTail->pTextInstance->SetColor(c_rColor.r, c_rColor.g, c_rColor.b);
	pTextTail->pTextInstance->Update();

	int xSize, ySize;
	pTextTail->pTextInstance->GetTextSize(&xSize, &ySize);
	pTextTail->xStart = (float)(-xSize / 2 - 2);
	pTextTail->yStart = -2.0f;
	pTextTail->xEnd = (float)(xSize / 2 + 2);
	pTextTail->yEnd = (float)ySize;
	pTextTail->Color = c_rColor;
	pTextTail->fDistanceFromPlayer = 0.0f;
	pTextTail->x = -100.0f;
	pTextTail->y = -100.0f;
	pTextTail->z = 0.0f;
	pTextTail->pMarkInstance = NULL;
	pTextTail->pGuildNameTextInstance = NULL;
	pTextTail->pTitleTextInstance = NULL;
	pTextTail->pLevelTextInstance = NULL;
#ifdef ENABLE_RANK_SYSTEM
	pTextTail->pRankImageInstance = NULL;
	pTextTail->pRankImageInstance2 = NULL;
#endif
#ifdef ENABLE_LANDRANK_SYSTEM
	pTextTail->pLandRankTextInstance = NULL;
#endif
#ifdef ENABLE_REBORN_SYSTEM
	pTextTail->pRebornTextInstance = NULL;
#endif
#ifdef ENABLE_TEAM_SYSTEM
	pTextTail->pTeamImageInstance = NULL;
	pTextTail->pTeamTextInstance = NULL;
#endif
#ifdef ENABLE_MONIKER_SYSTEM
	pTextTail->pMonikerTextInstance = NULL;
#endif
#ifdef ENABLE_LOVE_SYSTEM
	pTextTail->pLoveImageInstance = NULL;
	pTextTail->pLove1TextInstance = NULL;
	pTextTail->pLove2TextInstance = NULL;
#endif // ENABLE_LOVE_SYSTEM
#ifdef ENABLE_WORD_SYSTEM
	pTextTail->pWordTextInstance = NULL;
#endif
#if defined(ENABLE_MOB_AGGR_LVL_INFO)
	pTextTail->pAIFlagTextInstance = NULL;
#endif
	pTextTail->instanceType = CActorInstance::TYPE_SHOP;
	return pTextTail;
}

bool CPythonTextTail::GetPickedNewShop(DWORD* pdwVID)
{
	*pdwVID = 0;

	long ixMouse = 0, iyMouse = 0;

	POINT p;
	CPythonApplication::Instance().GetMousePosition(&p);

	ixMouse = p.x;
	iyMouse = p.y;

	for (auto itor = m_ShopTextTailMap.begin(); itor != m_ShopTextTailMap.end(); ++itor)
	{
		TTextTail* pTextTail = itor->second;

		if (ixMouse >= pTextTail->x + (pTextTail->xStart - 10) && ixMouse <= pTextTail->x + (pTextTail->xEnd + 10) &&
			iyMouse >= pTextTail->y + (pTextTail->yStart - 10) && iyMouse <= pTextTail->y + (pTextTail->yEnd + 10))
		{
			*pdwVID = itor->first;
			return true;
		}
	}

	return false;
}

#endif
void CPythonTextTail::DeleteTextTail(TTextTail* pTextTail)
{
	if (pTextTail->pTextInstance)
	{
		CGraphicTextInstance::Delete(pTextTail->pTextInstance);
		pTextTail->pTextInstance = NULL;
	}
	if (pTextTail->pOwnerTextInstance)
	{
		CGraphicTextInstance::Delete(pTextTail->pOwnerTextInstance);
		pTextTail->pOwnerTextInstance = NULL;
	}
	if (pTextTail->pMarkInstance)
	{
		CGraphicMarkInstance::Delete(pTextTail->pMarkInstance);
		pTextTail->pMarkInstance = NULL;
	}
	if (pTextTail->pGuildNameTextInstance)
	{
		CGraphicTextInstance::Delete(pTextTail->pGuildNameTextInstance);
		pTextTail->pGuildNameTextInstance = NULL;
	}
	if (pTextTail->pTitleTextInstance)
	{
		CGraphicTextInstance::Delete(pTextTail->pTitleTextInstance);
		pTextTail->pTitleTextInstance = NULL;
	}
	if (pTextTail->pLevelTextInstance)
	{
		CGraphicTextInstance::Delete(pTextTail->pLevelTextInstance);
		pTextTail->pLevelTextInstance = NULL;
	}
#ifdef ENABLE_RANK_SYSTEM
	if (pTextTail->pRankImageInstance)
	{
		CGraphicImageInstance::Delete(pTextTail->pRankImageInstance);
		pTextTail->pRankImageInstance = NULL;
	}
	if (pTextTail->pRankImageInstance2)
	{
		CGraphicImageInstance::Delete(pTextTail->pRankImageInstance2);
		pTextTail->pRankImageInstance2 = NULL;
	}
#endif
#ifdef ENABLE_LANDRANK_SYSTEM
	if (pTextTail->pLandRankTextInstance)
	{
		CGraphicTextInstance::Delete(pTextTail->pLandRankTextInstance);
		pTextTail->pLandRankTextInstance = NULL;
	}
#endif
#ifdef ENABLE_REBORN_SYSTEM
	if (pTextTail->pRebornTextInstance)
	{
		CGraphicTextInstance::Delete(pTextTail->pRebornTextInstance);
		pTextTail->pRebornTextInstance = NULL;
	}
#endif
#ifdef ENABLE_TEAM_SYSTEM
	if (pTextTail->pTeamImageInstance)
	{
		CGraphicImageInstance::Delete(pTextTail->pTeamImageInstance);
		pTextTail->pTeamImageInstance = NULL;
	}
	if (pTextTail->pTeamTextInstance)
	{
		CGraphicTextInstance::Delete(pTextTail->pTeamTextInstance);
		pTextTail->pTeamTextInstance = NULL;
	}
#endif
#ifdef ENABLE_MONIKER_SYSTEM
	if (pTextTail->pMonikerTextInstance)
	{
		CGraphicTextInstance::Delete(pTextTail->pMonikerTextInstance);
		pTextTail->pMonikerTextInstance = NULL;
	}
#endif
#ifdef ENABLE_LOVE_SYSTEM
	if (pTextTail->pLoveImageInstance)
	{
		CGraphicImageInstance::Delete(pTextTail->pLoveImageInstance);
		pTextTail->pLoveImageInstance = NULL;
	}

	if (pTextTail->pLove1TextInstance)
	{
		CGraphicTextInstance::Delete(pTextTail->pLove1TextInstance);
		pTextTail->pLove1TextInstance = NULL;
	}

	if (pTextTail->pLove2TextInstance)
	{
		CGraphicTextInstance::Delete(pTextTail->pLove2TextInstance);
		pTextTail->pLove2TextInstance = NULL;
	}
#endif // ENABLE_LOVE_SYSTEM
#ifdef ENABLE_WORD_SYSTEM
	if (pTextTail->pWordTextInstance)
	{
		CGraphicTextInstance::Delete(pTextTail->pWordTextInstance);
		pTextTail->pWordTextInstance = NULL;
	}
#endif
#if defined(ENABLE_MOB_AGGR_LVL_INFO)
	if (pTextTail->pAIFlagTextInstance)
	{
		CGraphicTextInstance::Delete(pTextTail->pAIFlagTextInstance);
		pTextTail->pAIFlagTextInstance = NULL;
	}
#endif
	m_TextTailPool.Free(pTextTail);
}

int CPythonTextTail::Pick(int ixMouse, int iyMouse)
{
	for (TTextTailMap::iterator itor = m_ItemTextTailMap.begin(); itor != m_ItemTextTailMap.end(); ++itor)
	{
		TTextTail* pTextTail = itor->second;

		if (ixMouse >= pTextTail->x + pTextTail->xStart && ixMouse <= pTextTail->x + pTextTail->xEnd &&
			iyMouse >= pTextTail->y + pTextTail->yStart && iyMouse <= pTextTail->y + pTextTail->yEnd)
		{
			SelectItemName(itor->first);
			return (itor->first);
		}
	}

	return -1;
}

void CPythonTextTail::SelectItemName(DWORD dwVirtualID)
{
	TTextTailMap::iterator itor = m_ItemTextTailMap.find(dwVirtualID);

	if (m_ItemTextTailMap.end() == itor)
		return;

	TTextTail* pTextTail = itor->second;
	pTextTail->pTextInstance->SetColor(0.1f, 0.9f, 0.1f);
}

void CPythonTextTail::AttachTitle(DWORD dwVID, const char* c_szName, const D3DXCOLOR& c_rColor)
{
	if (!bPKTitleEnable)
		return;
	TTextTailMap::iterator itor = m_CharacterTextTailMap.find(dwVID);
	if (m_CharacterTextTailMap.end() == itor)
		return;

	TTextTail* pTextTail = itor->second;

	CGraphicTextInstance*& prTitle = pTextTail->pTitleTextInstance;
	if (!prTitle)
	{
		prTitle = CGraphicTextInstance::New();
		prTitle->SetTextPointer(ms_pFont);
		prTitle->SetOutline(true);
		prTitle->SetHorizonalAlign(CGraphicTextInstance::HORIZONTAL_ALIGN_RIGHT);
		prTitle->SetVerticalAlign(CGraphicTextInstance::VERTICAL_ALIGN_BOTTOM);
	}

	prTitle->SetValue(c_szName);
	prTitle->SetColor(c_rColor.r, c_rColor.g, c_rColor.b);
	prTitle->Update();
}

void CPythonTextTail::DetachTitle(DWORD dwVID)
{
	if (!bPKTitleEnable)
		return;

	TTextTailMap::iterator itor = m_CharacterTextTailMap.find(dwVID);
	if (m_CharacterTextTailMap.end() == itor)
		return;

	TTextTail* pTextTail = itor->second;

	if (pTextTail->pTitleTextInstance)
	{
		CGraphicTextInstance::Delete(pTextTail->pTitleTextInstance);
		pTextTail->pTitleTextInstance = NULL;
	}
}

void CPythonTextTail::EnablePKTitle(BOOL bFlag)
{
	bPKTitleEnable = bFlag;
}

void CPythonTextTail::AttachLevel(DWORD dwVID, const char* c_szText, const D3DXCOLOR& c_rColor)
{
	if (!bPKTitleEnable)
		return;
	TTextTailMap::iterator itor = m_CharacterTextTailMap.find(dwVID);
	if (m_CharacterTextTailMap.end() == itor)
		return;

	TTextTail* pTextTail = itor->second;

	CGraphicTextInstance*& prLevel = pTextTail->pLevelTextInstance;
	if (!prLevel)
	{
		prLevel = CGraphicTextInstance::New();
		prLevel->SetTextPointer(ms_pFont);
		prLevel->SetOutline(true);
		prLevel->SetHorizonalAlign(CGraphicTextInstance::HORIZONTAL_ALIGN_RIGHT);
		prLevel->SetVerticalAlign(CGraphicTextInstance::VERTICAL_ALIGN_BOTTOM);
	}

	prLevel->SetValue(c_szText);
	prLevel->SetColor(c_rColor.r, c_rColor.g, c_rColor.b);
	prLevel->Update();
}

void CPythonTextTail::DetachLevel(DWORD dwVID)
{
	if (!bPKTitleEnable)
		return;
	TTextTailMap::iterator itor = m_CharacterTextTailMap.find(dwVID);
	if (m_CharacterTextTailMap.end() == itor)
		return;

	TTextTail* pTextTail = itor->second;

	if (pTextTail->pLevelTextInstance)
	{
		CGraphicTextInstance::Delete(pTextTail->pLevelTextInstance);
		pTextTail->pLevelTextInstance = NULL;
	}
}

#ifdef ENABLE_RANK_SYSTEM
void CPythonTextTail::AttachRank(DWORD dwVID, short sRank)
{
	if (!bPKTitleEnable)
		return;

	TTextTailMap::iterator itor = m_CharacterTextTailMap.find(dwVID);
	if (m_CharacterTextTailMap.end() == itor)
		return;

	TTextTail* pTextTail = itor->second;

	auto& pRankInstance = pTextTail->pRankImageInstance;
	auto& pRankInstance2 = pTextTail->pRankImageInstance2;

	if (!pRankInstance)
		pRankInstance = CGraphicImageInstance::New();
	if (!pRankInstance2)
		pRankInstance2 = CGraphicImageInstance::New();

	char szPath[256];
	sprintf(szPath, "d:/ymir work/rank/%d.png", sRank);

	char szPath2[256];
	if (0 < sRank && sRank <= 10)
		sprintf(szPath2, "d:/ymir work/rank/%d.png", sRank);
	else
		sprintf(szPath2, "d:/ymir work/rank/%d_1.png", sRank);

	pRankInstance->SetImagePointer((CGraphicImage*)CResourceManager::Instance().GetResourcePointer(szPath));
	pRankInstance2->SetImagePointer((CGraphicImage*)CResourceManager::Instance().GetResourcePointer(szPath2));
}
void CPythonTextTail::DeattachRank(DWORD dwVID)
{
	if (!bPKTitleEnable)
		return;

	TTextTailMap::iterator itor = m_CharacterTextTailMap.find(dwVID);
	if (m_CharacterTextTailMap.end() == itor)
		return;

	TTextTail* pTextTail = itor->second;

	if (pTextTail->pRankImageInstance)
	{
		CGraphicImageInstance::Delete(pTextTail->pRankImageInstance);
		pTextTail->pRankImageInstance = NULL;
	}
	if (pTextTail->pRankImageInstance2)
	{
		CGraphicImageInstance::Delete(pTextTail->pRankImageInstance2);
		pTextTail->pRankImageInstance2 = NULL;
	}
}
#endif

#ifdef ENABLE_TEAM_SYSTEM
void CPythonTextTail::AttachTeam(DWORD dwVID, short sTeam)
{
	if (!bPKTitleEnable)
		return;

	TTextTailMap::iterator itor = m_CharacterTextTailMap.find(dwVID);
	if (m_CharacterTextTailMap.end() == itor)
		return;

	TTextTail* pTextTail = itor->second;

	auto& pImageInstance = pTextTail->pTeamImageInstance;
	auto& pTextInstance = pTextTail->pTeamTextInstance;

	if (!pImageInstance)
		pImageInstance = CGraphicImageInstance::New();
	if (!pTextInstance)
	{
		pTextInstance = CGraphicTextInstance::New();
		pTextInstance->SetTextPointer(ms_pFont);
		pTextInstance->SetOutline(true);
	}

	char szPath[256];
	sprintf(szPath, "d:/ymir work/team/%d.tga", sTeam);
	pImageInstance->SetImagePointer((CGraphicImage*)CResourceManager::Instance().GetResourcePointer(szPath));

	pTextInstance->SetValue(teamNames[sTeam-1].first.c_str());
	pTextInstance->SetColor(teamNames[sTeam-1].second.first);
	pTextInstance->SetOutLineColor(teamNames[sTeam-1].second.second);
	pTextInstance->Update();
}
void CPythonTextTail::DeattachTeam(DWORD dwVID)
{
	if (!bPKTitleEnable)
		return;

	TTextTailMap::iterator itor = m_CharacterTextTailMap.find(dwVID);
	if (m_CharacterTextTailMap.end() == itor)
		return;

	TTextTail* pTextTail = itor->second;

	if (pTextTail->pTeamImageInstance)
	{
		CGraphicImageInstance::Delete(pTextTail->pTeamImageInstance);
		pTextTail->pTeamImageInstance = NULL;
	}
	if (pTextTail->pTeamTextInstance)
	{
		CGraphicTextInstance::Delete(pTextTail->pTeamTextInstance);
		pTextTail->pTeamTextInstance = NULL;
	}
}
#endif

#ifdef ENABLE_LANDRANK_SYSTEM
void CPythonTextTail::AttachLandRank(DWORD dwVID, const char* c_szName)
{
	if (!bPKTitleEnable)
		return;

	TTextTailMap::iterator itor = m_CharacterTextTailMap.find(dwVID);
	if (m_CharacterTextTailMap.end() == itor)
		return;

	TTextTail* pTextTail = itor->second;

	CGraphicTextInstance*& pLandRankInstance = pTextTail->pLandRankTextInstance;
	if (!pLandRankInstance)
	{
		pLandRankInstance = CGraphicTextInstance::New();
		pLandRankInstance->SetTextPointer(ms_pFont);
		pLandRankInstance->SetOutline(true);
		pLandRankInstance->SetHorizonalAlign(CGraphicTextInstance::HORIZONTAL_ALIGN_CENTER);
		pLandRankInstance->SetVerticalAlign(CGraphicTextInstance::VERTICAL_ALIGN_BOTTOM);
	}

	pLandRankInstance->SetValue(c_szName);
	D3DXCOLOR testColor = CInstanceBase::GetIndexedNameColor(CInstanceBase::NAMECOLOR_NORMAL_PC);
	pLandRankInstance->SetColor(testColor.r, testColor.g, testColor.b);
	pLandRankInstance->Update();
}

void CPythonTextTail::DetachLandRank(DWORD dwVID)
{
	if (!bPKTitleEnable)
		return;

	TTextTailMap::iterator itor = m_CharacterTextTailMap.find(dwVID);
	if (m_CharacterTextTailMap.end() == itor)
		return;

	TTextTail* pTextTail = itor->second;

	if (pTextTail->pLandRankTextInstance)
	{
		CGraphicTextInstance::Delete(pTextTail->pLandRankTextInstance);
		pTextTail->pLandRankTextInstance = NULL;
	}
}
#endif

#ifdef ENABLE_MONIKER_SYSTEM
void CPythonTextTail::AttachMoniker(DWORD dwVID, const char* moniker)
{
	if (!bPKTitleEnable)
		return;

	TTextTailMap::iterator itor = m_CharacterTextTailMap.find(dwVID);
	if (m_CharacterTextTailMap.end() == itor)
		return;

	TTextTail* pTextTail = itor->second;

	CGraphicTextInstance*& pMonikerInstance = pTextTail->pMonikerTextInstance;
	if (!pMonikerInstance)
	{
		pMonikerInstance = CGraphicTextInstance::New();
		pMonikerInstance->SetTextPointer(ms_pFont);
		pMonikerInstance->SetOutline(true);
		pMonikerInstance->SetHorizonalAlign(CGraphicTextInstance::HORIZONTAL_ALIGN_CENTER);
		pMonikerInstance->SetVerticalAlign(CGraphicTextInstance::VERTICAL_ALIGN_BOTTOM);
	}

	pMonikerInstance->SetValue(moniker);

	DWORD color = 0;
	color = D3DXCOLOR(0.19f, 0.84f, 0.78f, 1.0f); // sari
	if (strlen(moniker) > 20)
		color = D3DXCOLOR(0.0f, 0.0f, 1.0f, 1.0f); // mavi
	else if (strlen(moniker) > 15)
		color = D3DXCOLOR(1.0f, 0.0f, 0.0f, 1.0f); // kirmizi
	else if (strlen(moniker) > 10)
		color = D3DXCOLOR(0.84f, 0.0f, 1.0f, 1.0f); // mor
	else if (strlen(moniker) > 5)
		color = D3DXCOLOR(0.0f, 1.0f, 1.0f, 1.0f); // turkuaz

	pMonikerInstance->SetColor(color);

	pMonikerInstance->Update();
}

void CPythonTextTail::DetachMoniker(DWORD dwVID)
{
	if (!bPKTitleEnable)
		return;

	TTextTailMap::iterator itor = m_CharacterTextTailMap.find(dwVID);
	if (m_CharacterTextTailMap.end() == itor)
		return;

	TTextTail* pTextTail = itor->second;

	if (pTextTail->pMonikerTextInstance)
	{
		CGraphicTextInstance::Delete(pTextTail->pMonikerTextInstance);
		pTextTail->pMonikerTextInstance = NULL;
	}
}
#endif

#ifdef ENABLE_REBORN_SYSTEM
void CPythonTextTail::AttachReborn(DWORD dwVID, int reborn)
{
	if (!bPKTitleEnable)
		return;

	TTextTailMap::iterator itor = m_CharacterTextTailMap.find(dwVID);
	if (m_CharacterTextTailMap.end() == itor)
		return;

	TTextTail* pTextTail = itor->second;

	CGraphicTextInstance*& pRebornInstance = pTextTail->pRebornTextInstance;
	if (!pRebornInstance)
	{
		pRebornInstance = CGraphicTextInstance::New();
		pRebornInstance->SetTextPointer(ms_pFont);
		pRebornInstance->SetOutline(true);
		pRebornInstance->SetHorizonalAlign(CGraphicTextInstance::HORIZONTAL_ALIGN_CENTER);
		pRebornInstance->SetVerticalAlign(CGraphicTextInstance::VERTICAL_ALIGN_BOTTOM);
	}

	char szText[20];
	sprintf(szText, "Reborn  [ %d ]", reborn);
	pRebornInstance->SetValue(szText);

	DWORD color = 0;
	if (reborn <= 20)
		color = D3DXCOLOR(0.19f, 0.84f, 0.78f, 1.0f); // sari
	else if (reborn <= 40)
		color = D3DXCOLOR(0.0f, 0.0f, 1.0f, 1.0f); // mavi
	else if (reborn <= 60)
		color = D3DXCOLOR(1.0f, 0.0f, 0.0f, 1.0f); // kirmizi
	else if (reborn <= 80)
		color = D3DXCOLOR(0.84f, 0.0f, 1.0f, 1.0f); // mor
	else if (reborn <= 100)
		color = D3DXCOLOR(0.0f, 1.0f, 1.0f, 1.0f); // turkuaz

	pRebornInstance->SetColor(color);

	pRebornInstance->Update();
}

void CPythonTextTail::DetachReborn(DWORD dwVID)
{
	if (!bPKTitleEnable)
		return;

	TTextTailMap::iterator itor = m_CharacterTextTailMap.find(dwVID);
	if (m_CharacterTextTailMap.end() == itor)
		return;

	TTextTail* pTextTail = itor->second;

	if (pTextTail->pRebornTextInstance)
	{
		CGraphicTextInstance::Delete(pTextTail->pRebornTextInstance);
		pTextTail->pRebornTextInstance = NULL;
	}
}
#endif
#ifdef ENABLE_LOVE_SYSTEM
void CPythonTextTail::AttachLove(DWORD dwVID, const char* love1, const char* love2)
{
	if (!bPKTitleEnable)
		return;

	TTextTailMap::iterator itor = m_CharacterTextTailMap.find(dwVID);
	if (m_CharacterTextTailMap.end() == itor)
		return;

	TTextTail* pTextTail = itor->second;

	CGraphicTextInstance*& pLove1Instance = pTextTail->pLove1TextInstance;
	if (!pLove1Instance)
	{
		pLove1Instance = CGraphicTextInstance::New();
		pLove1Instance->SetTextPointer(ms_pFont);
		pLove1Instance->SetOutline(true);
		pLove1Instance->SetHorizonalAlign(CGraphicTextInstance::HORIZONTAL_ALIGN_CENTER);
		pLove1Instance->SetVerticalAlign(CGraphicTextInstance::VERTICAL_ALIGN_BOTTOM);
	}

	CGraphicTextInstance*& pLove2Instance = pTextTail->pLove2TextInstance;
	if (!pLove2Instance)
	{
		pLove2Instance = CGraphicTextInstance::New();
		pLove2Instance->SetTextPointer(ms_pFont);
		pLove2Instance->SetOutline(true);
		pLove2Instance->SetHorizonalAlign(CGraphicTextInstance::HORIZONTAL_ALIGN_CENTER);
		pLove2Instance->SetVerticalAlign(CGraphicTextInstance::VERTICAL_ALIGN_BOTTOM);
	}

	auto& pLoveImageInstance = pTextTail->pLoveImageInstance;
	if (!pLoveImageInstance)
		pLoveImageInstance = CGraphicImageInstance::New();

	pLoveImageInstance->SetImagePointer((CGraphicImage*)CResourceManager::Instance().GetResourcePointer("d:/ymir work/ui/heart.tga"));

	pLove1Instance->SetColor(c_TextTail_Chat_Color);
	pLove2Instance->SetColor(c_TextTail_Chat_Color);
	pLove1Instance->SetValue(love1);
	pLove2Instance->SetValue(love2);
	pLove1Instance->Update();
	pLove2Instance->Update();
}

void CPythonTextTail::DetachLove(DWORD dwVID)
{
	if (!bPKTitleEnable)
		return;

	TTextTailMap::iterator itor = m_CharacterTextTailMap.find(dwVID);
	if (m_CharacterTextTailMap.end() == itor)
		return;

	TTextTail* pTextTail = itor->second;

	if (pTextTail->pLove1TextInstance)
	{
		CGraphicTextInstance::Delete(pTextTail->pLove1TextInstance);
		pTextTail->pLove1TextInstance = NULL;
	}

	if (pTextTail->pLove2TextInstance)
	{
		CGraphicTextInstance::Delete(pTextTail->pLove2TextInstance);
		pTextTail->pLove2TextInstance = NULL;
	}

	if (pTextTail->pLoveImageInstance)
	{
		CGraphicImageInstance::Delete(pTextTail->pLoveImageInstance);
		pTextTail->pLoveImageInstance = NULL;
	}
}
#endif // ENABLE_LOVE_SYSTEM

#ifdef ENABLE_WORD_SYSTEM
void CPythonTextTail::AttachWord(DWORD dwVID, const char* word)
{
	if (!bPKTitleEnable)
		return;

	TTextTailMap::iterator itor = m_CharacterTextTailMap.find(dwVID);
	if (m_CharacterTextTailMap.end() == itor)
		return;

	TTextTail* pTextTail = itor->second;

	CGraphicTextInstance*& pWordInstance = pTextTail->pWordTextInstance;
	if (!pWordInstance)
	{
		pWordInstance = CGraphicTextInstance::New();
		pWordInstance->SetTextPointer(ms_pFont);
		pWordInstance->SetOutline(true);
		pWordInstance->SetHorizonalAlign(CGraphicTextInstance::HORIZONTAL_ALIGN_CENTER);
		pWordInstance->SetVerticalAlign(CGraphicTextInstance::VERTICAL_ALIGN_BOTTOM);
	}

	pWordInstance->SetColor(c_TextTail_Word_Name_Color);
	pWordInstance->SetValue(word);
	pWordInstance->Update();
}

void CPythonTextTail::DetachWord(DWORD dwVID)
{
	if (!bPKTitleEnable)
		return;

	TTextTailMap::iterator itor = m_CharacterTextTailMap.find(dwVID);
	if (m_CharacterTextTailMap.end() == itor)
		return;

	TTextTail* pTextTail = itor->second;

	if (pTextTail->pWordTextInstance)
	{
		CGraphicTextInstance::Delete(pTextTail->pWordTextInstance);
		pTextTail->pWordTextInstance = NULL;
	}
}
#endif // ENABLE_WORD_SYSTEM

void CPythonTextTail::Initialize()
{
	// DEFAULT_FONT
	//ms_pFont = (CGraphicText *)CResourceManager::Instance().GetTypeResourcePointer(g_strDefaultFontName.c_str());

	CGraphicText* pkDefaultFont = static_cast<CGraphicText*>(DefaultFont_GetResource());
	if (!pkDefaultFont)
	{
		TraceError("CPythonTextTail::Initialize - CANNOT_FIND_DEFAULT_FONT");
		return;
	}

	ms_pFont = pkDefaultFont;
	// END_OF_DEFAULT_FONT
}

void CPythonTextTail::Destroy()
{
	m_TextTailPool.Clear();
}

void CPythonTextTail::Clear()
{
	m_CharacterTextTailMap.clear();
	m_CharacterTextTailList.clear();
	m_ItemTextTailMap.clear();
	m_ItemTextTailList.clear();
#ifdef ENABLE_OFFLINE_SHOP_CITIES
	m_ShopTextTailMap.clear();
#endif
	m_ChatTailMap.clear();

	m_TextTailPool.Clear();
}

CPythonTextTail::CPythonTextTail()
{
	Clear();
}

CPythonTextTail::~CPythonTextTail()
{
	Destroy();
}
