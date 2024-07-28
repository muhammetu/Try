#pragma once
#include "Locale_inc.h"

#ifdef ENABLE_MODEL_RENDER_TARGET
#include "../eterLib/RenderTarget.h"

#include "InstanceBase.h"
class CPythonRenderTarget : public CSingleton<CPythonRenderTarget>
{
	public:
		CPythonRenderTarget();
		virtual ~CPythonRenderTarget();
	protected:
};
#endif