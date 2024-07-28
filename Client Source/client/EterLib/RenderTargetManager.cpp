#include "StdAfx.h"
#ifdef ENABLE_MODEL_RENDER_TARGET
#include "RenderTargetManager.h"

#include "../EterBase/Stl.h"
#include <memory>


CRenderTargetManager::CRenderTargetManager()
{
}

CRenderTargetManager::~CRenderTargetManager()
{
	Destroy();
}

void CRenderTargetManager::Destroy()
{
	m_renderTargets.clear();
}

void CRenderTargetManager::DeformModels()
{
	for (const auto& elem : m_renderTargets)
		elem.second->DeformModel();
}

void CRenderTargetManager::UpdateModels()
{
	for (auto& elem : m_renderTargets)
		elem.second->UpdateModel();
}

void CRenderTargetManager::RenderBackgrounds()
{
	for (const auto& elem : m_renderTargets)
		elem.second->RenderBackground();
}

void CRenderTargetManager::RenderModels()
{
	for (const auto& elem : m_renderTargets)
		elem.second->RenderModel();
}

void CRenderTargetManager::CreateRenderTargetTextures()
{
	for (const auto& elem : m_renderTargets)
		elem.second->CreateTextures();
}

void CRenderTargetManager::ReleaseRenderTargetTextures()
{
	for (const auto& elem : m_renderTargets)
		elem.second->ReleaseTextures();
}

bool CRenderTargetManager::CreateRenderTarget(const PyObject* window, const int width, const int height)
{
	if (GetRenderTarget(window))
	{
		Tracen("Render Target already registered!");
		return false;
	}

	m_renderTargets.emplace(window, std::make_shared<CRenderTarget>(width, height));
	Tracenf("CRenderTargetManager LOG m_renderTargets.size %d               ", m_renderTargets.size());
	return true;
}

std::shared_ptr<CRenderTarget> CRenderTargetManager::GetRenderTarget(const PyObject* window)
{
	const auto it = m_renderTargets.find(window);
	if (it != m_renderTargets.end())
		return it->second;

	return NULL;
}

bool CRenderTargetManager::Remove(const PyObject* window)
{
	return m_renderTargets.erase(window) != 0;
}
#endif