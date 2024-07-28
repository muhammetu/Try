#pragma once
#include <unordered_map>
#include <memory>

#include "RenderTarget.h"

#include "../EterBase/Singleton.h"
#include "../EterPythonLib/StdAfx.h"

class CRenderTargetManager : public CSingleton<CRenderTargetManager>
{
	public:
		CRenderTargetManager();
		virtual ~CRenderTargetManager();

		std::shared_ptr<CRenderTarget> GetRenderTarget(const PyObject* window);
		bool CreateRenderTarget(const PyObject* window, int width, int height);
		bool Remove(const PyObject* window);

		void CreateRenderTargetTextures();
		void ReleaseRenderTargetTextures();

		void Destroy();
		void DeformModels();
		void UpdateModels();
		void RenderBackgrounds();
		void RenderModels();
	protected:
		std::unordered_map<const PyObject*, std::shared_ptr<CRenderTarget>> m_renderTargets;
};
