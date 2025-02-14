#pragma once

#include "PythonBackground.h"

class CPythonMiniMap : public CScreen, public CSingleton<CPythonMiniMap>
{
	public:
		enum
		{
			EMPIRE_NUM = 4,

			MINI_WAYPOINT_IMAGE_COUNT = 12,
			WAYPOINT_IMAGE_COUNT = 15,
			TARGET_MARK_IMAGE_COUNT = 2,
		};
		enum
		{
			TYPE_OPC,
			TYPE_OPCPVP,
			TYPE_OPCPVPSELF,
			TYPE_NPC,
			TYPE_MONSTER,
			TYPE_WARP,
			TYPE_WAYPOINT,
			TYPE_PARTY,
			TYPE_EMPIRE,
			TYPE_EMPIRE_END = TYPE_EMPIRE + EMPIRE_NUM,
			TYPE_TARGET,
#ifdef ENABLE_METINSTONE_ON_MINIMAP
			TYPE_STONE,
#endif
#ifdef ENABLE_BOSS_ON_MINIMAP
			TYPE_BOSS,
#endif
			TYPE_COUNT,
		};

#ifdef ENABLE_ATLAS_RENDER_OPTION
		enum EMiniMapMarker
		{
			MARKER_NPC,
			MARKER_WARP,
			MARKER_WAYPOINT,
			MARKER_STONE,
			MARKER_BOSS,
			MARKER_MAX,
		};
#endif

	public:
		CPythonMiniMap();
		virtual ~CPythonMiniMap();

		void Destroy();
		bool Create();

		bool IsAtlas();
		bool CanShow();
		bool CanShowAtlas();

		void SetMiniMapSize(float fWidth, float fHeight);
		void SetScale(float fScale);
		void ScaleUp();
		void ScaleDown();

		void SetCenterPosition(float fCenterX, float fCenterY);

		void Update(float fCenterX, float fCenterY);
		void Render(float fScreenX, float fScreenY);

		void Show();
		void Hide();

		bool GetPickedInstanceInfo(float fScreenX, float fScreenY, std::string & rReturnName, float * pReturnPosX, float * pReturnPosY, DWORD * pdwTextColor);

		// Atlas
		bool LoadAtlas();
		void UpdateAtlas();
		void RenderAtlas(float fScreenX, float fScreenY);
		void ShowAtlas();
		void HideAtlas();
#ifdef ENABLE_ATLAS_RENDER_OPTION
		bool ToggleAtlasMarker(int type);
		bool IsCheckedAtlasMarker(int type);
#endif

		bool GetAtlasInfo(float fScreenX, float fScreenY, std::string & rReturnString, float * pReturnPosX, float * pReturnPosY, DWORD * pdwTextColor, DWORD * pdwGuildID);
		bool GetAtlasSize(float * pfSizeX, float * pfSizeY);

		void AddObserver(DWORD dwVID, float fSrcX, float fSrcY);
		void MoveObserver(DWORD dwVID, float fDstX, float fDstY);
		void RemoveObserver(DWORD dwVID);

		// WayPoint
		void AddWayPoint(BYTE byType, DWORD dwID, float fX, float fY, std::string strText, DWORD dwChrVID=0);
		void RemoveWayPoint(DWORD dwID);

		// SignalPoint
		void AddSignalPoint(float fX, float fY);
		void ClearAllSignalPoint();

		void RegisterAtlasWindow(PyObject* poHandler);
		void UnregisterAtlasWindow();
		void OpenAtlasWindow();
		void SetAtlasCenterPosition(int x, int y);

		// NPC List
		void ClearAtlasMarkInfo();
		void RegisterAtlasMark(BYTE byType, const char * c_szName, long lx, long ly);

		// Target
		void CreateTarget(int iID, const char * c_szName);
		void CreateTarget(int iID, const char * c_szName, DWORD dwVID);
		void UpdateTarget(int iID, int ix, int iy);
		void DeleteTarget(int iID);

	protected:
		void __Initialize();
		void __SetPosition();
		void __LoadAtlasMarkInfo(const char* fileName);

		void __RenderWayPointMark(int ixCenter, int iyCenter);
		void __RenderMiniWayPointMark(int ixCenter, int iyCenter);
		void __RenderTargetMark(int ixCenter, int iyCenter);

		void __GlobalPositionToAtlasPosition(long lx, long ly, float * pfx, float * pfy);

	protected:
		// Atlas
		typedef struct
		{
			BYTE m_byType;
			DWORD m_dwID; // For WayPoint
			float m_fX;
			float m_fY;
			float m_fScreenX;
			float m_fScreenY;
			float m_fMiniMapX;
			float m_fMiniMapY;
			DWORD m_dwChrVID;
			std::string m_strText;
		} TAtlasMarkInfo;

		struct SObserver
		{
			float fCurX;
			float fCurY;
			float fSrcX;
			float fSrcY;
			float fDstX;
			float fDstY;

			DWORD dwSrcTime;
			DWORD dwDstTime;
		};

		typedef struct
		{
			UINT	m_bType;
			float	m_fX;
			float	m_fY;
			UINT	m_eNameColor;
		} TMarkPosition;

		typedef std::vector<TMarkPosition>				TInstanceMarkPositionVector;
		typedef TInstanceMarkPositionVector::iterator	TInstancePositionVectorIterator;

	protected:
		bool __GetWayPoint(DWORD dwID, TAtlasMarkInfo ** ppkInfo);
		void __UpdateWayPoint(TAtlasMarkInfo * pkInfo, int ix, int iy);

	protected:
		float							m_fWidth;
		float							m_fHeight;

		float							m_fScale;

		float							m_fCenterX;
		float							m_fCenterY;

		float							m_fCenterCellX;
		float							m_fCenterCellY;

		float							m_fScreenX;
		float							m_fScreenY;

		float							m_fMiniMapRadius;

		LPDIRECT3DTEXTURE8				m_lpMiniMapTexture[AROUND_AREA_NUM];

		CGraphicImageInstance			m_MiniMapFilterGraphicImageInstance;
		CGraphicExpandedImageInstance	m_MiniMapCameraraphicImageInstance;

		CGraphicExpandedImageInstance	m_PlayerMark;
		CGraphicImageInstance			m_WhiteMark;

		std::vector<TMarkPosition>		m_MinimapPosVector;
		std::map<DWORD, SObserver>		m_kMap_dwVID_kObserver;

		bool							m_bAtlas;
		bool							m_bShow;

		CGraphicVertexBuffer			m_VertexBuffer;
		CGraphicIndexBuffer				m_IndexBuffer;

		D3DXMATRIX						m_matIdentity;
		D3DXMATRIX						m_matWorld;
		D3DXMATRIX						m_matMiniMapCover;

		bool							m_bShowAtlas;
		bool							m_bAtlasRenderNpc;
		bool							m_bAtlasRenderWarp;
		bool							m_bAtlasRenderWaypoint;
		bool							m_bAtlasRenderStone;
		bool							m_bAtlasRenderBoss;

		CGraphicImageInstance			m_AtlasImageInstance;
		D3DXMATRIX						m_matWorldAtlas;
		CGraphicExpandedImageInstance	m_AtlasPlayerMark;

		float							m_fAtlasScreenX;
		float							m_fAtlasScreenY;

		DWORD							m_dwAtlasBaseX;
		DWORD							m_dwAtlasBaseY;

		float							m_fAtlasMaxX;
		float							m_fAtlasMaxY;

		float							m_fAtlasImageSizeX;
		float							m_fAtlasImageSizeY;

		typedef std::vector<TAtlasMarkInfo>		TAtlasMarkInfoVector;
		typedef TAtlasMarkInfoVector::iterator	TAtlasMarkInfoVectorIterator;
		TAtlasMarkInfoVectorIterator			m_AtlasMarkInfoListIterator;
		TAtlasMarkInfoVector					m_AtlasNPCInfoVector;
		TAtlasMarkInfoVector					m_AtlasStoneInfoVector;
		TAtlasMarkInfoVector					m_AtlasBossInfoVector;
		TAtlasMarkInfoVector					m_AtlasWarpInfoVector;

		// WayPoint
		CGraphicExpandedImageInstance			m_MiniWayPointGraphicImageInstances[MINI_WAYPOINT_IMAGE_COUNT];
		CGraphicExpandedImageInstance			m_WayPointGraphicImageInstances[WAYPOINT_IMAGE_COUNT];
		CGraphicExpandedImageInstance			m_TargetMarkGraphicImageInstances[TARGET_MARK_IMAGE_COUNT];
		TAtlasMarkInfoVector					m_AtlasWayPointInfoVector;

		// SignalPoint
		struct TSignalPoint
		{
			D3DXVECTOR2 v2Pos;
			unsigned int id;
		};
		vector<TSignalPoint>				m_SignalPointVector;

		PyObject*							m_poHandler;
};