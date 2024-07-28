#include "StdAfx.h"
#include "PythonChat.h"

#include "AbstractApplication.h"
#include "PythonCharacterManager.h"
#include "../eterbase/Timer.h"
#ifdef ENABLE_PB2_PREMIUM_SYSTEM
#include "PythonPlayer.h"
#include <boost/algorithm/string.hpp>
#endif // ENABLE_PB2_PREMIUM_SYSTEM
int CPythonChat::TChatSet::ms_iChatModeSize = CHAT_TYPE_MAX_NUM;

const float c_fStartDisappearingTime = 11.0f;
const int c_iMaxLineCount = 10; // marty de 5

///////////////////////////////////////////////////////////////////////////////////////////////////

CDynamicPool<CPythonChat::SChatLine> CPythonChat::SChatLine::ms_kPool;

void CPythonChat::SetChatColor(UINT eType, UINT r, UINT g, UINT b)
{
	if (eType >= CHAT_TYPE_MAX_NUM)
		return;

	DWORD dwColor = (0xff000000) | (r << 16) | (g << 8) | (b);
	m_akD3DXClrChat[eType] = D3DXCOLOR(dwColor);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

CPythonChat::SChatLine* CPythonChat::SChatLine::New()
{
	return ms_kPool.Alloc();
}

void CPythonChat::SChatLine::Delete(CPythonChat::SChatLine* pkChatLine)
{
	pkChatLine->Instance.Destroy();
#ifdef ENABLE_CHAT_SHOUT_REWORK
	if (pkChatLine->ImageInstance)
	{
		CGraphicImageInstance::Delete(pkChatLine->ImageInstance);
		pkChatLine->ImageInstance = NULL;
	}
#endif // ENABLE_CHAT_SHOUT_REWORK
	ms_kPool.Free(pkChatLine);
}

void CPythonChat::SChatLine::DestroySystem()
{
	ms_kPool.Destroy();
}

void CPythonChat::SChatLine::SetColor(DWORD dwID, DWORD dwColor)
{
	assert(dwID < CHAT_LINE_COLOR_ARRAY_MAX_NUM);

	if (dwID >= CHAT_LINE_COLOR_ARRAY_MAX_NUM)
		return;

	aColor[dwID] = dwColor;
}

void CPythonChat::SChatLine::SetColorAll(DWORD dwColor)
{
	for (int i = 0; i < CHAT_LINE_COLOR_ARRAY_MAX_NUM; ++i)
		aColor[i] = dwColor;
}

D3DXCOLOR& CPythonChat::SChatLine::GetColorRef(DWORD dwID)
{
	assert(dwID < CHAT_LINE_COLOR_ARRAY_MAX_NUM);

	if (dwID >= CHAT_LINE_COLOR_ARRAY_MAX_NUM)
	{
		static D3DXCOLOR color(1.0f, 0.0f, 0.0f, 1.0f);
		return color;
	}

	return aColor[dwID];
}

CPythonChat::SChatLine::SChatLine() : iType(0), fAppendedTime(0)
{
	for (int i = 0; i < CHAT_LINE_COLOR_ARRAY_MAX_NUM; ++i)
		aColor[i] = 0xff0000ff;
}
CPythonChat::SChatLine::~SChatLine()
{
	Instance.Destroy();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

int CPythonChat::CreateChatSet(DWORD dwID)
{
	m_ChatSetMap.emplace(dwID, TChatSet());
	return dwID;
}

void CPythonChat::UpdateViewMode(DWORD dwID)
{
	IAbstractApplication& rApp = IAbstractApplication::GetSingleton();

	float fcurTime = rApp.GetGlobalTime();
	//float felapsedTime = rApp.GetGlobalElapsedTime();

	TChatSet* pChatSet = GetChatSetPtr(dwID);
	if (!pChatSet)
		return;

	TChatLineList* pLineList = &(pChatSet->m_ShowingChatLineList);
	int iLineIndex = pLineList->size();
	int iHeight = -(int(pLineList->size() + 1) * pChatSet->m_iStep);

	TChatLineList::iterator itor;
	for (itor = pLineList->begin(); itor != pLineList->end();)
	{
		TChatLine* pChatLine = (*itor);

		D3DXCOLOR& rColor = pChatLine->GetColorRef(dwID);

		float fElapsedTime = (fcurTime - pChatLine->fAppendedTime);
		if (fElapsedTime >= c_fStartDisappearingTime || iLineIndex >= c_iMaxLineCount)
		{
			rColor.a -= rColor.a / 10.0f;

			if (rColor.a <= 0.1f)
			{
				itor = pLineList->erase(itor);
			}
			else
			{
				++itor;
			}
		}
		else
		{
			++itor;
		}

		/////

		iHeight += pChatSet->m_iStep;
		--iLineIndex;

		pChatLine->Instance.SetPosition(pChatSet->m_ix, pChatSet->m_iy + iHeight);
		pChatLine->Instance.SetColor(rColor);
		pChatLine->Instance.Update();
#ifdef ENABLE_CHAT_SHOUT_REWORK
		if (pChatLine->ImageInstance && (pChatLine->iType == CHAT_TYPE_SHOUT))
			pChatLine->ImageInstance->SetPosition(pChatSet->m_ix, pChatSet->m_iy + iHeight + 2);
#endif
	}
}

void CPythonChat::UpdateEditMode(DWORD dwID)
{
	TChatSet* pChatSet = GetChatSetPtr(dwID);
	if (!pChatSet)
		return;

	const int c_iAlphaLine = max(0, GetVisibleLineCount(dwID) - GetEditableLineCount(dwID) + 2);

	int iLineIndex = 0;
	float fAlpha = 0.0f;
	float fAlphaStep = 0.0f;

	if (c_iAlphaLine > 0)
		fAlphaStep = 1.0f / float(c_iAlphaLine);

	TChatLineList* pLineList = &(pChatSet->m_ShowingChatLineList);
	int iHeight = -(int(pLineList->size() + 1) * pChatSet->m_iStep);

	for (TChatLineList::iterator itor = pLineList->begin(); itor != pLineList->end(); ++itor)
	{
		TChatLine* pChatLine = (*itor);

		D3DXCOLOR& rColor = pChatLine->GetColorRef(dwID);

		if (iLineIndex < c_iAlphaLine)
		{
			rColor.a += (fAlpha - rColor.a) / 10.0f;
			fAlpha = fMIN(fAlpha + fAlphaStep, 1.0f);
		}
		else
		{
			rColor.a = fMIN(rColor.a + 0.05f, 1.0f);
		}

		iHeight += pChatSet->m_iStep;
		pChatLine->Instance.SetPosition(pChatSet->m_ix, pChatSet->m_iy + iHeight);
		pChatLine->Instance.SetColor(rColor);
		pChatLine->Instance.Update();
#ifdef ENABLE_CHAT_SHOUT_REWORK
		if (pChatLine->ImageInstance && (pChatLine->iType == CHAT_TYPE_SHOUT))
			pChatLine->ImageInstance->SetPosition(pChatSet->m_ix, pChatSet->m_iy + iHeight + 2);
#endif
	}
}

void CPythonChat::UpdateLogMode(DWORD dwID)
{
	TChatSet* pChatSet = GetChatSetPtr(dwID);
	if (!pChatSet)
		return;

	TChatLineList* pLineList = &(pChatSet->m_ShowingChatLineList);
	int iHeight = 0;

	for (TChatLineList::reverse_iterator itor = pLineList->rbegin(); itor != pLineList->rend(); ++itor)
	{
		TChatLine* pChatLine = (*itor);

		iHeight -= pChatSet->m_iStep;
		pChatLine->Instance.SetPosition(pChatSet->m_ix, pChatSet->m_iy + iHeight);
		pChatLine->Instance.SetColor(pChatLine->GetColorRef(dwID));
		pChatLine->Instance.Update();
#ifdef ENABLE_CHAT_SHOUT_REWORK
		if (pChatLine->ImageInstance && (pChatLine->iType == CHAT_TYPE_SHOUT))
			pChatLine->ImageInstance->SetPosition(pChatSet->m_ix, pChatSet->m_iy + iHeight + 2);
#endif
	}
}

void CPythonChat::Update(DWORD dwID)
{
	TChatSet* pChatSet = GetChatSetPtr(dwID);
	if (!pChatSet)
		return;

	switch (pChatSet->m_iBoardState)
	{
	case BOARD_STATE_VIEW:
		UpdateViewMode(dwID);
		break;
	case BOARD_STATE_EDIT:
		UpdateEditMode(dwID);
		break;
	case BOARD_STATE_LOG:
		UpdateLogMode(dwID);
		break;
	}

	DWORD dwcurTime = CTimer::Instance().GetCurrentMillisecond();
	for (TWaitChatList::iterator itor = m_WaitChatList.begin(); itor != m_WaitChatList.end();)
	{
		TWaitChat& rWaitChat = *itor;

		if (rWaitChat.dwAppendingTime < dwcurTime)
		{
			AppendChat(rWaitChat.iType, rWaitChat.strChat.c_str());

			itor = m_WaitChatList.erase(itor);
		}
		else
		{
			++itor;
		}
	}
}

void CPythonChat::Render(DWORD dwID)
{
	TChatLineList* pLineList = GetChatLineListPtr(dwID);
	if (!pLineList)
		return;

	for (TChatLineList::iterator itor = pLineList->begin(); itor != pLineList->end(); ++itor)
	{
		CGraphicTextInstance& rInstance = (*itor)->Instance;
		rInstance.Render();
#ifdef ENABLE_CHAT_SHOUT_REWORK
		TChatLine* pChatLine = (*itor);
		if (pChatLine->ImageInstance && (pChatLine->iType == CHAT_TYPE_SHOUT))
		{
			CGraphicImageInstance*& imInstance = (*itor)->ImageInstance;
			if (imInstance)
				imInstance->Render();
		}
#endif
	}
}

void CPythonChat::SetBoardState(DWORD dwID, int iState)
{
	TChatSet* pChatSet = GetChatSetPtr(dwID);
	if (!pChatSet)
		return;

	pChatSet->m_iBoardState = iState;
	ArrangeShowingChat(dwID);
}
void CPythonChat::SetPosition(DWORD dwID, int ix, int iy)
{
	TChatSet* pChatSet = GetChatSetPtr(dwID);
	if (!pChatSet)
		return;

	pChatSet->m_ix = ix;
	pChatSet->m_iy = iy;
}
void CPythonChat::SetHeight(DWORD dwID, int iHeight)
{
	TChatSet* pChatSet = GetChatSetPtr(dwID);
	if (!pChatSet)
		return;

	pChatSet->m_iHeight = iHeight;
}
void CPythonChat::SetStep(DWORD dwID, int iStep)
{
	TChatSet* pChatSet = GetChatSetPtr(dwID);
	if (!pChatSet)
		return;

	pChatSet->m_iStep = iStep;
}
void CPythonChat::ToggleChatMode(DWORD dwID, int iMode)
{
	TChatSet* pChatSet = GetChatSetPtr(dwID);
	if (!pChatSet)
		return;

	pChatSet->m_iMode[iMode] = 1 - pChatSet->m_iMode[iMode];
	// 	Tracef("ToggleChatMode : %d\n", iMode);
	ArrangeShowingChat(dwID);
}
void CPythonChat::EnableChatMode(DWORD dwID, int iMode)
{
	TChatSet* pChatSet = GetChatSetPtr(dwID);
	if (!pChatSet)
		return;

	pChatSet->m_iMode[iMode] = TRUE;
	// 	Tracef("EnableChatMode : %d\n", iMode);
	ArrangeShowingChat(dwID);
}
void CPythonChat::DisableChatMode(DWORD dwID, int iMode)
{
	TChatSet* pChatSet = GetChatSetPtr(dwID);
	if (!pChatSet)
		return;

	pChatSet->m_iMode[iMode] = FALSE;
	// 	Tracef("DisableChatMode : %d\n", iMode);
	ArrangeShowingChat(dwID);
}
void CPythonChat::SetEndPos(DWORD dwID, float fPos)
{
	TChatSet* pChatSet = GetChatSetPtr(dwID);
	if (!pChatSet)
		return;

	fPos = max(0.0f, fPos);
	fPos = min(1.0f, fPos);
	if (pChatSet->m_fEndPos != fPos)
	{
		pChatSet->m_fEndPos = fPos;
		ArrangeShowingChat(dwID);
	}
}

int CPythonChat::GetVisibleLineCount(DWORD dwID)
{
	TChatLineList* pLineList = GetChatLineListPtr(dwID);
	if (!pLineList)
		return 0;

	return pLineList->size();
}

int CPythonChat::GetEditableLineCount(DWORD dwID)
{
	TChatSet* pChatSet = GetChatSetPtr(dwID);
	if (!pChatSet)
		return 0;

	return pChatSet->m_iHeight / pChatSet->m_iStep + 1;
}

int CPythonChat::GetLineCount(DWORD dwID)
{
	TChatSet* pChatSet = GetChatSetPtr(dwID);
	if (!pChatSet)
		return 0;

	int iCount = 0;
	for (DWORD i = 0; i < m_ChatLineDeque.size(); ++i)
	{
		if (!pChatSet->CheckMode(m_ChatLineDeque[i]->iType))
			continue;

		++iCount;
	}

	return iCount;
}

int CPythonChat::GetLineStep(DWORD dwID)
{
	TChatSet* pChatSet = GetChatSetPtr(dwID);
	if (!pChatSet)
		return 0;

	return pChatSet->m_iStep;
}

CPythonChat::TChatLineList* CPythonChat::GetChatLineListPtr(DWORD dwID)
{
	TChatSetMap::iterator itor = m_ChatSetMap.find(dwID);
	if (m_ChatSetMap.end() == itor)
		return NULL;

	TChatSet& rChatSet = itor->second;
	return &(rChatSet.m_ShowingChatLineList);
}

CPythonChat::TChatSet* CPythonChat::GetChatSetPtr(DWORD dwID)
{
	TChatSetMap::iterator itor = m_ChatSetMap.find(dwID);
	if (m_ChatSetMap.end() == itor)
		return NULL;

	TChatSet& rChatSet = itor->second;
	return &rChatSet;
}

void CPythonChat::ArrangeShowingChat(DWORD dwID)
{
	TChatSet* pChatSet = GetChatSetPtr(dwID);
	if (!pChatSet)
		return;

	pChatSet->m_ShowingChatLineList.clear();

	TChatLineDeque TempChatLineDeque;
	for (TChatLineDeque::iterator itor = m_ChatLineDeque.begin(); itor != m_ChatLineDeque.end(); ++itor)
	{
		TChatLine* pChatLine = *itor;
		if (pChatSet->CheckMode(pChatLine->iType))
			TempChatLineDeque.push_back(pChatLine);
	}

	int icurLineCount = TempChatLineDeque.size();
	int iVisibleLineCount = min(icurLineCount, (pChatSet->m_iHeight + pChatSet->m_iStep) / pChatSet->m_iStep);
	int iEndLine = iVisibleLineCount + int(float(icurLineCount - iVisibleLineCount - 1) * pChatSet->m_fEndPos);

	/////

	int iHeight = 12;
	for (int i = min(icurLineCount - 1, iEndLine); i >= 0; --i)
	{
		if (!pChatSet->CheckMode(TempChatLineDeque[i]->iType))
			continue;

		if (pChatSet->m_iHeight + pChatSet->m_iStep <= iHeight)
		{
			break;
		}

		pChatSet->m_ShowingChatLineList.push_front(TempChatLineDeque[i]);

		iHeight += pChatSet->m_iStep;
	}
}

#ifdef ENABLE_CHAT_STOP
void CPythonChat::AppendChat(int iType, const char* c_szChat, DWORD appendTime)
#else
void CPythonChat::AppendChat(int iType, const char* c_szChat)
#endif
{
	// DEFAULT_FONT
	//static CResource * s_pResource = CResourceManager::Instance().GetResourcePointer(g_strDefaultFontName.c_str());
	CResource* pkNewFont = DefaultFont_GetResource();

	CGraphicText* pkDefaultFont = static_cast<CGraphicText*>(pkNewFont);
	if (!pkDefaultFont)
	{
		TraceError("CPythonChat::AppendChat - CANNOT_FIND_DEFAULT_FONT");
		return;
	}
	// END_OF_DEFAULT_FONT


#ifdef ENABLE_CHAT_STOP
	IAbstractApplication& rApp = IAbstractApplication::GetSingleton();
	if (m_isStopped)
	{
		TStoppedChat StoppedChat;
		StoppedChat.iType = iType;
		StoppedChat.strChat = c_szChat;
		StoppedChat.dwAppendingTime = rApp.GetGlobalTime();
		m_StoppedChatList.push_back(StoppedChat);
		if (m_StoppedChatList.size() > 20)
		{
			m_StoppedChatList.pop_front();
		}
		return;
	}
	else
	{
		IAbstractApplication& rApp = IAbstractApplication::GetSingleton();
	}
#endif

	SChatLine* pChatLine = SChatLine::New();
	pChatLine->iType = iType;

	pChatLine->ImageInstance = NULL;
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

	pChatLine->Instance.SetValue(strContent.c_str());
#else
	pChatLine->Instance.SetValue(c_szChat);
#endif // ENABLE_PB2_PREMIUM_SYSTEM


	// DEFAULT_FONT
	pChatLine->Instance.SetTextPointer(pkDefaultFont);
	// END_OF_DEFAULT_FONT

#ifdef ENABLE_CHAT_STOP
	if (appendTime == -1)
		pChatLine->fAppendedTime = rApp.GetGlobalTime();
	else
		pChatLine->fAppendedTime = appendTime;
#else
	pChatLine->fAppendedTime = rApp.GetGlobalTime();
#endif
	pChatLine->SetColorAll(GetChatColor(iType));

	m_ChatLineDeque.push_back(pChatLine);
	if (m_ChatLineDeque.size() > CHAT_LINE_MAX_NUM)
	{
		SChatLine* pChatLine = m_ChatLineDeque.front();
		if (pChatLine)
			SChatLine::Delete(pChatLine);
		m_ChatLineDeque.pop_front();
	}

	for (TChatSetMap::iterator itor = m_ChatSetMap.begin(); itor != m_ChatSetMap.end(); ++itor)
	{
		TChatSet* pChatSet = &(itor->second);
		//pChatLine->SetColor(itor->first, GetChatColor(iType));

		if (BOARD_STATE_EDIT == pChatSet->m_iBoardState)
		{
			ArrangeShowingChat(itor->first);
		}
		else// if (BOARD_STATE_VIEW == pChatSet->m_iBoardState)
		{
			pChatSet->m_ShowingChatLineList.push_back(pChatLine);
			if (pChatSet->m_ShowingChatLineList.size() > CHAT_LINE_MAX_NUM)
			{
				pChatSet->m_ShowingChatLineList.pop_front();
			}
		}
	}

#ifdef ENABLE_CHAT_SHOUT_REWORK
	if (iType == CHAT_TYPE_SHOUT)
	{
		CGraphicImageInstance*& prFlag = pChatLine->ImageInstance;
		std::string const chat = c_szChat;
		const char* empireflag = "";
		if (chat.find("[1]") != std::string::npos)
			empireflag = "shinsoo";
		else if (chat.find("[2]") != std::string::npos)
			empireflag = "chunjo";
		else if (chat.find("[3]") != std::string::npos)
			empireflag = "jinno";
		else
			empireflag = "none";

		if (empireflag != "")
		{
			char szPath[256];
			sprintf(szPath, "d:/ymir work/ui/game/chat/flag/%s.tga", empireflag);
			CGraphicImage* pFlagImage = (CGraphicImage*)CResourceManager::Instance().GetResourcePointer(szPath);
			prFlag = CGraphicImageInstance::New();
			prFlag->SetImagePointer(pFlagImage);
		}
	}
#endif
}

void CPythonChat::AppendChatWithDelay(int iType, const char* c_szChat, int iDelay)
{
	TWaitChat WaitChat;
	WaitChat.iType = iType;
	WaitChat.strChat = c_szChat;
	WaitChat.dwAppendingTime = CTimer::Instance().GetCurrentMillisecond() + iDelay;
	m_WaitChatList.push_back(WaitChat);
}

DWORD CPythonChat::GetChatColor(int iType)
{
	if (iType < CHAT_TYPE_MAX_NUM)
	{
		return m_akD3DXClrChat[iType];
	}

	return D3DXCOLOR(0.0f, 0.0f, 1.0f, 1.0f);
}
#ifdef ENABLE_CHAT_STACK_SYSTEM
void CPythonChat::AddChatStack(const char* c_szChat)
{
	std::string s; s += c_szChat;

	std::vector<std::string>::iterator position = std::find(m_WritedChatStack.begin(), m_WritedChatStack.end(), s);
	if (position != m_WritedChatStack.end())
		m_WritedChatStack.erase(position);

	m_WritedChatStack.push_back(s);
	if (m_WritedChatStack.size() > CHAT_STACK_MAX_NUM)
		m_WritedChatStack.erase(m_WritedChatStack.begin());
}
void CPythonChat::GetChatStack(BYTE index, std::string& stack)
{
	if (index > m_WritedChatStack.size())
		return;
	stack = m_WritedChatStack[m_WritedChatStack.size() - index];
	return;
}

BYTE CPythonChat::GetChatStackSize()
{
	return (unsigned char)m_WritedChatStack.size();
}
#endif

#ifdef ENABLE_CHAT_MODE_SYSTEM
BOOL CPythonChat::GetChatMode(DWORD dwID, int* iSum)
{
	TChatSet* pChatSet = GetChatSetPtr(dwID);
	if (!pChatSet)
		return FALSE;

#ifdef ENABLE_DICE_SYSTEM
	BYTE byChatHistoryLogModes[7] = { CHAT_TYPE_TALKING, CHAT_TYPE_INFO, CHAT_TYPE_NOTICE, CHAT_TYPE_PARTY, CHAT_TYPE_GUILD, CHAT_TYPE_SHOUT, CHAT_TYPE_DICE_INFO };
#else
	BYTE byChatHistoryLogModes[6] = { CHAT_TYPE_TALKING, CHAT_TYPE_INFO, CHAT_TYPE_NOTICE, CHAT_TYPE_PARTY, CHAT_TYPE_GUILD, CHAT_TYPE_SHOUT };
#endif

	int iRet = 0;
	for (BYTE i = 0; i < sizeof(byChatHistoryLogModes) / sizeof(*byChatHistoryLogModes); i++)
	{
		if (pChatSet->CheckMode(byChatHistoryLogModes[i]))
			iRet += 1 << byChatHistoryLogModes[i];
	}
	*iSum = iRet;
	return (iRet > 0) ? TRUE : FALSE;
}
#endif

void CPythonChat::IgnoreCharacter(const char* c_szName)
{
	TIgnoreCharacterSet::iterator itor = m_IgnoreCharacterSet.find(c_szName);
	if (m_IgnoreCharacterSet.end() != itor)
	{
		m_IgnoreCharacterSet.erase(itor);
	}
	else
	{
		m_IgnoreCharacterSet.insert(c_szName);
	}
}

BOOL CPythonChat::IsIgnoreCharacter(const char* c_szName)
{
	TIgnoreCharacterSet::iterator itor = m_IgnoreCharacterSet.find(c_szName);

	if (m_IgnoreCharacterSet.end() == itor)
		return FALSE;

	return TRUE;
}

CWhisper* CPythonChat::CreateWhisper(const char* c_szName)
{
	CWhisper* pWhisper = CWhisper::New();
	m_WhisperMap.insert(TWhisperMap::value_type(c_szName, pWhisper));
	return pWhisper;
}

void CPythonChat::AppendWhisper(int iType, const char* c_szName, const char* c_szChat)
{
	TWhisperMap::iterator itor = m_WhisperMap.find(c_szName);

	CWhisper* pWhisper;
	if (itor == m_WhisperMap.end())
	{
		pWhisper = CreateWhisper(c_szName);
	}
	else
	{
		pWhisper = itor->second;
	}

	pWhisper->AppendChat(iType, c_szChat);
}

void CPythonChat::ClearWhisper(const char* c_szName)
{
	TWhisperMap::iterator itor = m_WhisperMap.find(c_szName);

	if (itor != m_WhisperMap.end())
	{
		CWhisper* pWhisper = itor->second;
		CWhisper::Delete(pWhisper);

		m_WhisperMap.erase(itor);
	}
}

BOOL CPythonChat::GetWhisper(const char* c_szName, CWhisper** ppWhisper)
{
	TWhisperMap::iterator itor = m_WhisperMap.find(c_szName);

	if (itor == m_WhisperMap.end())
		return FALSE;

	*ppWhisper = itor->second;

	return TRUE;
}

void CPythonChat::InitWhisper(PyObject* ppyObject)
{
	TWhisperMap::iterator itor = m_WhisperMap.begin();
	for (; itor != m_WhisperMap.end(); ++itor)
	{
		std::string strName = itor->first;
		PyCallClassMemberFunc(ppyObject, "MakeWhisperButton", Py_BuildValue("(s)", strName.c_str()));
	}
}

void CPythonChat::__DestroyWhisperMap()
{
	TWhisperMap::iterator itor = m_WhisperMap.begin();
	for (; itor != m_WhisperMap.end(); ++itor)
	{
		CWhisper::Delete(itor->second);
	}
	m_WhisperMap.clear();
}

void CPythonChat::Close()
{
	TChatSetMap::iterator itor = m_ChatSetMap.begin();
	for (; itor != m_ChatSetMap.end(); ++itor)
	{
		TChatSet& rChatSet = itor->second;
		TChatLineList* pLineList = &(rChatSet.m_ShowingChatLineList);
		for (TChatLineList::iterator itor = pLineList->begin(); itor != pLineList->end(); ++itor)
		{
			TChatLine* pChatLine = (*itor);
			pChatLine->fAppendedTime = 0.0f;
		}
	}
}

void CPythonChat::Destroy()
{
	__DestroyWhisperMap();

	m_ShowingChatLineList.clear();
	m_ChatSetMap.clear();
	m_ChatLineDeque.clear();

	SChatLine::DestroySystem();
	CWhisper::DestroySystem();

	__Initialize();
}

#ifdef ENABLE_CLEAR_CHAT_SYSTEM
void CPythonChat::ClearChat()
{
	std::for_each(m_ChatLineDeque.begin(), m_ChatLineDeque.end(), SChatLine::Delete);
	m_ChatLineDeque.clear();
	m_ShowingChatLineList.clear();

	for (TChatSetMap::iterator itor = m_ChatSetMap.begin(); itor != m_ChatSetMap.end(); ++itor)
	{
		TChatSet* pChatSet = &(itor->second);

		if (BOARD_STATE_EDIT == pChatSet->m_iBoardState)
			ArrangeShowingChat(itor->first);
		else
			pChatSet->m_ShowingChatLineList.clear();
	}
}
#endif

#ifdef ENABLE_CHAT_STOP
void CPythonChat::StopChatToggle()
{
	if (m_isStopped)
		AppendChat(CHAT_TYPE_INFO, "Chat akýþý devam ediyor.");
	else
		AppendChat(CHAT_TYPE_INFO, "Chat akýþý durduruldu(Sadece kendi bilgisayarýnýzda).");

	m_isStopped = !m_isStopped;
	if (!m_isStopped)//Continue
	{
		for (TStoppedChatList::iterator itor = m_StoppedChatList.begin(); itor != m_StoppedChatList.end();++itor)
		{
			TStoppedChat & rStoppedChat = *itor;
			AppendChat(rStoppedChat.iType, rStoppedChat.strChat.c_str(), rStoppedChat.dwAppendingTime);
		}
		m_StoppedChatList.clear();
	}
}
bool CPythonChat::IsStoppedChat()
{
	return m_isStopped;
}
#endif

void CPythonChat::__Initialize()
{
	m_akD3DXClrChat[CHAT_TYPE_TALKING] = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	m_akD3DXClrChat[CHAT_TYPE_INFO] = D3DXCOLOR(1.0f, 0.785f, 0.785f, 1.0f);
	m_akD3DXClrChat[CHAT_TYPE_NOTICE] = D3DXCOLOR(1.0f, 0.902f, 0.730f, 1.0f);
	m_akD3DXClrChat[CHAT_TYPE_PARTY] = D3DXCOLOR(0.542f, 1.0f, 0.949f, 1.0f);
	m_akD3DXClrChat[CHAT_TYPE_GUILD] = D3DXCOLOR(0.906f, 0.847f, 1.0f, 1.0f);
	m_akD3DXClrChat[CHAT_TYPE_COMMAND] = D3DXCOLOR(0.658f, 1.0f, 0.835f, 1.0f);
	m_akD3DXClrChat[CHAT_TYPE_SHOUT] = D3DXCOLOR(0.658f, 1.0f, 0.835f, 1.0f);
	m_akD3DXClrChat[CHAT_TYPE_WHISPER] = D3DXCOLOR(0xff4AE14A);
	m_akD3DXClrChat[CHAT_TYPE_BIG_NOTICE] = D3DXCOLOR(1.0f, 0.902f, 0.730f, 1.0f);
#ifdef ENABLE_DICE_SYSTEM
	m_akD3DXClrChat[CHAT_TYPE_DICE_INFO] = D3DXCOLOR(0xFFcc00cc);
#endif
#ifdef ENABLE_CHAT_FILTER
	m_akD3DXClrChat[CHAT_TYPE_NOTICE_IMPROVING] = D3DXCOLOR(1.0f, 0.902f, 0.730f, 1.0f);
#endif
#ifdef ENABLE_CHAT_STOP
	m_isStopped = false;
#endif
}

CPythonChat::CPythonChat()
{
	__Initialize();
}

CPythonChat::~CPythonChat()
{
	assert(m_ChatLineDeque.empty());
	assert(m_ShowingChatLineList.empty());
	assert(m_ChatSetMap.empty());
	assert(m_WhisperMap.empty());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
CDynamicPool<CWhisper> CWhisper::ms_kPool;

CWhisper* CWhisper::New()
{
	return ms_kPool.Alloc();
}

void CWhisper::Delete(CWhisper* pkWhisper)
{
	pkWhisper->Destroy();
	ms_kPool.Free(pkWhisper);
}

void CWhisper::DestroySystem()
{
	ms_kPool.Destroy();

	SChatLine::DestroySystem();
}

void CWhisper::SetPosition(float fPosition)
{
	m_fcurPosition = fPosition;
	__ArrangeChat();
}

void CWhisper::SetBoxSize(float fWidth, float fHeight)
{
	m_fWidth = fWidth;
	m_fHeight = fHeight;

	for (TChatLineDeque::iterator itor = m_ChatLineDeque.begin(); itor != m_ChatLineDeque.end(); ++itor)
	{
		TChatLine* pChatLine = *itor;
		pChatLine->Instance.SetLimitWidth(fWidth);
	}
}

void CWhisper::AppendChat(int iType, const char* c_szChat)
{
	// DEFAULT_FONT
	//static CResource * s_pResource = CResourceManager::Instance().GetResourcePointer(g_strDefaultFontName.c_str());

	CGraphicText* pkDefaultFont = static_cast<CGraphicText*>(DefaultFont_GetResource());

	if (!pkDefaultFont)
	{
		TraceError("CWhisper::AppendChat - CANNOT_FIND_DEFAULT_FONT");
		return;
	}
	// END_OF_DEFAULT_FONT

	SChatLine* pChatLine = SChatLine::New();
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

	pChatLine->Instance.SetValue(strContent.c_str());
#else
	pChatLine->Instance.SetValue(c_szChat);
#endif // ENABLE_PB2_PREMIUM_SYSTEM

	// DEFAULT_FONT
	pChatLine->Instance.SetTextPointer(pkDefaultFont);
	// END_OF_DEFAULT_FONT

	pChatLine->Instance.SetLimitWidth(m_fWidth);
	pChatLine->Instance.SetMultiLine(TRUE);

	switch (iType)
	{
	case CPythonChat::WHISPER_TYPE_SYSTEM:
		pChatLine->Instance.SetColor(D3DXCOLOR(1.0f, 0.785f, 0.785f, 1.0f));
		break;
	case CPythonChat::WHISPER_TYPE_GM:
		pChatLine->Instance.SetColor(D3DXCOLOR(1.0f, 0.632f, 0.0f, 1.0f));
		break;
	case CPythonChat::WHISPER_TYPE_CHAT:
	default:
		pChatLine->Instance.SetColor(0xffffffff);
		break;
	}

	m_ChatLineDeque.push_back(pChatLine);

	__ArrangeChat();
}

void CWhisper::__ArrangeChat()
{
	for (TChatLineDeque::iterator itor = m_ChatLineDeque.begin(); itor != m_ChatLineDeque.end(); ++itor)
	{
		TChatLine* pChatLine = *itor;
		pChatLine->Instance.Update();
	}
}

void CWhisper::Render(float fx, float fy)
{
	float fHeight = fy + m_fHeight;
	int iViewCount = int(m_fHeight / m_fLineStep) - 1;
	int iLineCount = int(m_ChatLineDeque.size());
	int iStartLine = -1;
	if (iLineCount > iViewCount)
	{
		iStartLine = int(float(iLineCount - iViewCount) * m_fcurPosition) + iViewCount - 1;
	}
	else if (!m_ChatLineDeque.empty())
	{
		iStartLine = iLineCount - 1;
	}

	RECT Rect = { static_cast<LONG>(fx), static_cast<LONG>(fy), static_cast<LONG>(fx + m_fWidth), static_cast<LONG>(fy + m_fHeight) };

	for (int i = iStartLine; i >= 0; --i)
	{
		assert(i >= 0 && i < int(m_ChatLineDeque.size()));
		TChatLine* pChatLine = m_ChatLineDeque[i];

		WORD wLineCount = pChatLine->Instance.GetTextLineCount();
		fHeight -= wLineCount * m_fLineStep;

		pChatLine->Instance.SetPosition(fx, fHeight);
		pChatLine->Instance.Render(&Rect);

		if (fHeight < fy)
			break;
	}
}

void CWhisper::__Initialize()
{
	m_fLineStep = 15.0f;
	m_fWidth = 300.0f;
	m_fHeight = 120.0f;
	m_fcurPosition = 1.0f;
}

void CWhisper::Destroy()
{
	std::for_each(m_ChatLineDeque.begin(), m_ChatLineDeque.end(), SChatLine::Delete);
	m_ChatLineDeque.clear();
	m_ShowingChatLineList.clear();
}

CWhisper::CWhisper()
{
	__Initialize();
}

CWhisper::~CWhisper()
{
	Destroy();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

CDynamicPool<CWhisper::SChatLine> CWhisper::SChatLine::ms_kPool;

CWhisper::SChatLine* CWhisper::SChatLine::New()
{
	return ms_kPool.Alloc();
}

void CWhisper::SChatLine::Delete(CWhisper::SChatLine* pkChatLine)
{
	pkChatLine->Instance.Destroy();
	ms_kPool.Free(pkChatLine);
}

void CWhisper::SChatLine::DestroySystem()
{
	ms_kPool.Destroy();
}