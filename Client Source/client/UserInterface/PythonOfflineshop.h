#ifndef __INCLUDE_HEADER_PYTHON_OFFLINESHO__
#define __INCLUDE_HEADER_PYTHON_OFFLINESHO__

#ifdef ENABLE_OFFLINE_SHOP
#include "PythonBackground.h"

namespace offlineshop
{
	template <class T>
	void ZeroObject(T& obj)
	{
		memset(&obj, 0, sizeof(obj));
	}

	template <class T>
	void CopyObject(T& objDest, const T& objSrc)
	{
		memcpy(&objDest, &objSrc, sizeof(objDest));
	}

	template <class T>
	void CopyContainer(T& objDest, const T& objSrc)
	{
		objDest = objSrc;
	}

	template <class T>
	void DeletePointersContainer(T& obj)
	{
		typename T::iterator it = obj.begin();
		for (; it != obj.end(); it++)
			delete(*it);
	}

	template <class T, template <class> class S, typename S<T>::iterator >
	void ForEach(S<T>& container, std::function<void(T&)> func)
	{
		S<T>::iterator it = container.begin(), iter;

		while ((iter = it++) != container.end())
			func(*iter);
	}

	template <class T, class K, template <class, class> class S, typename S<K, T>::iterator >
	void ForEach(S<K, T>& container, std::function<void(T&)> func)
	{
		S<K, T>::iterator it = container.begin(), iter;

		while ((iter = it++) != container.end())
			func(iter->second);
	}
}

namespace offlineshop
{
	enum eConstOfflineshop {
		OFFLINESHOP_ITEM_PAGE_MAX = 1,
		OFFLINESHOP_MAX_ITEM_COUNT = 90,
		OFFLINESHOP_TAB_NAME_MAX = 64,
		OFFLINESHOP_SEARCH_CATEGORY_MAX = 15,
		OFFLINESHOP_SEARCH_RESULT_PER_PAGE_ITEM_COUNT = 10,
		OFFLINESHOP_SEARCH_TIME = 1,
	};
}

namespace offlineshop
{
	enum EOfflineShopMain
	{

	};

	enum EOfflineShopSearchMain
	{
		OFFLINESHOP_SEARCH_PER_PAGE_RESULT_MAX = 50,
	};

	enum EOfflineShopSearchCategories
	{
		OFFLINESHOP_CAT_ALL,
		OFFLINESHOP_CAT_PREMIUM,
		OFFLINESHOP_CAT_WEAPON,
		OFFLINESHOP_CAT_ARMOR,
		OFFLINESHOP_CAT_ACCESSORY,
		OFFLINESHOP_CAT_PENDANT,
		OFFLINESHOP_CAT_ACCE,
		OFFLINESHOP_CAT_AURA,
		OFFLINESHOP_CAT_COSTUME,
		OFFLINESHOP_CAT_EQUIPABLE,
		OFFLINESHOP_CAT_DSS,
		OFFLINESHOP_CAT_PET,
		OFFLINESHOP_CAT_BOOK,
		OFFLINESHOP_CAT_POTION,
		OFFLINESHOP_CAT_UPGRADE,
		OFFLINESHOP_CAT_MAX,
	};

	enum EOfflineShopSearchSubCategoriesPremium
	{
		OFFLINESHOP_SUB_CAT_PREMIUM_MAX,
	};

	enum EOfflineShopSearchSubCategoriesWeapon
	{
		OFFLINESHOP_SUB_CAT_WEAPON_SWORD,
		OFFLINESHOP_SUB_CAT_WEAPON_TWOHAND,
		OFFLINESHOP_SUB_CAT_WEAPON_BOW,
		OFFLINESHOP_SUB_CAT_WEAPON_DAGGER,
		OFFLINESHOP_SUB_CAT_WEAPON_BELL,
		OFFLINESHOP_SUB_CAT_WEAPON_FAN,
		OFFLINESHOP_SUB_CAT_WEAPON_CLAW,
		OFFLINESHOP_SUB_CAT_WEAPON_MAX,
	};
}

#ifdef ENABLE_OFFLINE_SHOP_CITIES
namespace offlineshop
{
	class ShopInstance
	{
	public:
		ShopInstance() {
			m_dwVID = 0;
			m_iType = 0;
			m_owner = 0;
			m_stSign.clear();
		}

		~ShopInstance() {
			m_dwVID = 0;
			m_iType = 0;
			m_owner = 0;
			m_stSign.clear();
		}

		void SetVID(DWORD dwVID) {
			m_dwVID = dwVID;
		}

		void SetShopType(int iType) {
			m_iType = iType;
		}

		void SetSign(const char* cpszSign) {
			m_stSign = cpszSign;
		}

		void SetOwner(DWORD owner) {
			m_owner = owner;
		}

		void SetNPCGrannyResource()
		{
			m_grNPCResource = CResourceManager::Instance().GetResourcePointer("d:/ymir work/npc2/privateshop/privateshop.gr2");
		}

		void Show(float x, float y, float z) {
			m_thingInstance.Clear();
			m_thingInstance.ReserveModelThing(1);
			m_thingInstance.ReserveModelInstance(1);
			m_thingInstance.RegisterModelThing(0, (CGraphicThing*)m_grNPCResource);
			m_thingInstance.SetModelInstance(0, 0, 0);
			m_thingInstance.SetPosition(x, -y, z);

			m_thingInstance.Show();
			m_thingInstance.Update();
			m_thingInstance.Transform();
			m_thingInstance.Deform();
		}

		DWORD GetVID() const {
			return m_dwVID;
		}

		std::string GetSign() const {
			return m_stSign;
		}

		int GetType() const {
			return m_iType;
		}

		DWORD GetOwner() const {
			return m_owner;
		}

		CGraphicThingInstance* GetThingInstancePtr() {
			return &m_thingInstance;
		}

		void Clear() {
			m_thingInstance.Clear();
			m_dwVID = 0;
			m_iType = 0;
			m_owner = 0;
			m_stSign.clear();
		}

		void Render()
		{
			m_thingInstance.Render();
		}

		void BlendRender()
		{
			m_thingInstance.BlendRender();
		}

		void Update()
		{
			m_thingInstance.Update();
		}

	private:
		CGraphicThingInstance	m_thingInstance;
		DWORD					m_dwVID;
		int						m_iType;
		std::string				m_stSign;
		DWORD					m_owner;
		CResource* m_grNPCResource;
	};
}
#endif

class CPythonOfflineshop : public CSingleton<CPythonOfflineshop>
{
public:
	typedef struct
	{
		BYTE	bMinutes;
		BYTE	bHour;

		BYTE	bDay;
		BYTE	bMonth;
		int		iYear;
	} TDatetime;

	static void GetNowAsDatetime(TDatetime& datetime)
	{
		SYSTEMTIME time;
		GetLocalTime(&time);

		datetime.bMinutes = (BYTE)time.wMinute;
		datetime.bHour = (BYTE)time.wHour;
		datetime.bDay = (BYTE)time.wDay;

		datetime.bMonth = (BYTE)time.wMonth;
		datetime.iYear = (int)time.wYear;
	}

	static int AllocPatternID()
	{
		static int id = 0;
		return id++;
	}

public:
	CPythonOfflineshop();
	~CPythonOfflineshop();

	//starts
	void		SetWindowObjectPointer(PyObject* poWindow);
	void		SetWindowObjectViewPointer(PyObject* poWindow);
	void		SetWindowObjectSearchPointer(PyObject* poWindow);
	void		SetWindowObjectBuilderPointer(PyObject* poWindow);
	PyObject* GetOfflineshopBoard();
	PyObject* GetOfflineShopViewBoard();
	PyObject* GetOfflineShopSearchBoard();
	PyObject* GetOfflineShopBuilderBoard();

	void	BuyFromSearch(DWORD dwOwnerID, DWORD dwItemID);

	void	OpenShop(const offlineshop::TShopInfo& shop, const std::vector<offlineshop::TItemInfo>& vec);
	void	OpenShopOwner(const offlineshop::TShopInfo& shop,
		const std::vector<offlineshop::TItemInfo>& vec
	);
	void	OpenShopOwnerNoShop();

	void	ShopClose();
	void	ShopFilterResult(const std::vector<offlineshop::TItemInfo>& vec);
	void	SafeboxRefresh(const offlineshop::TValutesInfo& valute, const std::vector<DWORD>& ids, const std::vector<offlineshop::TItemInfoEx>& item);

	//interfaces methods
	void	EnableGuiRefreshSymbol();

	void	ShopCreateSuccess();
	void	ShopPopupError(BYTE bWindow, BYTE bErrorHeader);

#ifdef ENABLE_OFFLINE_SHOP_CITIES
	void	InsertEntity(DWORD dwVID, int iType, const char* szName, long x, long y, long z, DWORD owner);
	void	RemoveEntity(DWORD dwVID);

	void	RenderEntities();
	void	UpdateEntities();

	bool	GetShowNameFlag();
	void	SetShowNameFlag(bool flag);

	void	DeleteEntities();
#endif

private:
	PyObject* m_poWindow;
	PyObject* m_poWindowView;
	PyObject* m_poWindowSearch;
	PyObject* m_poWindowBuilder;

#ifdef ENABLE_OFFLINE_SHOP_CITIES
	std::vector<offlineshop::ShopInstance*>
		m_vecShopInstance;
	bool			m_bIsShowName;
#endif
};

extern void initofflineshop();

#endif //ENABLE_OFFLINE_SHOP

#endif //__INCLUDE_HEADER_PYTHON_OFFLINESHO__