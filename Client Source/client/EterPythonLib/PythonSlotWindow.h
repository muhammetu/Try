#pragma once
#include "../UserInterface/Locale_inc.h"
#include "PythonWindow.h"

namespace UI
{
	enum
	{
		ITEM_WIDTH = 32,
		ITEM_HEIGHT = 32,

		SLOT_NUMBER_NONE = 0xffffffff,
	};

	enum ESlotStyle
	{
		SLOT_STYLE_NONE,
		SLOT_STYLE_PICK_UP,
		SLOT_STYLE_SELECT,
	};

	enum ESlotState
	{
		SLOT_STATE_LOCK = (1 << 0),
		SLOT_STATE_CANT_USE = (1 << 1),
		SLOT_STATE_DISABLE = (1 << 2),
		SLOT_STATE_ALWAYS_RENDER_COVER = (1 << 3),
		SLOT_STATE_HIGHLIGHT_GREEN = (1 << 4),
		SLOT_STATE_HIGHLIGHT_RED = (1 << 5),
#ifdef ENABLE_SLOT_MARKING_SYSTEM
		SLOT_STATE_CANT_MOUSE_EVENT = (1 << 6),
		SLOT_STATE_UNUSABLE = (1 << 7),
#endif
	};

	enum ESlotDiffuseColorType
	{
		SLOT_COLOR_TYPE_ORANGE = 0,
		SLOT_COLOR_TYPE_WHITE = 1,
		SLOT_COLOR_TYPE_RED = 2,
		SLOT_COLOR_TYPE_GREEN = 3,
		SLOT_COLOR_TYPE_YELLOW = 4,
		SLOT_COLOR_TYPE_SKY = 5,
		SLOT_COLOR_TYPE_PINK = 6,
	};

	enum ESlotDiffuseColorTypeEx
	{
		COLOR_TYPE_ORANGE,
		COLOR_TYPE_WHITE,
		COLOR_TYPE_RED,
		COLOR_TYPE_GREEN,
		COLOR_TYPE_YELLOW,
		COLOR_TYPE_SKY,
		COLOR_TYPE_PINK,
		COLOR_TYPE_DARK,
		COLOR_TYPE_MAX,
	};

	class CSlotWindow : public CWindow
	{
	public:
		static DWORD Type();

	public:
		class CSlotButton;
		class CCoverButton;
		class CCoolTimeFinishEffect;

		friend class CSlotButton;
		friend class CCoverButton;

		typedef struct SSlot
		{
			DWORD	dwState;
			DWORD	dwSlotNumber;
			DWORD	dwCenterSlotNumber;
			DWORD	dwItemIndex;
			BOOL	isItem;

			// CoolTime
			float	fCoolTime;
			float	fStartCoolTime;
			// Toggle
			BOOL	bActive;

			int		ixPosition;
			int		iyPosition;

			int		ixCellSize;
			int		iyCellSize;

			BYTE	byxPlacedItemSize;
			BYTE	byyPlacedItemSize;

			CGraphicExpandedImageInstance* pInstance;
			CNumberLine* pNumberLine;
#ifdef ENABLE_SIGN_PLUS_ITEMS
			CNumberLine* pPlusLine;
			int		ixPlusPosition;
			int		iyPlusPosition;
#endif
			bool	bRenderBaseSlotImage;
			CCoverButton* pCoverButton;
#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
			CCoverButton* pEvolutionButton;
#endif
			CSlotButton* pSlotButton;
			CImageBox* pSignImage;
			CAniImageBox* pFinishCoolTimeEffect;
#ifdef ENABLE_ACCE_SYSTEM
			BOOL	bActiveEffect;
			BYTE	byToggleColorType;
#endif
		} TSlot;
		typedef std::list<TSlot> TSlotList;
		typedef TSlotList::iterator TSlotListIterator;
		typedef struct { float fCoolTime; float fElapsedTime; bool bActive; } SStoreCoolDown;

	public:
		CSlotWindow(PyObject* ppyObject);
		virtual ~CSlotWindow();

		void Destroy();

		// Manage Slot
		void SetSlotType(DWORD dwType);
		void SetSlotStyle(DWORD dwStyle);

		void AppendSlot(DWORD dwIndex, int ixPosition, int iyPosition, int ixCellSize, int iyCellSize);
		void GetSlotPosition(DWORD dwIndex, int& ixPosition, int& iyPosition);
		void SetCoverButton(DWORD dwIndex, const char* c_szUpImageName, const char* c_szOverImageName, const char* c_szDownImageName, const char* c_szDisableImageName, BOOL bLeftButtonEnable, BOOL bRightButtonEnable);
#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
		void SetEvolutionButton(DWORD dwIndex, const char* c_szUpImageName, const char* c_szOverImageName, const char* c_szDownImageName, const char* c_szDisableImageName, BOOL bLeftButtonEnable, BOOL bRightButtonEnable);
#endif
		void DeleteCoverButton(DWORD dwIndex);
		void SetSlotBaseImage(const char* c_szFileName, float fr, float fg, float fb, float fa);
		void SetSlotBaseImageScale(const char* c_szFileName, float fr, float fg, float fb, float fa, float sx, float sy);
		void SetSlotScale(DWORD dwIndex, DWORD dwVirtualNumber, BYTE byWidth, BYTE byHeight, CGraphicImage* pImage, float sx, float sy, D3DXCOLOR& diffuseColor);
		void AppendSlotButton(const char* c_szUpImageName, const char* c_szOverImageName, const char* c_szDownImageName);
		void AppendRequirementSignImage(const char* c_szImageName);

		void EnableCoverButton(DWORD dwIndex);
		void DisableCoverButton(DWORD dwIndex);
#ifdef ENABLE_ITEM_EVOLUTION_SYSTEM
		void EnableEvolutionButton(DWORD dwIndex);
		void DisableEvolutionButton(DWORD dwIndex);
#endif
		void SetAlwaysRenderCoverButton(DWORD dwIndex, bool bAlwaysRender = false);

		void ShowSlotBaseImage(DWORD dwIndex);
		void HideSlotBaseImage(DWORD dwIndex);
		BOOL IsDisableCoverButton(DWORD dwIndex);
		BOOL HasSlot(DWORD dwIndex);

		void ClearAllSlot();
		void ClearSlot(DWORD dwIndex);
		void SetSlot(DWORD dwIndex, DWORD dwVirtualNumber, BYTE byWidth, BYTE byHeight, CGraphicImage* pImage, D3DXCOLOR& diffuseColor);
		void SetCardSlot(DWORD dwIndex, DWORD dwVirtualNumber, BYTE byWidth, BYTE byHeight, const char* c_szFileName, D3DXCOLOR& diffuseColor);
		void SetSlotCount(DWORD dwIndex, DWORD dwCount);
		void SetSlotCountNew(DWORD dwIndex, DWORD dwGrade, DWORD dwCount);
#ifdef ENABLE_SIGN_PLUS_ITEMS
		void AppendPlusOnSlot(DWORD dwIndex, int ixPlusPosition, int iyPlusPosition);
#endif
		void SetSlotCoolTime(DWORD dwIndex, float fCoolTime, float fElapsedTime = 0.0f);
		void StoreSlotCoolTime(DWORD dwKey, DWORD dwSlotIndex, float fCoolTime, float fElapsedTime = .0f);
		void RestoreSlotCoolTime(DWORD dwKey);
		void ActivateSlot(DWORD dwIndex);
		void DeactivateSlot(DWORD dwIndex);
		bool IsActivatedSlot(DWORD dwIndex);
		bool IsActiveEffect(DWORD dwIndex);
		void RefreshSlot();

		DWORD GetSlotCount();
		DWORD GetSlotItemVnum(DWORD dwIndex);

		void LockSlot(DWORD dwIndex);
		void UnlockSlot(DWORD dwIndex);
		void SetCantUseSlot(DWORD dwIndex);
		void SetUseSlot(DWORD dwIndex);
		void EnableSlot(DWORD dwIndex);
		void DisableSlot(DWORD dwIndex);

		// Select
		void ClearSelected();
		void SelectSlot(DWORD dwSelectingIndex);
		BOOL isSelectedSlot(DWORD dwIndex);
		DWORD GetSelectedSlotCount();
		DWORD GetSelectedSlotNumber(DWORD dwIndex);

		// Slot Button
		void ShowSlotButton(DWORD dwSlotNumber);
		void HideAllSlotButton();
		void OnPressedSlotButton(DWORD dwType, DWORD dwSlotNumber, BOOL isLeft = TRUE);

		// Requirement Sign
		void ShowRequirementSign(DWORD dwSlotNumber);
		void HideRequirementSign(DWORD dwSlotNumber);

		// ToolTip
		BOOL OnOverInItem(DWORD dwSlotNumber);
		void OnOverOutItem();

		// For Usable Item
		void SetUseMode(BOOL bFlag);
		void SetUsableItem(BOOL bFlag);

		// CallBack
		void ReserveDestroyCoolTimeFinishEffect(DWORD dwSlotIndex);

#ifdef ENABLE_ACCE_SYSTEM
		void ActivateEffect(DWORD dwSlotIndex, BYTE byColorType);
		void DeactivateEffect(DWORD dwSlotIndex);
#endif
#ifdef ENABLE_SLOT_MARKING_SYSTEM
		void SetSlotHighlightedGreeen(DWORD dwIndex, DWORD dwColor = 0);
		void DisableSlotHighlightedGreen(DWORD dwIndex, DWORD dwColor = 0);
		void SetCanMouseEventSlot(DWORD dwIndex);
		void SetCantMouseEventSlot(DWORD dwIndex);
		void SetUsableSlotOnTopWnd(DWORD dwIndex);
		void SetUnusableSlotOnTopWnd(DWORD dwIndex);
#endif
	protected:
		void __Initialize();
		void __CreateToggleSlotImage();
		void __CreateSlotEnableEffect();
#ifdef ENABLE_ACCE_SYSTEM
		void __CreateSlotEnableEffectEx();
#endif
		void __CreateFinishCoolTimeEffect(TSlot* pSlot);
		void __CreateBaseImage(const char* c_szFileName, float fr, float fg, float fb, float fa);
		void __CreateBaseImageScale(const char* c_szFileName, float fr, float fg, float fb, float fa, float sx, float sy);
		void __DestroyToggleSlotImage();
		void __DestroySlotEnableEffect();
#ifdef ENABLE_ACCE_SYSTEM
		void __DestroySlotEnableEffectEx();
#endif
		void __DestroyFinishCoolTimeEffect(TSlot* pSlot);
		void __DestroyBaseImage();

		// Event
		void OnUpdate();
		void OnRender();
		BOOL OnMouseLeftButtonDown();
		BOOL OnMouseLeftButtonUp();
		BOOL OnMouseRightButtonDown();
		BOOL OnMouseLeftButtonDoubleClick();
		void OnMouseOverOut();
		void OnMouseOver();
		void RenderSlotBaseImage();
		void RenderLockedSlot();
		virtual void OnRenderPickingSlot();
		virtual void OnRenderSelectedSlot();

		// Select
		void OnSelectEmptySlot(int iSlotNumber);
		void OnSelectItemSlot(int iSlotNumber);
		void OnUnselectEmptySlot(int iSlotNumber);
		void OnUnselectItemSlot(int iSlotNumber);
		void OnUseSlot();

		// Manage Slot
	public:
		BOOL GetSlotPointer(DWORD dwIndex, TSlot** ppSlot);
	protected:
		BOOL GetSelectedSlotPointer(TSlot** ppSlot);
		virtual BOOL GetPickedSlotPointer(TSlot** ppSlot);
		void ClearSlot(TSlot* pSlot);
		virtual void OnRefreshSlot();

		// ETC
		BOOL OnIsType(DWORD dwType);

	protected:
		DWORD m_dwSlotType;
		DWORD m_dwSlotStyle;
		std::list<DWORD> m_dwSelectedSlotIndexList;
		TSlotList m_SlotList;
		DWORD m_dwToolTipSlotNumber;

		BOOL m_isUseMode;
		BOOL m_isUsableItem;

		CGraphicImageInstance* m_pBaseImageInstance;
		CImageBox* m_pToggleSlotImage;
#ifdef ENABLE_HIGHLIGHT_ITEM
		CAniImageBox* m_apSlotActiveEffect[3];
#else
		CAniImageBox* m_pSlotActiveEffect;
#endif
#ifdef ENABLE_ACCE_SYSTEM
		CAniImageBox* m_apSlotActiveEffectEx[3];
#endif
		std::deque<DWORD> m_ReserveDestroyEffectDeque;
		std::map<DWORD, std::map<DWORD, SStoreCoolDown>>	m_CoolDownStore;
	};
};