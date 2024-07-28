#pragma once

#include "Locale_inc.h"

unsigned	LocaleService_GetCodePage();
const char* LocaleService_GetName();
const char* LocaleService_GetLocaleName();
const char* LocaleService_GetLocalePath();

void		LocaleService_ForceSetLocale(const char* name, const char* localePath);
void		LocaleService_LoadConfig(const char* fileName);
bool		LocaleService_LoadGlobal(HINSTANCE hInstance);
unsigned	LocaleService_GetLastExp(int level);
int			LocaleService_GetSkillPower(unsigned level);
