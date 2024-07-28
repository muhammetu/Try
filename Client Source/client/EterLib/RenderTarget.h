#pragma once

#include <memory>
#include "GrpRenderTargetTexture.h"

class CInstanceBase;
class CGraphicImageInstance;

class CRenderTarget
{
	using TCharacterInstanceMap = std::map<DWORD, CInstanceBase*>;

	public:
		CRenderTarget(DWORD width, DWORD height);
		~CRenderTarget();

		void SetVisibility(bool isShow);
		void ScaleToFitInViewport(int width, int height) const;
		void SetMenu(bool f);
		void SetWiki(bool f);

		// @
		// instance functions
		void ChangeGuild(DWORD guildID);

		void SetArmor(DWORD armor);
		void SetWeapon(DWORD weapon);
		void SetHair(DWORD hair);
#ifdef ENABLE_ACCE_SYSTEM
		void SetAcce(DWORD acce);
		void SetShining(DWORD index, DWORD shining);
#endif
#ifdef ENABLE_AURA_COSTUME_SYSTEM
		void SetAura(DWORD aura);
#endif

		void SetLevel(DWORD level);
		void SetAlignment(DWORD alignment);
		void SetPKMode(BYTE pkMode);
		// @

		void SelectModel(DWORD index);
		void SetScale(float x, float y, float z);
		const D3DXVECTOR3& GetScale();

		void SetModelRotation(float f) { m_modelRotation = f; }

		bool CreateBackground(const char* imgPath, DWORD width, DWORD height);
		void CreateTextures() const;

		void ReleaseTextures() const;

		void SetRenderingRect(RECT* rect) const;

		void RenderTexture() const;
		void RenderModel() const;
		void RenderBackground() const;
		void UpdateModel();
		void DeformModel() const;
	
	private:
		std::unique_ptr<CInstanceBase> m_pModel; 
		std::unique_ptr<CGraphicImageInstance> m_background;
		std::unique_ptr<CGraphicRenderTargetTexture> m_renderTargetTexture;
		float m_modelRotation;
		bool m_visible;
		bool m_isMenu;
		bool m_isWiki;
	protected:
		mutable D3DXVECTOR3	m_vScale;
};
