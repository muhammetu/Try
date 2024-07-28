#pragma once

#include "AbstractSingleton.h"
#include "Locale_inc.h"

class IAbstractChat : public TAbstractSingleton<IAbstractChat>
{
public:
	IAbstractChat() {}
	virtual ~IAbstractChat() {}

#ifdef ENABLE_CHAT_STOP
	virtual void AppendChat(int iType, const char* c_szChat, DWORD appendTime = -1) = 0;
#else
	virtual void AppendChat(int iType, const char* c_szChat) = 0;
#endif
};