#pragma once
#include "../UserInterface/Locale_inc.h"

#ifdef ENABLE_GRAPHIC_OPTIMIZATION
class EffectHelper : public CSingleton<EffectHelper>
{
	public:
		EffectHelper();
		virtual ~EffectHelper();
	public:
		// TODO:
		// @@ classlar arasi cakisma yarattigi icin global bir alana tasinacak bu arkadaslar
		enum EEftType
		{
			TYPE_ENEMY,
			TYPE_NPC,
			TYPE_STONE,
			TYPE_WARP,
			TYPE_DOOR,
			TYPE_PC,
			TYPE_POLY,
			TYPE_HORSE,
			TYPE_GOTO,
			TYPE_BOSS,
			TYPE_PET,
			TYPE_MOUNT,
			TYPE_SHOP,
			TYPE_OBJECT, // Only For Client
		};
		bool GetEffectRenderStatus(bool isMain, BYTE actorType);
};
#endif