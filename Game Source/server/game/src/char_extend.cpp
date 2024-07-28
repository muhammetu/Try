#ifdef __WIN32__
#define NOMINMAX
#endif
#include "stdafx.h"

#include "../../common/VnumHelper.h"

#include "char.h"

#include "config.h"
#include "utils.h"
#include "crc32.h"
#include "char_manager.h"
#include "desc_client.h"
#include "desc_manager.h"
#include "buffer_manager.h"
#include "item_manager.h"
#include "motion.h"
#include "vector.h"
#include "packet.h"
#include "cmd.h"
#include "fishing.h"
#include "exchange.h"
#include "battle.h"
#include "affect.h"
#include "shop.h"
#include "shop_manager.h"
#include "safebox.h"
#include "regen.h"
#include "pvp.h"
#include "party.h"
#include "start_position.h"
#include "questmanager.h"
#include "log.h"
#include "p2p.h"
#include "guild.h"
#include "guild_manager.h"
#ifdef __SCP1453_EXTENSIONS__
#include "dungeon_new.h"
#else
#include "dungeon.h"
#endif
#include "messenger_manager.h"
#include "unique_item.h"
#include "priv_manager.h"
#include "war_map.h"
#include "target.h"
#include "mob_manager.h"
#include "mining.h"
#include "horsename_manager.h"
#include "gm.h"
#include "map_location.h"
#include "skill_power.h"
#include "buff_on_attributes.h"

#ifdef __PET_SYSTEM__
#include "PetSystem.h"
#endif
#include "DragonSoul.h"

#ifdef __GROWTH_PET_SYSTEM__
#include "New_PetSystem.h"
#endif

#ifdef __GROWTH_MOUNT_SYSTEM__
#include "New_MountSystem.h"
#endif

#ifdef __SAFEBOX_AUTO_SORT__
#include "../../libgame/include/grid.h"
#endif

#ifdef __SWITCHBOT__
#include "new_switchbot.h"
#endif

#ifdef __OFFLINE_SHOP__
#include "new_offlineshop.h"
#include "new_offlineshop_manager.h"
#endif

#ifdef __MOUNT_COSTUME_SYSTEM__
#include "MountSystem.h"
#endif


#ifdef __DUNGEON_LIMIT__
void CHARACTER::RefreshDungeonTimeIndex()
{
}
bool CHARACTER::UpdateDungeonTime()
{
	return true;
}

#endif // __DUNGEON_LIMIT__