#include "StdAfx.h"
#include "../eterBase/CRC32.h"
#include "../eterBase/Filename.h"
#include "PythonWindow.h"
#include "PythonSlotWindow.h"
#ifdef ENABLE_HIGHLIGHT_ITEM
#include "../UserInterface/PythonSystem.h"
#endif

using namespace UI;

class UI::CSlotWindow::CSlotButton : public CButton
{
public:
	enum ESlotButtonType
	{
		SLOT_BUTTON_TYPE_PLUS,
		SLOT_BUTTON_TYPE_COVER,
	};

public:
	CSlotButton(ESlotButtonType dwType, DWORD dwSlotNumber, CSlotWindow* pParent) : CButton(NULL)
	{
		m_dwSlotButtonType = dwType;
		m_dwSlotNumber = dwSlotNumber;
		m_pParent = pParent;
	}
	virtual ~CSlotButton()
	{
	}

	BOOL OnMouseLeftButtonDown()
	{
		if (!IsEnable())
			return TRUE;
		if (UI::CWindowManager::Instance().IsAttaching())
			return TRUE;

		m_isPressed = TRUE;
		Down();

		return TRUE;
	}
	BOOL OnMouseLeftButtonUp()
	{
		if (!IsEnable())
			return TRUE;
		if (!IsPressed())
			return TRUE;

		if (IsIn())
			SetCurrentVisual(&m_overVisual);
		else
			SetCurrentVisual(&m_upVisual);

		m_pParent->OnPressedSlotButton(m_dwSlotButtonType, m_dwSlotNumber);

		return TRUE;
	}

	void OnMouseOverIn()
	{
		if (IsEnable())
		{
			SetCurrentVisual(&m_overVisual);
		}
		m_pParent->OnOverInItem(m_dwSlotNumber);
	}
	void OnMouseOverOut()
	{
		if (IsEnable())
		{
			SetUp();
			SetCurrentVisual(&m_upVisual);
		}
		m_pParent->OnOverOutItem();
	}

protected:
	ESlotButtonType m_dwSlotButtonType;
	DWORD m_dwSlotNumber;
	CSlotWindow* m_pParent;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

class UI::CSlotWindow::CCoverButton : public CSlotButton
{
public:
	CCoverButton(ESlotButtonType dwType, DWORD dwSlotNumber, CSlotWindow* pParent) : CSlotButton(dwType, dwSlotNumber, pParent)
	{
		m_bLeftButtonEnable = TRUE;
		m_bRightButtonEnable = TRUE;
	}
	virtual ~CCoverButton()
	{
	}

	void SetLeftButtonEnable(BOOL bEnable)
	{
		m_bLeftButtonEnable = bEnable;
	}
	void SetRightButtonEnable(BOOL bEnable)
	{
		m_bRightButtonEnable = bEnable;
	}

	void OnRender()
	{
	}

	void RenderButton()
	{
		CButton::OnRender();
	}

	BOOL OnMouseLeftButtonDown()
	{
		if (!IsEnable())
			return TRUE;
		if (m_bLeftButtonEnable)
		{
			CButton::OnMouseLeftButtonDown();
		}
		else
		{
			m_pParent->OnMouseLeftButtonDown();
		}
		return TRUE;
	}
	BOOL OnMouseLeftButtonUp()
	{
		if (!IsEnable())
			return TRUE;
		if (m_bLeftButtonEnable)
		{
			CButton::OnMouseLeftButtonUp();
			m_pParent->OnMouseLeftButtonDown();
		}
		else
		{
			m_pParent->OnMouseLeftButtonUp();
		}
		return TRUE;
	}

	BOOL OnMouseRightButtonDown()
	{
		if (!IsEnable())
			return TRUE;
		if (m_bRightButtonEnable)
			CButton::OnMouseLeftButtonDown();
		return TRUE;
	}
	BOOL OnMouseRightButtonUp()
	{
		if (!IsEnable())
			return TRUE;
		m_pParent->OnMouseRightButtonDown();
		if (m_bRightButtonEnable)
			CButton::OnMouseLeftButtonUp();
		return TRUE;
	}

protected:
	BOOL m_bLeftButtonEnable;
	BOOL m_bRightButtonEnable;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

class UI::CSlotWindow::CCoolTimeFinishEffect : public CAniImageBox
{
public:
	CCoolTimeFinishEffect(CSlotWindow* pParent, DWORD dwSlotIndex) : CAniImageBox(NULL)
	{
		m_pParent = pParent;
		m_dwSlotIndex = dwSlotIndex;
	}
	virtual ~CCoolTimeFinishEffect()
	{
	}

	void OnEndFrame()
	{
		((CSlotWindow*)m_pParent)->ReserveDestroyCoolTimeFinishEffect(m_dwSlotIndex);
	}

protected:
	DWORD m_dwSlotIndex;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// Set & Append

void CSlotWindow::SetSlotType(DWORD dwType)
{
	m_dwSlotType = dwType;
}

void CSlotWindow::SetSlotStyle(DWORD dwStyle)
{
	m_dwSlotStyle = dwStyle;
}

void CSlotWindow::AppendSlot(DWORD dwIndex, int ixPosition, int iyPosition, int ixCellSize, int iyCellSize)
{
	TSlot Slot;
	Slot.pInstance = NULL;
	Slot.pNumberLine = NULL;
#ifdef ENABLE_SIGN_PLUS_ITEMS
	Slot.pPlusLine = NULL;
	Slot.ixPlusPosition = 0;
	Slot.iyPlusPosition = 0;
#endif
	Slot.pCoverButton = NULL;
#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
	Slot.pEvolutionButton = NULL;
#endif
#ifdef ENABLE_ACCE_SYSTEM
	Slot.bActiveEffect = false;
	Slot.byToggleColorType = 0;
#endif
	Slot.pSlotButton = NULL;
	Slot.pSignImage = NULL;
	Slot.pFinishCoolTimeEffect = NULL;
	Slot.bRenderBaseSlotImage = false;
	ClearSlot(&Slot);
	Slot.dwSlotNumber = dwIndex;
	Slot.dwCenterSlotNumber = dwIndex;
	Slot.ixPosition = ixPosition;
	Slot.iyPosition = iyPosition;
	Slot.ixCellSize = ixCellSize;
	Slot.iyCellSize = iyCellSize;
	m_SlotList.push_back(Slot);
}

void CSlotWindow::GetSlotPosition(DWORD dwIndex, int& x, int& y)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	x = pSlot->ixPosition;
	y = pSlot->iyPosition;
}

void CSlotWindow::SetCoverButton(DWORD dwIndex, const char* c_szUpImageName, const char* c_szOverImageName, const char* c_szDownImageName, const char* c_szDisableImageName, BOOL bLeftButtonEnable, BOOL bRightButtonEnable)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	CCoverButton*& rpCoverButton = pSlot->pCoverButton;

	if (!rpCoverButton)
	{
		rpCoverButton = new CCoverButton(CSlotButton::SLOT_BUTTON_TYPE_COVER, pSlot->dwSlotNumber, this);
		CWindowManager::Instance().SetParent(rpCoverButton, this);
	}

	rpCoverButton->SetLeftButtonEnable(bLeftButtonEnable);
	rpCoverButton->SetRightButtonEnable(bRightButtonEnable);
	rpCoverButton->SetUpVisual(c_szUpImageName);
	rpCoverButton->SetOverVisual(c_szOverImageName);
	rpCoverButton->SetDownVisual(c_szDownImageName);
	rpCoverButton->SetDisableVisual(c_szDisableImageName);
	rpCoverButton->Enable();
	rpCoverButton->Show();

	if (pSlot->pSlotButton)
	{
		SetTop(pSlot->pSlotButton);
	}
}

#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
void CSlotWindow::SetEvolutionButton(DWORD dwIndex, const char* c_szUpImageName, const char* c_szOverImageName, const char* c_szDownImageName, const char* c_szDisableImageName, BOOL bLeftButtonEnable, BOOL bRightButtonEnable)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	CCoverButton*& rpEvolutionButton = pSlot->pEvolutionButton;

	if (!rpEvolutionButton)
	{
		rpEvolutionButton = new CCoverButton(CSlotButton::SLOT_BUTTON_TYPE_COVER, pSlot->dwSlotNumber, this);
		CWindowManager::Instance().SetParent(rpEvolutionButton, this);
	}

	rpEvolutionButton->SetLeftButtonEnable(bLeftButtonEnable);
	rpEvolutionButton->SetRightButtonEnable(bRightButtonEnable);
	rpEvolutionButton->SetUpVisual(c_szUpImageName);
	rpEvolutionButton->SetOverVisual(c_szOverImageName);
	rpEvolutionButton->SetDownVisual(c_szDownImageName);
	rpEvolutionButton->SetDisableVisual(c_szDisableImageName);
	rpEvolutionButton->Enable();
	rpEvolutionButton->Show();

	if (pSlot->pSlotButton)
	{
		SetTop(pSlot->pSlotButton);
	}
}

void CSlotWindow::EnableEvolutionButton(DWORD dwIndex)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	if (!pSlot->pEvolutionButton)
		return;

	pSlot->pEvolutionButton->Enable();
}

void CSlotWindow::DisableEvolutionButton(DWORD dwIndex)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	if (!pSlot->pEvolutionButton)
		return;

	pSlot->pEvolutionButton->Disable();
}
#endif

void CSlotWindow::DeleteCoverButton(DWORD dwIndex)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	CCoverButton*& rpCoverButton = pSlot->pCoverButton;
	if (!rpCoverButton)
		return;

	rpCoverButton->Hide();
	rpCoverButton->Disable();
	rpCoverButton->DestroyHandle();
}

void CSlotWindow::EnableCoverButton(DWORD dwIndex)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	if (!pSlot->pCoverButton)
		return;

	pSlot->pCoverButton->Enable();
}

void CSlotWindow::DisableCoverButton(DWORD dwIndex)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	if (!pSlot->pCoverButton)
		return;

	pSlot->pCoverButton->Disable();
}

void CSlotWindow::SetAlwaysRenderCoverButton(DWORD dwIndex, bool bAlwaysRender)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	if (bAlwaysRender)
		SET_BIT(pSlot->dwState, SLOT_STATE_ALWAYS_RENDER_COVER);
	else
		REMOVE_BIT(pSlot->dwState, SLOT_STATE_ALWAYS_RENDER_COVER);
}

void CSlotWindow::ShowSlotBaseImage(DWORD dwIndex)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	pSlot->bRenderBaseSlotImage = true;
}

void CSlotWindow::HideSlotBaseImage(DWORD dwIndex)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	pSlot->bRenderBaseSlotImage = false;
}

BOOL CSlotWindow::IsDisableCoverButton(DWORD dwIndex)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return FALSE;

	if (!pSlot->pCoverButton)
		return FALSE;

	return pSlot->pCoverButton->IsDisable();
}

void CSlotWindow::SetSlotBaseImage(const char* c_szFileName, float fr, float fg, float fb, float fa)
{
	__CreateBaseImage(c_szFileName, fr, fg, fb, fa);
}

void CSlotWindow::SetSlotBaseImageScale(const char* c_szFileName, float fr, float fg, float fb, float fa, float sx, float sy)
{
	__CreateBaseImageScale(c_szFileName, fr, fg, fb, fa, sx, sy);
}

void CSlotWindow::AppendSlotButton(const char* c_szUpImageName, const char* c_szOverImageName, const char* c_szDownImageName)
{
	for (TSlotListIterator itor = m_SlotList.begin(); itor != m_SlotList.end(); ++itor)
	{
		TSlot& rSlot = *itor;
		CSlotButton*& rpSlotButton = rSlot.pSlotButton;

		if (!rpSlotButton)
		{
			rpSlotButton = new CSlotButton(CSlotButton::SLOT_BUTTON_TYPE_PLUS, rSlot.dwSlotNumber, this);
			rpSlotButton->AddFlag(FLAG_FLOAT);
			CWindowManager::Instance().SetParent(rpSlotButton, this);
		}

		rpSlotButton->SetUpVisual(c_szUpImageName);
		rpSlotButton->SetOverVisual(c_szOverImageName);
		rpSlotButton->SetDownVisual(c_szDownImageName);
		rpSlotButton->SetPosition(rSlot.ixPosition + 1, rSlot.iyPosition + 19);
		rpSlotButton->Hide();
	}
}

void CSlotWindow::AppendRequirementSignImage(const char* c_szImageName)
{
	for (TSlotListIterator itor = m_SlotList.begin(); itor != m_SlotList.end(); ++itor)
	{
		TSlot& rSlot = *itor;
		CImageBox*& rpSignImage = rSlot.pSignImage;

		if (!rpSignImage)
		{
			rpSignImage = new CImageBox(NULL);
			CWindowManager::Instance().SetParent(rpSignImage, this);
		}

		rpSignImage->LoadImage(c_szImageName);
		rpSignImage->Hide();
	}
}

BOOL CSlotWindow::HasSlot(DWORD dwIndex)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return FALSE;

	return TRUE;
}

void CSlotWindow::SetSlot(DWORD dwIndex, DWORD dwVirtualNumber, BYTE byWidth, BYTE byHeight, CGraphicImage* pImage, D3DXCOLOR& diffuseColor)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	if (pSlot->isItem)
		if (pSlot->dwItemIndex == dwVirtualNumber)
		{
			pSlot->dwState = 0;
			pSlot->isItem = TRUE;
			if (pImage && pSlot->pInstance)
			{
				pSlot->pInstance->SetImagePointer(pImage);
			}
			return;
		}

	ClearSlot(pSlot);
	pSlot->dwState = 0;
	pSlot->isItem = TRUE;
	pSlot->dwItemIndex = dwVirtualNumber;

	if (pImage)
	{
		assert(NULL == pSlot->pInstance);
		pSlot->pInstance = CGraphicExpandedImageInstance::New();
		pSlot->pInstance->SetDiffuseColor(diffuseColor.r, diffuseColor.g, diffuseColor.b, diffuseColor.a);
		pSlot->pInstance->SetImagePointer(pImage);
	}

	pSlot->byxPlacedItemSize = byWidth;
	pSlot->byyPlacedItemSize = byHeight;

	if (pSlot->pCoverButton)
	{
		pSlot->pCoverButton->Show();
	}

#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
	if (pSlot->pEvolutionButton)
	{
		pSlot->pEvolutionButton->Show();
	}
#endif
}

void CSlotWindow::SetCardSlot(DWORD dwIndex, DWORD dwVirtualNumber, BYTE byWidth, BYTE byHeight, const char* c_szFileName, D3DXCOLOR& diffuseColor)
{
	CGraphicImage* pImage = (CGraphicImage*)CResourceManager::Instance().GetResourcePointer(c_szFileName);
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	if (pSlot->isItem)
		if (pSlot->dwItemIndex == dwVirtualNumber)
		{
			pSlot->dwState = 0;
			pSlot->isItem = TRUE;
			if (pImage && pSlot->pInstance)
			{
				pSlot->pInstance->SetImagePointer(pImage);
			}
			return;
		}

	ClearSlot(pSlot);
	pSlot->dwState = 0;
	pSlot->isItem = TRUE;
	pSlot->dwItemIndex = dwVirtualNumber;

	if (pImage)
	{
		assert(NULL == pSlot->pInstance);
		pSlot->pInstance = CGraphicExpandedImageInstance::New();
		pSlot->pInstance->SetImagePointer(pImage);
		pSlot->pInstance->SetDiffuseColor(diffuseColor.r, diffuseColor.g, diffuseColor.b, diffuseColor.a);
	}

	pSlot->byxPlacedItemSize = byWidth;
	pSlot->byyPlacedItemSize = byHeight;

	if (pSlot->pCoverButton)
	{
		pSlot->pCoverButton->Show();
	}

#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
	if (pSlot->pEvolutionButton)
	{
		pSlot->pEvolutionButton->Show();
	}
#endif
}

void CSlotWindow::SetSlotScale(DWORD dwIndex, DWORD dwVirtualNumber, BYTE byWidth, BYTE byHeight, CGraphicImage* pImage, float sx, float sy, D3DXCOLOR& diffuseColor)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	if (pSlot->isItem)
		if (pSlot->dwItemIndex == dwVirtualNumber)
		{
			pSlot->dwState = 0;
			pSlot->isItem = TRUE;
			if (pImage && pSlot->pInstance)
			{
				pSlot->pInstance->SetImagePointer(pImage);
			}
			return;
		}

	ClearSlot(pSlot);
	pSlot->dwState = 0;
	pSlot->isItem = TRUE;
	pSlot->dwItemIndex = dwVirtualNumber;

	if (pImage)
	{
		assert(NULL == pSlot->pInstance);
		pSlot->pInstance = CGraphicExpandedImageInstance::New();
		pSlot->pInstance->SetDiffuseColor(diffuseColor.r, diffuseColor.g, diffuseColor.b, diffuseColor.a);
		pSlot->pInstance->SetImagePointer(pImage);
		pSlot->pInstance->SetScale(sx, sy);
	}

	pSlot->byxPlacedItemSize = byWidth;
	pSlot->byyPlacedItemSize = byHeight;

	if (pSlot->pCoverButton)
	{
		pSlot->pCoverButton->Show();
	}

#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
	if (pSlot->pEvolutionButton)
	{
		pSlot->pEvolutionButton->Show();
	}
#endif
}

void CSlotWindow::SetSlotCount(DWORD dwIndex, DWORD dwCount)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	if (dwCount <= 0)
	{
		if (pSlot->pNumberLine)
		{
			delete pSlot->pNumberLine;
			pSlot->pNumberLine = NULL;
		}
	}
	else
	{
		string szCount;

		unsigned long long index = 0;

		long long modIndex = 1;
		while (dwCount > 0)
		{
			if ((index + 1) % 4 == 0)
			{

				if (modIndex == 1)
					szCount = "k" + szCount;
				else if (modIndex == 2)
					szCount = "m" + szCount;
				else
					szCount = "t" + szCount;
				modIndex++;
			}
			else
			{
				szCount = std::to_string(dwCount % 10) + szCount;
				dwCount /= 10;
			}

			index++;
		}

		std::string::reverse_iterator checkPoint = szCount.rbegin();
		for (auto it = szCount.rbegin(); it != szCount.rend(); it++)
		{
			if (*it == '0')
			{
				continue;
			}
			if (*it >= '1' && *it <= '9')
			{
				break;
			}
			else // k m t
			{
				checkPoint = it;
			}
		}

		if (checkPoint != szCount.rbegin())
		{
			szCount.erase(checkPoint.base(), szCount.end());
		}

		if (!pSlot->pNumberLine)
		{
			CNumberLine* pNumberLine = new UI::CNumberLine(this);
			pNumberLine->SetHorizontalAlign(CNumberLine::HORIZONTAL_ALIGN_RIGHT);
			pNumberLine->Show();
			pSlot->pNumberLine = pNumberLine;
		}

		pSlot->pNumberLine->SetNumber(szCount.c_str());
	}
}

#ifdef ENABLE_SIGN_PLUS_ITEMS
void CSlotWindow::AppendPlusOnSlot(DWORD dwIndex, int ixPlusPosition, int iyPlusPosition)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	if (ixPlusPosition == 0 && iyPlusPosition == 0)
	{
		if (pSlot->pPlusLine)
		{
			delete pSlot->pPlusLine;
			pSlot->pPlusLine = NULL;
		}
	}
	else
	{
		if (!pSlot->pPlusLine)
		{
			CNumberLine* pPlusLine = new UI::CNumberLine(this);
			pPlusLine->SetPath("d:/ymir work/ui/public/number2/");
			pPlusLine->SetHorizontalAlign(CNumberLine::HORIZONTAL_ALIGN_RIGHT);
			pPlusLine->Show();
			pSlot->pPlusLine = pPlusLine;
		}

		pSlot->ixPlusPosition = ixPlusPosition;
		pSlot->iyPlusPosition = iyPlusPosition;

		pSlot->pPlusLine->SetNumber("+");
	}
}
#endif

void CSlotWindow::SetSlotCountNew(DWORD dwIndex, DWORD dwGrade, DWORD dwCount)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	if (dwCount <= 0)
	{
		if (pSlot->pNumberLine)
		{
			delete pSlot->pNumberLine;
			pSlot->pNumberLine = NULL;
		}
	}
	else
	{
		char szCount[16 + 1];

		switch (dwGrade)
		{
		case 0:
			_snprintf(szCount, sizeof(szCount), "%d", dwCount);
			break;
		case 1:
			_snprintf(szCount, sizeof(szCount), "m%d", dwCount);
			break;
		case 2:
			_snprintf(szCount, sizeof(szCount), "g%d", dwCount);
			break;
		case 3:
#ifdef ENABLE_SAGE_SKILL // ENABLE_EXPERT_SKILL
			_snprintf(szCount, sizeof(szCount), "p%d", dwCount);
#else
			_snprintf(szCount, sizeof(szCount), "p");
#endif
			break;
#ifdef ENABLE_SAGE_SKILL // ENABLE_EXPERT_SKILL
		case 4:
			_snprintf(szCount, sizeof(szCount), "s%d", dwCount);
			break;
#endif
#ifdef ENABLE_EXPERT_SKILL
		case 5:
			_snprintf(szCount, sizeof(szCount), "e%d", dwCount);
			break;
		case 6:
			_snprintf(szCount, sizeof(szCount), "d%d", dwCount);
			break;
		case 7:
			_snprintf(szCount, sizeof(szCount), "t");
			break;
#endif
		}

		if (!pSlot->pNumberLine)
		{
			CNumberLine* pNumberLine = new UI::CNumberLine(this);
			pNumberLine->SetHorizontalAlign(CNumberLine::HORIZONTAL_ALIGN_RIGHT);
			pNumberLine->Show();
			pSlot->pNumberLine = pNumberLine;
		}

		pSlot->pNumberLine->SetNumber(szCount);
	}
}

void CSlotWindow::SetSlotCoolTime(DWORD dwIndex, float fCoolTime, float fElapsedTime)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	pSlot->fCoolTime = fCoolTime;
	pSlot->fStartCoolTime = CTimer::Instance().GetCurrentSecond() - fElapsedTime;
}

// @duzenleme slotlarin cooltimesini aklinda tutuyor.
// fakat calistigini dusunmuyorum.
void CSlotWindow::StoreSlotCoolTime(DWORD dwKey, DWORD dwSlotIndex, float fCoolTime, float fElapsedTime)
{
	std::map<DWORD, SStoreCoolDown>::iterator it = m_CoolDownStore[dwKey].find(dwSlotIndex);
	if (it != m_CoolDownStore[dwKey].end())
	{
		it->second.fCoolTime = fCoolTime;
		it->second.fElapsedTime = CTimer::Instance().GetCurrentSecond() - fElapsedTime;
		it->second.bActive = false;
	}
	else
	{
		SStoreCoolDown m_storeCoolDown;
		m_storeCoolDown.fCoolTime = fCoolTime;
		m_storeCoolDown.fElapsedTime = CTimer::Instance().GetCurrentSecond() - fElapsedTime;
		m_storeCoolDown.bActive = false;
		m_CoolDownStore[dwKey].insert(std::map<DWORD, SStoreCoolDown>::value_type(dwSlotIndex, m_storeCoolDown));
	}
}

void CSlotWindow::RestoreSlotCoolTime(DWORD dwKey)
{
	for (std::map<DWORD, SStoreCoolDown>::iterator it = m_CoolDownStore[dwKey].begin(); it != m_CoolDownStore[dwKey].end(); it++)
	{
		TSlot* pSlot;
		if (!GetSlotPointer(it->first, &pSlot))
			return;

		pSlot->fCoolTime = it->second.fCoolTime;
		pSlot->fStartCoolTime = it->second.fElapsedTime;
		if (it->second.bActive)
			ActivateSlot(it->first);
		else
			DeactivateSlot(it->first);
	}
}

void CSlotWindow::ActivateSlot(DWORD dwIndex)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	pSlot->bActive = TRUE;

#ifdef ENABLE_HIGHLIGHT_ITEM
	if (!m_apSlotActiveEffect[0] || !m_apSlotActiveEffect[1] || !m_apSlotActiveEffect[2])
#else
	if (!m_pSlotActiveEffect)
#endif
	{
		__CreateSlotEnableEffect();
	}

	std::map<DWORD, SStoreCoolDown>::iterator it = m_CoolDownStore[1].find(dwIndex);
	if (it != m_CoolDownStore[1].end())
		it->second.bActive = true;
}

bool CSlotWindow::IsActivatedSlot(DWORD dwIndex)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return false;

	return pSlot->bActive;
}

bool CSlotWindow::IsActiveEffect(DWORD dwIndex)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return false;

	return pSlot->bActiveEffect;
}

void CSlotWindow::DeactivateSlot(DWORD dwIndex)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	pSlot->bActive = FALSE;

	std::map<DWORD, SStoreCoolDown>::iterator it = m_CoolDownStore[1].find(dwIndex);
	if (it != m_CoolDownStore[1].end())
		it->second.bActive = false;
}

void CSlotWindow::ClearSlot(DWORD dwIndex)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	ClearSlot(pSlot);
}

void CSlotWindow::ClearSlot(TSlot* pSlot)
{
	pSlot->bActive = FALSE;
#ifdef ENABLE_ACCE_SYSTEM
	pSlot->bActiveEffect = FALSE;
	pSlot->byToggleColorType = COLOR_TYPE_WHITE;
#endif

	pSlot->byxPlacedItemSize = 1;
	pSlot->byyPlacedItemSize = 1;

	pSlot->isItem = FALSE;
	pSlot->dwState = 0;
	pSlot->fCoolTime = 0.0f;
	pSlot->fStartCoolTime = 0.0f;
	pSlot->dwCenterSlotNumber = 0xffffffff;

	pSlot->dwItemIndex = 0;
	pSlot->bRenderBaseSlotImage = true;
	if (pSlot->pInstance)
	{
		CGraphicExpandedImageInstance::Delete(pSlot->pInstance);
		pSlot->pInstance = NULL;
	}
	if (pSlot->pCoverButton)
	{
		pSlot->pCoverButton->Hide();
	}
#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
	if (pSlot->pEvolutionButton)
	{
		pSlot->pEvolutionButton->Hide();
	}
#endif
	if (pSlot->pSlotButton)
	{
		pSlot->pSlotButton->Hide();
	}
	if (pSlot->pSignImage)
	{
		pSlot->pSignImage->Hide();
	}
	if (pSlot->pFinishCoolTimeEffect)
	{
		pSlot->pFinishCoolTimeEffect->Hide();
	}
}

void CSlotWindow::ClearAllSlot()
{
	Destroy();
}

void CSlotWindow::RefreshSlot()
{
	OnRefreshSlot();

	if (IsRendering())
	{
		TSlot* pSlot;
		if (GetPickedSlotPointer(&pSlot))
		{
			OnOverOutItem();
			OnOverInItem(pSlot->dwSlotNumber);
		}
	}
}

void CSlotWindow::OnRefreshSlot()
{
}

DWORD CSlotWindow::GetSlotCount()
{
	return m_SlotList.size();
}

void CSlotWindow::LockSlot(DWORD dwIndex)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	pSlot->dwState |= SLOT_STATE_LOCK;
}
void CSlotWindow::UnlockSlot(DWORD dwIndex)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	pSlot->dwState ^= SLOT_STATE_LOCK;
}
void CSlotWindow::SetCantUseSlot(DWORD dwIndex)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	pSlot->dwState |= SLOT_STATE_CANT_USE;
}
void CSlotWindow::SetUseSlot(DWORD dwIndex)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	pSlot->dwState ^= SLOT_STATE_CANT_USE;
}
void CSlotWindow::EnableSlot(DWORD dwIndex)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	REMOVE_BIT(pSlot->dwState, SLOT_STATE_DISABLE);
	//pSlot->dwState |= SLOT_STATE_DISABLE;
}
void CSlotWindow::DisableSlot(DWORD dwIndex)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;
	SET_BIT(pSlot->dwState, SLOT_STATE_DISABLE);
	//pSlot->dwState ^= SLOT_STATE_DISABLE;
}
#ifdef ENABLE_SLOT_MARKING_SYSTEM
void CSlotWindow::SetSlotHighlightedGreeen(DWORD dwIndex, DWORD dwColor)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	if (dwColor == 0)
		SET_BIT(pSlot->dwState, SLOT_STATE_HIGHLIGHT_GREEN);
	else
		SET_BIT(pSlot->dwState, SLOT_STATE_HIGHLIGHT_RED);

	SetCantMouseEventSlot(dwIndex);
}

void CSlotWindow::DisableSlotHighlightedGreen(DWORD dwIndex, DWORD dwColor)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	if (dwColor == 0)
		REMOVE_BIT(pSlot->dwState, SLOT_STATE_HIGHLIGHT_GREEN);
	else
		REMOVE_BIT(pSlot->dwState, SLOT_STATE_HIGHLIGHT_RED);

	SetCanMouseEventSlot(dwIndex);
}

void CSlotWindow::SetCanMouseEventSlot(DWORD dwIndex)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	REMOVE_BIT(pSlot->dwState, SLOT_STATE_CANT_MOUSE_EVENT);
	//pSlot->dwState |= SLOT_STATE_CANT_MOUSE_EVENT;
}

void CSlotWindow::SetCantMouseEventSlot(DWORD dwIndex)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	SET_BIT(pSlot->dwState, SLOT_STATE_CANT_MOUSE_EVENT);
	//pSlot->dwState ^= SLOT_STATE_CANT_MOUSE_EVENT;
}

void CSlotWindow::SetUsableSlotOnTopWnd(DWORD dwIndex)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	REMOVE_BIT(pSlot->dwState, SLOT_STATE_UNUSABLE);
	SetCanMouseEventSlot(dwIndex);
	//pSlot->dwState |= SLOT_STATE_UNUSABLE;
}

void CSlotWindow::SetUnusableSlotOnTopWnd(DWORD dwIndex)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	SET_BIT(pSlot->dwState, SLOT_STATE_UNUSABLE);
	SetCantMouseEventSlot(dwIndex);
	//pSlot->dwState ^= SLOT_STATE_UNUSABLE;
}
#endif

void CSlotWindow::SelectSlot(DWORD dwSelectingIndex)
{
	std::list<DWORD>::iterator itor = m_dwSelectedSlotIndexList.begin();
	for (; itor != m_dwSelectedSlotIndexList.end();)
	{
		if (dwSelectingIndex == *itor)
		{
			itor = m_dwSelectedSlotIndexList.erase(itor);
			return;
		}
		else
		{
			++itor;
		}
	}

	TSlot* pSlot;
	if (GetSlotPointer(dwSelectingIndex, &pSlot))
	{
		if (!pSlot->isItem)
			return;

		m_dwSelectedSlotIndexList.push_back(dwSelectingIndex);
	}
}

BOOL CSlotWindow::isSelectedSlot(DWORD dwIndex)
{
	std::list<DWORD>::iterator itor = m_dwSelectedSlotIndexList.begin();
	for (; itor != m_dwSelectedSlotIndexList.end(); ++itor)
	{
		if (dwIndex == *itor)
			return TRUE;
	}

	return FALSE;
}

void CSlotWindow::ClearSelected()
{
	m_dwSelectedSlotIndexList.clear();
}

DWORD CSlotWindow::GetSelectedSlotCount()
{
	return m_dwSelectedSlotIndexList.size();
}

DWORD CSlotWindow::GetSelectedSlotNumber(DWORD dwIndex)
{
	if (dwIndex >= m_dwSelectedSlotIndexList.size())
		return DWORD(-1);

	DWORD dwCount = 0;
	std::list<DWORD>::iterator itor = m_dwSelectedSlotIndexList.begin();
	for (; itor != m_dwSelectedSlotIndexList.end(); ++itor)
	{
		if (dwIndex == dwCount)
			break;

		++dwCount;
	}

	return *itor;
}

void CSlotWindow::ShowSlotButton(DWORD dwSlotNumber)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwSlotNumber, &pSlot))
		return;

	if (pSlot->pSlotButton)
	{
		pSlot->pSlotButton->Show();
	}
}

DWORD CSlotWindow::GetSlotItemVnum(DWORD dwSlotNumber)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwSlotNumber, &pSlot))
		return 0;

	return pSlot->dwItemIndex;
}

void CSlotWindow::HideAllSlotButton()
{
	for (TSlotListIterator itor = m_SlotList.begin(); itor != m_SlotList.end(); ++itor)
	{
		TSlot& rSlot = *itor;

		if (rSlot.pSlotButton)
		{
			rSlot.pSlotButton->Hide();
		}
	}
}

void CSlotWindow::ShowRequirementSign(DWORD dwSlotNumber)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwSlotNumber, &pSlot))
		return;

	if (!pSlot->pSignImage)
		return;

	pSlot->pSignImage->Show();
}

void CSlotWindow::HideRequirementSign(DWORD dwSlotNumber)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwSlotNumber, &pSlot))
		return;

	if (!pSlot->pSignImage)
		return;

	pSlot->pSignImage->Hide();
}

// Event

BOOL CSlotWindow::OnMouseLeftButtonDown()
{
	TSlot* pSlot;
	if (!GetPickedSlotPointer(&pSlot))
	{
		UI::CWindowManager::Instance().DeattachIcon();
		return TRUE;
	}

#ifdef ENABLE_SLOT_MARKING_SYSTEM
	if (!(pSlot->dwState & SLOT_STATE_CANT_MOUSE_EVENT))
		if (pSlot->isItem && !(pSlot->dwState & SLOT_STATE_LOCK))
			OnSelectItemSlot(pSlot->dwSlotNumber);
		else
			OnSelectEmptySlot(pSlot->dwSlotNumber);
#else
	if (pSlot->isItem && !(pSlot->dwState & SLOT_STATE_LOCK))
		OnSelectItemSlot(pSlot->dwSlotNumber);
	else
		OnSelectEmptySlot(pSlot->dwSlotNumber);
#endif

	return TRUE;
}

BOOL CSlotWindow::OnMouseLeftButtonUp()
{
	if (UI::CWindowManager::Instance().IsAttaching())
		if (UI::CWindowManager::Instance().IsDragging())
			if (IsIn())
			{
				UI::CWindow* pWin = UI::CWindowManager::Instance().GetPointWindow();

				if (pWin)
					if (pWin->IsType(UI::CSlotWindow::Type()))
					{
						UI::CSlotWindow* pSlotWin = (UI::CSlotWindow*)pWin;

						TSlot* pSlot;
						if (!pSlotWin->GetPickedSlotPointer(&pSlot))
						{
							UI::CWindowManager::Instance().DeattachIcon();
							return TRUE;
						}

#ifdef ENABLE_SLOT_MARKING_SYSTEM
						if (!(pSlot->dwState & SLOT_STATE_CANT_MOUSE_EVENT))
							if (pSlot->isItem)
								pSlotWin->OnSelectItemSlot(pSlot->dwSlotNumber);
							else
								pSlotWin->OnSelectEmptySlot(pSlot->dwSlotNumber);
#else
						if (pSlot->isItem)
							pSlotWin->OnSelectItemSlot(pSlot->dwSlotNumber);
						else
							pSlotWin->OnSelectEmptySlot(pSlot->dwSlotNumber);
#endif

						return TRUE;
					}

			}

	CWindow::OnMouseLeftButtonUp();

	return FALSE;
}

BOOL CSlotWindow::OnMouseRightButtonDown()
{
	TSlot* pSlot;
	if (!GetPickedSlotPointer(&pSlot))
		return TRUE;

#ifdef ENABLE_SLOT_MARKING_SYSTEM
	if (!(pSlot->dwState & SLOT_STATE_CANT_MOUSE_EVENT))
		if (pSlot->isItem)
			OnUnselectItemSlot(pSlot->dwSlotNumber);
		else
			OnUnselectEmptySlot(pSlot->dwSlotNumber);
#else
	if (pSlot->isItem)
		OnUnselectItemSlot(pSlot->dwSlotNumber);
	else
		OnUnselectEmptySlot(pSlot->dwSlotNumber);
#endif

	return TRUE;
}

BOOL CSlotWindow::OnMouseLeftButtonDoubleClick()
{
	TSlot* pSlot;
#ifdef ENABLE_SLOT_MARKING_SYSTEM
	if (GetPickedSlotPointer(&pSlot))
		if (!(pSlot->dwState & SLOT_STATE_CANT_MOUSE_EVENT))
			OnUseSlot();
#else
	if (GetPickedSlotPointer(&pSlot))
		OnUseSlot();
#endif

	return TRUE;
}

void CSlotWindow::OnMouseOverOut()
{
	OnOverOutItem();
}

void CSlotWindow::OnMouseOver()
{
	//	if (UI::CWindowManager::Instance().IsCapture())
	//	if (!UI::CWindowManager::Instance().IsAttaching())
	//		return;

	CWindow* pPointWindow = UI::CWindowManager::Instance().GetPointWindow();
	if (this == pPointWindow)
	{
		TSlot* pSlot;
		if (GetPickedSlotPointer(&pSlot))
		{
			if (OnOverInItem(pSlot->dwSlotNumber))
				return;
		}
	}

	OnOverOutItem();
}

void CSlotWindow::OnSelectEmptySlot(int iSlotNumber)
{
	PyCallClassMemberFunc(m_poHandler, "OnSelectEmptySlot", Py_BuildValue("(i)", iSlotNumber));
}
void CSlotWindow::OnSelectItemSlot(int iSlotNumber)
{
	//	OnOverOutItem();
	PyCallClassMemberFunc(m_poHandler, "OnSelectItemSlot", Py_BuildValue("(i)", iSlotNumber));

	if (UI::CWindowManager::Instance().IsAttaching())
		OnOverOutItem();
}
void CSlotWindow::OnUnselectEmptySlot(int iSlotNumber)
{
	PyCallClassMemberFunc(m_poHandler, "OnUnselectEmptySlot", Py_BuildValue("(i)", iSlotNumber));
}
void CSlotWindow::OnUnselectItemSlot(int iSlotNumber)
{
	PyCallClassMemberFunc(m_poHandler, "OnUnselectItemSlot", Py_BuildValue("(i)", iSlotNumber));
}
void CSlotWindow::OnUseSlot()
{
	TSlot* pSlot;
	if (GetPickedSlotPointer(&pSlot))
		if (pSlot->isItem)
		{
			PyCallClassMemberFunc(m_poHandler, "OnUseSlot", Py_BuildValue("(i)", pSlot->dwSlotNumber));
		}
}

BOOL CSlotWindow::OnOverInItem(DWORD dwSlotNumber)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwSlotNumber, &pSlot))
		return FALSE;

	if (!pSlot->isItem)
		return FALSE;

	if (pSlot->dwSlotNumber == m_dwToolTipSlotNumber)
		return TRUE;

	m_dwToolTipSlotNumber = dwSlotNumber;
	PyCallClassMemberFunc(m_poHandler, "OnOverInItem", Py_BuildValue("(i)", dwSlotNumber));

	return TRUE;
}

void CSlotWindow::OnOverOutItem()
{
	if (SLOT_NUMBER_NONE == m_dwToolTipSlotNumber)
		return;

	m_dwToolTipSlotNumber = SLOT_NUMBER_NONE;
	PyCallClassMemberFunc(m_poHandler, "OnOverOutItem", Py_BuildValue("()"));
}

void CSlotWindow::OnPressedSlotButton(DWORD dwType, DWORD dwSlotNumber, BOOL isLeft)
{
	if (CSlotButton::SLOT_BUTTON_TYPE_PLUS == dwType)
	{
		PyCallClassMemberFunc(m_poHandler, "OnPressedSlotButton", Py_BuildValue("(i)", dwSlotNumber));
	}
	else if (CSlotButton::SLOT_BUTTON_TYPE_COVER == dwType)
	{
		if (isLeft)
		{
			OnMouseLeftButtonDown();
		}
	}
}

void CSlotWindow::OnUpdate()
{
	for (std::deque<DWORD>::iterator itor = m_ReserveDestroyEffectDeque.begin(); itor != m_ReserveDestroyEffectDeque.end(); ++itor)
	{
		DWORD dwSlotIndex = *itor;

		TSlot* pSlot;
		if (!GetSlotPointer(dwSlotIndex, &pSlot))
			continue;

		__DestroyFinishCoolTimeEffect(pSlot);
	}
	m_ReserveDestroyEffectDeque.clear();

#ifdef ENABLE_HIGHLIGHT_ITEM
	for (int i = 0; i < 3; ++i)
	{
		if (m_apSlotActiveEffect[i])
			m_apSlotActiveEffect[i]->Update();
	}
#else
	if (m_pSlotActiveEffect)
		m_pSlotActiveEffect->Update();
#endif

#ifdef ENABLE_ACCE_SYSTEM
	for (int i = 0; i < 3; ++i)
	{
		if (m_apSlotActiveEffectEx[i])
			m_apSlotActiveEffectEx[i]->Update();
	}
#endif
}

void CSlotWindow::OnRender()
{
	RenderSlotBaseImage();

	switch (m_dwSlotStyle)
	{
	case SLOT_STYLE_PICK_UP:
		OnRenderPickingSlot();
		break;
	case SLOT_STYLE_SELECT:
		OnRenderSelectedSlot();
		break;
	}

	std::for_each(m_pChildList.begin(), m_pChildList.end(), std::void_mem_fun(&CWindow::OnRender));

	TSlotListIterator itor;

	for (itor = m_SlotList.begin(); itor != m_SlotList.end(); ++itor)
	{
		TSlot& rSlot = *itor;

		if (rSlot.pSlotButton)
		{
			rSlot.pSlotButton->SetPosition(rSlot.ixPosition + 1, rSlot.iyPosition + 19);
		}
		if (rSlot.pSignImage)
		{
			rSlot.pSignImage->SetPosition(rSlot.ixPosition - 7, rSlot.iyPosition + 10);
		}

		if (rSlot.pInstance)
		{
			rSlot.pInstance->SetPosition(m_rect.left + rSlot.ixPosition, m_rect.top + rSlot.iyPosition);
			rSlot.pInstance->Render();
		}

#ifdef ENABLE_SLOT_MARKING_SYSTEM
		if (IS_SET(rSlot.dwState, SLOT_STATE_HIGHLIGHT_GREEN))
		{
			CPythonGraphic::Instance().SetDiffuseColor(74.0f / 255.f, 1.0f, 74.0f / 255.0f, 0.3f);
			CPythonGraphic::Instance().RenderBar2d(m_rect.left + rSlot.ixPosition,
				m_rect.top + rSlot.iyPosition,
				m_rect.left + rSlot.ixPosition + rSlot.byxPlacedItemSize * rSlot.ixCellSize,
				m_rect.top + rSlot.iyPosition + rSlot.byyPlacedItemSize * rSlot.iyCellSize);
		}

		if (IS_SET(rSlot.dwState, SLOT_STATE_HIGHLIGHT_RED))
		{
			CPythonGraphic::Instance().SetDiffuseColor(1.0f, 0.0f, 0.0f, 0.3f);
			CPythonGraphic::Instance().RenderBar2d(m_rect.left + rSlot.ixPosition,
				m_rect.top + rSlot.iyPosition,
				m_rect.left + rSlot.ixPosition + rSlot.byxPlacedItemSize * rSlot.ixCellSize,
				m_rect.top + rSlot.iyPosition + rSlot.byyPlacedItemSize * rSlot.iyCellSize);
		}

		if (IS_SET(rSlot.dwState, SLOT_STATE_UNUSABLE))
		{
			CPythonGraphic::Instance().SetDiffuseColor(1.0f, 1.0f, 1.0f, 0.3f);
			CPythonGraphic::Instance().RenderBar2d(m_rect.left + rSlot.ixPosition,
				m_rect.top + rSlot.iyPosition,
				m_rect.left + rSlot.ixPosition + rSlot.byxPlacedItemSize * ITEM_WIDTH,
				m_rect.top + rSlot.iyPosition + rSlot.byyPlacedItemSize * ITEM_HEIGHT);
		}
#endif

		if (!rSlot.isItem)
		{
			if (IS_SET(rSlot.dwState, SLOT_STATE_ALWAYS_RENDER_COVER))
			{
				rSlot.pCoverButton->Show();
				rSlot.pCoverButton->SetPosition(rSlot.ixPosition, rSlot.iyPosition);
				rSlot.pCoverButton->RenderButton();
			}

			continue;
		}

		if (IS_SET(rSlot.dwState, SLOT_STATE_DISABLE))
		{
			CPythonGraphic::Instance().SetDiffuseColor(1.0f, 0.0f, 0.0f, 0.3f);
			CPythonGraphic::Instance().RenderBar2d(m_rect.left + rSlot.ixPosition,
				m_rect.top + rSlot.iyPosition,
				m_rect.left + rSlot.ixPosition + rSlot.ixCellSize,
				m_rect.top + rSlot.iyPosition + rSlot.iyCellSize);
		}

		if (rSlot.fCoolTime != 0.0f)
		{
			float fcurTime = CTimer::Instance().GetCurrentSecond();
			float fPercentage = (fcurTime - rSlot.fStartCoolTime) / rSlot.fCoolTime;
			CPythonGraphic::Instance().RenderCoolTimeBox(m_rect.left + rSlot.ixPosition + 16.0f, m_rect.top + rSlot.iyPosition + 16.0f, 16.0f, fPercentage);

			if (fcurTime - rSlot.fStartCoolTime >= rSlot.fCoolTime)
			{
				if ((fcurTime - rSlot.fStartCoolTime) - rSlot.fCoolTime < 1.0f)
					__CreateFinishCoolTimeEffect(&rSlot);

				rSlot.fCoolTime = 0.0f;
				rSlot.fStartCoolTime = 0.0f;
			}
		}
		if (rSlot.pCoverButton)
		{
			rSlot.pCoverButton->SetPosition(rSlot.ixPosition, rSlot.iyPosition);
			rSlot.pCoverButton->RenderButton();
		}

#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
		if (rSlot.pEvolutionButton)
		{
			int ix = rSlot.byxPlacedItemSize * ITEM_WIDTH + rSlot.ixPosition - 13;
			int iy = rSlot.iyPosition + rSlot.byyPlacedItemSize * ITEM_HEIGHT - 13;
			rSlot.pEvolutionButton->SetPosition(ix, iy);
			rSlot.pEvolutionButton->RenderButton();
		}
#endif

		if (rSlot.pNumberLine)
		{
			int ix = rSlot.byxPlacedItemSize * ITEM_WIDTH + rSlot.ixPosition - 4;
			int iy = rSlot.iyPosition + rSlot.byyPlacedItemSize + 2;
			rSlot.pNumberLine->SetPosition(ix, iy);
			rSlot.pNumberLine->Update();
			rSlot.pNumberLine->Render();
		}
#ifdef ENABLE_SIGN_PLUS_ITEMS
		if (rSlot.pPlusLine)
		{
			int ix = rSlot.ixPosition + rSlot.byxPlacedItemSize + rSlot.ixPlusPosition;
			int iy = rSlot.iyPosition + rSlot.byyPlacedItemSize + rSlot.iyPlusPosition;
			rSlot.pPlusLine->SetPosition(ix, iy);
			rSlot.pPlusLine->Update();
			rSlot.pPlusLine->Render();
		}
#endif
		if (rSlot.pFinishCoolTimeEffect)
		{
			rSlot.pFinishCoolTimeEffect->SetPosition(rSlot.ixPosition, rSlot.iyPosition);
			rSlot.pFinishCoolTimeEffect->Update();
			rSlot.pFinishCoolTimeEffect->Render();
		}

#ifdef ENABLE_ACCE_SYSTEM
		if (rSlot.bActiveEffect)
		{
			int ix = m_rect.left + rSlot.ixPosition;
			int iy = m_rect.top + rSlot.iyPosition;
			int highlight = rSlot.byyPlacedItemSize;

			{
				if (highlight == 1)
				{
					m_apSlotActiveEffectEx[0]->Show();
					m_apSlotActiveEffectEx[0]->SetPosition(ix, iy);
					m_apSlotActiveEffectEx[0]->SetDiffuseColorByType(rSlot.byToggleColorType);
					m_apSlotActiveEffectEx[0]->Render();
				}

				if (highlight == 2)
				{
					m_apSlotActiveEffectEx[1]->Show();
					m_apSlotActiveEffectEx[1]->SetPosition(ix, iy);
					m_apSlotActiveEffectEx[1]->SetDiffuseColorByType(rSlot.byToggleColorType);
					m_apSlotActiveEffectEx[1]->Render();
				}

				if (highlight == 3)
				{
					m_apSlotActiveEffectEx[2]->Show();
					m_apSlotActiveEffectEx[2]->SetPosition(ix, iy);
					m_apSlotActiveEffectEx[2]->SetDiffuseColorByType(rSlot.byToggleColorType);
					m_apSlotActiveEffectEx[2]->Render();
				}
			}
		}
#endif

		if (rSlot.bActive)
		{
			int ix = m_rect.left + rSlot.ixPosition;
			int iy = m_rect.top + rSlot.iyPosition;
#ifdef ENABLE_HIGHLIGHT_ITEM
			int highlight = rSlot.byyPlacedItemSize;
			{
				if (highlight == 1)
				{
					m_apSlotActiveEffect[0]->Show();
					m_apSlotActiveEffect[0]->SetPosition(ix, iy);
					m_apSlotActiveEffect[0]->Render();
				}

				if (highlight == 2)
				{
					m_apSlotActiveEffect[1]->Show();
					m_apSlotActiveEffect[1]->SetPosition(ix, iy);
					m_apSlotActiveEffect[1]->Render();
				}

				if (highlight == 3)
				{
					m_apSlotActiveEffect[2]->Show();
					m_apSlotActiveEffect[2]->SetPosition(ix, iy);
					m_apSlotActiveEffect[2]->Render();
				}
			}
#else
			if (m_pSlotActiveEffect)
			{
				m_pSlotActiveEffect->SetPosition(ix, iy);
				m_pSlotActiveEffect->Render();
			}
#endif
		}
	}

	RenderLockedSlot();
}

void CSlotWindow::RenderSlotBaseImage()
{
	if (!m_pBaseImageInstance)
		return;

	for (TSlotListIterator itor = m_SlotList.begin(); itor != m_SlotList.end(); ++itor)
	{
		TSlot& rSlot = *itor;

		if (!rSlot.bRenderBaseSlotImage)
			continue;

		m_pBaseImageInstance->SetPosition(m_rect.left + rSlot.ixPosition, m_rect.top + rSlot.iyPosition);
		m_pBaseImageInstance->Render();
	}
}

void CSlotWindow::OnRenderPickingSlot()
{
	if (!UI::CWindowManager::Instance().IsAttaching())
		return;

	TSlot* pSlot;
	if (!GetSelectedSlotPointer(&pSlot))
		return;

	CPythonGraphic::Instance().SetDiffuseColor(1.0f, 1.0f, 1.0f, 0.5f);
	CPythonGraphic::Instance().RenderBar2d(m_rect.left + pSlot->ixPosition,
		m_rect.top + pSlot->iyPosition,
		m_rect.left + pSlot->ixPosition + pSlot->ixCellSize,
		m_rect.top + pSlot->iyPosition + pSlot->iyCellSize);
}

void CSlotWindow::OnRenderSelectedSlot()
{
	std::list<DWORD>::iterator itor = m_dwSelectedSlotIndexList.begin();
	for (; itor != m_dwSelectedSlotIndexList.end(); ++itor)
	{
		TSlot* pSlot;
		if (!GetSlotPointer(*itor, &pSlot))
			continue;

		CPythonGraphic::Instance().SetDiffuseColor(1.0f, 1.0f, 1.0f, 0.5f);
		CPythonGraphic::Instance().RenderBar2d(m_rect.left + pSlot->ixPosition,
			m_rect.top + pSlot->iyPosition,
			m_rect.left + pSlot->ixPosition + pSlot->ixCellSize,
			m_rect.top + pSlot->iyPosition + pSlot->iyCellSize);
	}
}

void CSlotWindow::RenderLockedSlot()
{
	CPythonGraphic::Instance().SetDiffuseColor(0.0f, 0.0f, 0.0f, 0.5f);
	for (TSlotListIterator itor = m_SlotList.begin(); itor != m_SlotList.end(); ++itor)
	{
		TSlot& rSlot = *itor;

		if (!rSlot.isItem)
			continue;

		if (rSlot.dwState & SLOT_STATE_LOCK)
		{
			CPythonGraphic::Instance().RenderBar2d(m_rect.left + rSlot.ixPosition,
				m_rect.top + rSlot.iyPosition,
				m_rect.left + rSlot.ixPosition + rSlot.ixCellSize,
				m_rect.top + rSlot.iyPosition + rSlot.iyCellSize);
		}
	}
}

// Check Slot

BOOL CSlotWindow::GetSlotPointer(DWORD dwIndex, TSlot** ppSlot)
{
	for (TSlotListIterator itor = m_SlotList.begin(); itor != m_SlotList.end(); ++itor)
	{
		TSlot& rSlot = *itor;

		if (dwIndex == rSlot.dwSlotNumber)
		{
			*ppSlot = &rSlot;
			return TRUE;
		}
	}

	return FALSE;
}

BOOL CSlotWindow::GetSelectedSlotPointer(TSlot** ppSlot)
{
	long lx, ly;
	GetMouseLocalPosition(lx, ly);

	for (TSlotListIterator itor = m_SlotList.begin(); itor != m_SlotList.end(); ++itor)
	{
		TSlot& rSlot = *itor;

		if (lx >= rSlot.ixPosition)
			if (ly >= rSlot.iyPosition)
				if (lx <= rSlot.ixPosition + rSlot.ixCellSize)
					if (ly <= rSlot.iyPosition + rSlot.iyCellSize)
					{
						*ppSlot = &rSlot;
						return TRUE;
					}
	}

	return FALSE;
}

BOOL CSlotWindow::GetPickedSlotPointer(TSlot** ppSlot)
{
	long lx, ly;
	CWindowManager::Instance().GetMousePosition(lx, ly);

	int ixLocal = lx - m_rect.left;
	int iyLocal = ly - m_rect.top;

	for (TSlotListIterator itor = m_SlotList.begin(); itor != m_SlotList.end(); ++itor)
	{
		TSlot& rSlot = *itor;

		int ixCellSize = rSlot.ixCellSize;
		int iyCellSize = rSlot.iyCellSize;

		if (rSlot.isItem)
		{
			ixCellSize = max(rSlot.ixCellSize, int(rSlot.byxPlacedItemSize * ITEM_WIDTH));
			iyCellSize = max(rSlot.iyCellSize, int(rSlot.byyPlacedItemSize * ITEM_HEIGHT));
		}

		if (ixLocal >= rSlot.ixPosition)
			if (iyLocal >= rSlot.iyPosition)
				if (ixLocal <= rSlot.ixPosition + ixCellSize)
					if (iyLocal <= rSlot.iyPosition + iyCellSize)
					{
						*ppSlot = &rSlot;
						return TRUE;
					}
	}

	return FALSE;
}

void CSlotWindow::SetUseMode(BOOL bFlag)
{
	m_isUseMode = bFlag;
}

void CSlotWindow::SetUsableItem(BOOL bFlag)
{
	m_isUsableItem = bFlag;
}

void CSlotWindow::ReserveDestroyCoolTimeFinishEffect(DWORD dwSlotIndex)
{
	m_ReserveDestroyEffectDeque.push_back(dwSlotIndex);
}

DWORD CSlotWindow::Type()
{
	static int s_Type = GetCRC32("CSlotWindow", strlen("CSlotWindow"));
	return s_Type;
}

#ifdef ENABLE_ACCE_SYSTEM
void CSlotWindow::ActivateEffect(DWORD dwSlotIndex, BYTE byColorType)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwSlotIndex, &pSlot))
		return;

	pSlot->bActiveEffect = TRUE;
	pSlot->byToggleColorType = byColorType;

	if (!m_apSlotActiveEffectEx[0] || !m_apSlotActiveEffectEx[1] || !m_apSlotActiveEffectEx[2])
	{
		__CreateSlotEnableEffectEx();
	}
}

void CSlotWindow::DeactivateEffect(DWORD dwSlotIndex)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwSlotIndex, &pSlot))
		return;

	pSlot->bActiveEffect = FALSE;
}
#endif

BOOL CSlotWindow::OnIsType(DWORD dwType)
{
	if (CSlotWindow::Type() == dwType)
		return TRUE;

	return CWindow::OnIsType(dwType);
}

void CSlotWindow::__CreateToggleSlotImage()
{
	__DestroyToggleSlotImage();

	m_pToggleSlotImage = new CImageBox(NULL);
	m_pToggleSlotImage->LoadImage("d:/ymir work/ui/public/slot_toggle.sub");
	m_pToggleSlotImage->Show();
}

void CSlotWindow::__CreateSlotEnableEffect()
{
	__DestroySlotEnableEffect();
#ifdef ENABLE_HIGHLIGHT_ITEM
	/*for (int i = 0; i < 3; ++i)
	{
		CAniImageBox* pEff = new CAniImageBox(NULL);
		for (int j = 0; j <= 12; ++j)
		{
			char buf[64 + 1];
			sprintf_s(buf, "d:/ymir work/ui/public/slotactiveeffect/slot%d/%02d.sub", (i + 1), j);
			pEff->AppendImage(buf);
		}

		pEff->SetRenderingMode(CGraphicExpandedImageInstance::RENDERING_MODE_SCREEN);
		m_apSlotActiveEffect[i] = pEff;
	}*/
	for (int i = 0; i < 3; ++i)
	{
		CAniImageBox* pEff = new CAniImageBox(NULL);
		int forCount = 0;
		int startLine = 0;
		if (i == 0) {
			startLine = 1;
			forCount = 22;
		}
		else if (i == 1) {
			startLine = 23;
			forCount = 32;
		}
		else if (i == 2) {
			startLine = 55;
			forCount = 39;
		}
		for (int j = startLine; j <= startLine + forCount; ++j)
		{
			char buf[128 + 1];
			sprintf_s(buf, "d:/ymir work/ui/public/newitemeffect/newitemeffect_%06d.sub", j);
			pEff->AppendImage(buf);
		}

		pEff->SetRenderingMode(CGraphicExpandedImageInstance::RENDERING_MODE_NORMAL);
		m_apSlotActiveEffect[i] = pEff;
		m_apSlotActiveEffect[i]->SetDelay(2.86);
	}
#else
	m_pSlotActiveEffect = new CAniImageBox(NULL);
	m_pSlotActiveEffect->AppendImage("d:/ymir work/ui/public/slotactiveeffect/00.sub");
	m_pSlotActiveEffect->AppendImage("d:/ymir work/ui/public/slotactiveeffect/01.sub");
	m_pSlotActiveEffect->AppendImage("d:/ymir work/ui/public/slotactiveeffect/02.sub");
	m_pSlotActiveEffect->AppendImage("d:/ymir work/ui/public/slotactiveeffect/03.sub");
	m_pSlotActiveEffect->AppendImage("d:/ymir work/ui/public/slotactiveeffect/04.sub");
	m_pSlotActiveEffect->AppendImage("d:/ymir work/ui/public/slotactiveeffect/05.sub");
	m_pSlotActiveEffect->AppendImage("d:/ymir work/ui/public/slotactiveeffect/06.sub");
	m_pSlotActiveEffect->AppendImage("d:/ymir work/ui/public/slotactiveeffect/07.sub");
	m_pSlotActiveEffect->AppendImage("d:/ymir work/ui/public/slotactiveeffect/08.sub");
	m_pSlotActiveEffect->AppendImage("d:/ymir work/ui/public/slotactiveeffect/09.sub");
	m_pSlotActiveEffect->AppendImage("d:/ymir work/ui/public/slotactiveeffect/10.sub");
	m_pSlotActiveEffect->AppendImage("d:/ymir work/ui/public/slotactiveeffect/11.sub");
	m_pSlotActiveEffect->AppendImage("d:/ymir work/ui/public/slotactiveeffect/12.sub");
	m_pSlotActiveEffect->SetRenderingMode(CGraphicExpandedImageInstance::RENDERING_MODE_SCREEN);
	m_pSlotActiveEffect->Show();
#endif
}

#ifdef ENABLE_ACCE_SYSTEM
void CSlotWindow::__CreateSlotEnableEffectEx()
{
	__DestroySlotEnableEffectEx();
	for (int i = 0; i < 3; ++i)
	{
		CAniImageBox* pEff = new CAniImageBox(NULL);
		for (int j = 0; j <= 12; ++j)
		{
			char buf[64 + 1];
			sprintf_s(buf, "d:/ymir work/ui/public/slotactiveeffect/slot%d/%02d.sub", (i + 1), j);
			pEff->AppendImage(buf);
		}

		pEff->SetRenderingMode(CGraphicExpandedImageInstance::RENDERING_MODE_SCREEN);
		m_apSlotActiveEffectEx[i] = pEff;
	}
}
#endif

void CSlotWindow::__CreateFinishCoolTimeEffect(TSlot* pSlot)
{
	__DestroyFinishCoolTimeEffect(pSlot);

	CAniImageBox* pFinishCoolTimeEffect = new CCoolTimeFinishEffect(this, pSlot->dwSlotNumber);
	pFinishCoolTimeEffect->AppendImage("d:/ymir work/ui/public/slotfinishcooltimeeffect/00.sub");
	pFinishCoolTimeEffect->AppendImage("d:/ymir work/ui/public/slotfinishcooltimeeffect/01.sub");
	pFinishCoolTimeEffect->AppendImage("d:/ymir work/ui/public/slotfinishcooltimeeffect/02.sub");
	pFinishCoolTimeEffect->AppendImage("d:/ymir work/ui/public/slotfinishcooltimeeffect/03.sub");
	pFinishCoolTimeEffect->AppendImage("d:/ymir work/ui/public/slotfinishcooltimeeffect/04.sub");
	pFinishCoolTimeEffect->AppendImage("d:/ymir work/ui/public/slotfinishcooltimeeffect/05.sub");
	pFinishCoolTimeEffect->AppendImage("d:/ymir work/ui/public/slotfinishcooltimeeffect/06.sub");
	pFinishCoolTimeEffect->AppendImage("d:/ymir work/ui/public/slotfinishcooltimeeffect/07.sub");
	pFinishCoolTimeEffect->AppendImage("d:/ymir work/ui/public/slotfinishcooltimeeffect/08.sub");
	pFinishCoolTimeEffect->AppendImage("d:/ymir work/ui/public/slotfinishcooltimeeffect/09.sub");
	pFinishCoolTimeEffect->AppendImage("d:/ymir work/ui/public/slotfinishcooltimeeffect/10.sub");
	pFinishCoolTimeEffect->SetRenderingMode(CGraphicExpandedImageInstance::RENDERING_MODE_SCREEN);
	pFinishCoolTimeEffect->ResetFrame();
	pFinishCoolTimeEffect->SetDelay(2);
	pFinishCoolTimeEffect->Show();

	pSlot->pFinishCoolTimeEffect = pFinishCoolTimeEffect;
}

void CSlotWindow::__CreateBaseImage(const char* c_szFileName, float fr, float fg, float fb, float fa)
{
	__DestroyBaseImage();

	CGraphicImage* pImage = (CGraphicImage*)CResourceManager::Instance().GetResourcePointer(c_szFileName);
	m_pBaseImageInstance = CGraphicImageInstance::New();
	m_pBaseImageInstance->SetImagePointer(pImage);
	m_pBaseImageInstance->SetDiffuseColor(fr, fg, fb, fa);
}

void CSlotWindow::__CreateBaseImageScale(const char* c_szFileName, float fr, float fg, float fb, float fa, float sx, float sy)
{
	__DestroyBaseImage();

	CGraphicImage* pImage = (CGraphicImage*)CResourceManager::Instance().GetResourcePointer(c_szFileName);
	m_pBaseImageInstance = CGraphicImageInstance::New();
	m_pBaseImageInstance->SetImagePointer(pImage);
	m_pBaseImageInstance->SetDiffuseColor(fr, fg, fb, fa);
	m_pBaseImageInstance->SetScale(sx, sy);
}

void CSlotWindow::__DestroyToggleSlotImage()
{
	if (m_pToggleSlotImage)
	{
		delete m_pToggleSlotImage;
		m_pToggleSlotImage = NULL;
	}
}

void CSlotWindow::__DestroySlotEnableEffect()
{
#ifdef ENABLE_HIGHLIGHT_ITEM
	for (int i = 0; i < 3; ++i)
	{
		if (m_apSlotActiveEffect[i])
		{
			delete m_apSlotActiveEffect[i];
			m_apSlotActiveEffect[i] = NULL;
		}
	}
#else
	if (m_pSlotActiveEffect)
	{
		delete m_pSlotActiveEffect;
		m_pSlotActiveEffect = NULL;
	}
#endif
}

#ifdef ENABLE_ACCE_SYSTEM
void CSlotWindow::__DestroySlotEnableEffectEx()
{
	for (int i = 0; i < 3; ++i)
	{
		if (m_apSlotActiveEffectEx[i])
		{
			delete m_apSlotActiveEffectEx[i];
			m_apSlotActiveEffectEx[i] = NULL;
		}
	}
}
#endif

void CSlotWindow::__DestroyFinishCoolTimeEffect(TSlot* pSlot)
{
	if (pSlot->pFinishCoolTimeEffect)
	{
		delete pSlot->pFinishCoolTimeEffect;
		pSlot->pFinishCoolTimeEffect = NULL;
	}
}

void CSlotWindow::__DestroyBaseImage()
{
	if (m_pBaseImageInstance)
	{
		CGraphicImageInstance::Delete(m_pBaseImageInstance);
		m_pBaseImageInstance = NULL;
	}
}

void CSlotWindow::__Initialize()
{
	m_dwSlotType = 0;
	m_dwSlotStyle = SLOT_STYLE_PICK_UP;
	m_dwToolTipSlotNumber = SLOT_NUMBER_NONE;

	m_isUseMode = FALSE;
	m_isUsableItem = FALSE;

	m_pToggleSlotImage = NULL;
#ifdef ENABLE_HIGHLIGHT_ITEM
	for (int i = 0; i < 3; ++i)
	{
		m_apSlotActiveEffect[i] = NULL;
	}
#else
	m_pSlotActiveEffect = NULL;
#endif
#ifdef ENABLE_ACCE_SYSTEM
	for (int i = 0; i < 3; ++i)
	{
		m_apSlotActiveEffectEx[i] = NULL;
	}
#endif
	m_pBaseImageInstance = NULL;
	m_CoolDownStore.clear();
}

void CSlotWindow::Destroy()
{
	for (TSlotListIterator itor = m_SlotList.begin(); itor != m_SlotList.end(); ++itor)
	{
		TSlot& rSlot = *itor;

		ClearSlot(&rSlot);

		if (rSlot.pNumberLine)
		{
			delete rSlot.pNumberLine;
			rSlot.pNumberLine = NULL;
		}
#ifdef ENABLE_SIGN_PLUS_ITEMS
		if (rSlot.pPlusLine)
		{
			delete rSlot.pPlusLine;
			rSlot.pPlusLine = NULL;
		}
#endif
		if (rSlot.pCoverButton)
		{
			CWindowManager::Instance().DestroyWindow(rSlot.pCoverButton);
		}
#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
		if (rSlot.pEvolutionButton)
		{
			CWindowManager::Instance().DestroyWindow(rSlot.pEvolutionButton);
		}
#endif
		if (rSlot.pSlotButton)
		{
			CWindowManager::Instance().DestroyWindow(rSlot.pSlotButton);
		}
		if (rSlot.pSignImage)
		{
			CWindowManager::Instance().DestroyWindow(rSlot.pSignImage);
		}
		if (rSlot.pFinishCoolTimeEffect)
		{
			CWindowManager::Instance().DestroyWindow(rSlot.pFinishCoolTimeEffect);
		}
	}

	m_SlotList.clear();

	__DestroyToggleSlotImage();
	__DestroySlotEnableEffect();
#ifdef ENABLE_ACCE_SYSTEM
	__DestroySlotEnableEffectEx();
#endif
	__DestroyBaseImage();

	__Initialize();
}

CSlotWindow::CSlotWindow(PyObject* ppyObject) : CWindow(ppyObject)
{
	__Initialize();
}

CSlotWindow::~CSlotWindow()
{
	Destroy();
}