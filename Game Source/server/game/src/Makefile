CXX = g++9

GAME_VERSION := $(shell cat ../../__REVISION__)

GccMajorVersion := $(shell expr `$(CXX) -dumpversion | cut -f1 -d.`)
GccMinorVersion := $(shell expr `$(CXX) -dumpversion | cut -f2 -d.`)
GccMinorEGT8 := $(shell expr $(GccMinorVersion) \>= 8)

INCDIR =
LIBDIR =
BINDIR = ..
OBJDIR = .obj
SRVDIR = /home/main/srv1
$(shell if [ ! -d $(OBJDIR) ]; then mkdir $(OBJDIR); fi)

## LIST OF CONSTANTS BEGIN
ENABLE_LUA_5_VERSION = 0
ENABLE_GCC_AUTODEPEND = 1
ENABLE_STATIC = 0
__EVENT_SYSTEM__ = 1
__AUTO_EVENT_SYSTEM__ = 1
__BAN_REASON_UTILITY__ = 1
__CUBE_WINDOW__ = 1
__HWID_SECURITY_UTILITY__ = 1
__INGAME_MALL__ = 1
__OFFLINE_SHOP__ = 1
__GROWTH_PET_SYSTEM__ = 1
__GROWTH_MOUNT_SYSTEM__ = 1
__SWITCHBOT__ = 1
__DUNGEON_INFORMATION__ = 1
__ENABLE_WIKI_SYSTEM__ = 1
__GAME_MASTER_UTILITY__ = 1
__MOUNT_COSTUME_SYSTEM__ = 1
__AS_BOT__ = 1
__SCP1453_EXTENSIONS__ = 1
__BATTLE_PASS_SYSTEM__ = 1
## LIST OF CONSTANTS END

# Depend Path File
ifneq ($(ENABLE_GCC_AUTODEPEND), 1)
DEPFILE = Depend
endif

# Standard Libraries
LIBS = -lm -lmd

# Project Flags
CFLAGS  = -m32 -g -Wall -O2 -pipe -fexceptions -fno-strict-aliasing -pthread -D_THREAD_SAFE -DNDEBUG -march=ivybridge
CFLAGS += -Wno-deprecated-declarations -Wno-nonnull-compare -Wno-deprecated-declarations -Wno-array-bounds -Wno-address
CFLAGS += -Wno-int-in-bool-context -Wno-format-truncation -Wno-stringop-truncation -Wno-sign-compare
CXXFLAGS = -std=c++11 -Wl,-rpath=/usr/local/lib/gcc9

ifeq ($(ENABLE_STATIC), 1)
CFLAGS += -static
endif

ifeq ($(GccMinorEGT8), 1)
CFLAGS += -Wno-unused-local-typedefs
endif

# FreeBSD stack protector
CFLAGS += -fstack-protector-all

# Version defines
CFLAGS += -D__USER__=\"$(USER)\" -D__HOSTNAME__=\"$(HOSTNAME)\" -D__PWD__=\"$(PWD)\" -D__GAME_VERSION__=\"$(GAME_VERSION)\"

# Boost
INCDIR += -I../../../extern/include/boost

# DevIL
INCDIR += -I../../../extern/include/IL
LIBS += ../../../extern/lib/libIL.a\
		../../../extern/lib/libjasper.a\
		../../../extern/lib/libpng.a\
		../../../extern/lib/libtiff.a\
		../../../extern/lib/libjbig.a\
		../../../extern/lib/libmng.a\
		/usr/lib/liblzma.a\
		../../../extern/lib/liblcms.a\
		../../../extern/lib/libjpeg.a

# MySQL
INCDIR += -I/usr/local/include/mysql
LIBS += /usr/local/lib/mysql/libmariadbclient.a /usr/lib/libz.a

# CryptoPP
LIBS += ../../../extern/lib/libcryptopp.a

# OpenSSL
INCDIR += -I/usr/include
LIBS += -lssl -lcrypto
# LIBS += /usr/lib/libssl.a

# Lua
CFLAGS += -DENABLE_LUA_5_VERSION=ENABLE_LUA_5_VERSION
ifeq ($(ENABLE_LUA_5_VERSION), 2)
INCDIR += -I../../liblua/5.2/install/include
LIBDIR += -L../../liblua/5.2/install/lib
LIBS += ../../liblua/5.2/install/lib/liblua.a
else
INCDIR += -I../../liblua/5.0/include
LIBDIR += -L../../liblua/5.0/lib
LIBS += ../../liblua/5.0/lib/liblua.a ../../liblua/5.0/lib/liblualib.a
endif

# Project Libraries
INCDIR += -I../../../extern/include
INCDIR += -I/usr/local/include
LIBDIR += -L/usr/local/lib

LIBDIR += -L../../libthecore/lib -L../../libpoly -L../../libsql -L../../libgame/lib
LIBS += -lthecore -lpoly -lsql -lgame

# PROJECT_SRC_FILES BEGIN
MAINCPP = main.cpp

CFILE	= minilzo.c

CPPFILE = FSM.cpp char_extend.cpp MarkConvert.cpp MarkImage.cpp MarkManager.cpp\
		  battle.cpp blend_item.cpp buffer_manager.cpp\
		  char.cpp char_affect.cpp char_battle.cpp char_change_empire.cpp char_horse.cpp char_item.cpp char_manager.cpp\
		  char_quickslot.cpp char_resist.cpp char_skill.cpp char_state.cpp PetSystem.cpp cmd.cpp cmd_emotion.cpp cmd_general.cpp\
		  cmd_gm.cpp config.cpp constants.cpp crc32.cpp db.cpp desc.cpp\
		  desc_client.cpp desc_manager.cpp desc_p2p.cpp entity.cpp\
		  entity_view.cpp event.cpp event_queue.cpp exchange.cpp file_loader.cpp fishing.cpp gm.cpp guild.cpp\
		  guild_manager.cpp guild_war.cpp horse_rider.cpp horsename_manager.cpp input.cpp input_auth.cpp input_db.cpp\
		  input_login.cpp input_main.cpp input_p2p.cpp\
		  item.cpp item_addon.cpp item_attribute.cpp item_manager.cpp item_manager_idrange.cpp locale.cpp\
		  locale_service.cpp log.cpp login_data.cpp lzo_manager.cpp\
		  messenger_manager.cpp mining.cpp mob_manager.cpp motion.cpp p2p.cpp packet_info.cpp\
		  party.cpp priv_manager.cpp pvp.cpp\
		  questevent.cpp questlua.cpp questlua_affect.cpp\
		  questlua_game.cpp questlua_global.cpp\
		  questlua_guild.cpp questlua_horse.cpp questlua_pet.cpp questlua_item.cpp\
		  questlua_npc.cpp questlua_party.cpp questlua_pc.cpp\
		  questlua_quest.cpp questlua_target.cpp questmanager.cpp questnpc.cpp questpc.cpp\
		  refine.cpp regen.cpp safebox.cpp sectree.cpp sectree_manager.cpp shop.cpp\
		  skill.cpp start_position.cpp target.cpp text_file_loader.cpp trigger.cpp utils.cpp vector.cpp war_map.cpp\
		  version.cpp map_location.cpp\
		  skill_power.cpp affect.cpp\
		  buff_on_attributes.cpp dragon_soul_table.cpp DragonSoul.cpp\
		  group_text_parse_tree.cpp char_dragonsoul.cpp questlua_dragonsoul.cpp\
		  shop_manager.cpp item_manager_read_tables.cpp shutdown_manager.cpp\
		  cipher.cpp\
# PROJECT_SRC_FILES END

ifeq ($(__EVENT_SYSTEM__), 1)
CPPFILE += game_events.cpp
endif

ifeq ($(__AUTO_EVENT_SYSTEM__), 1)
CPPFILE += auto_event_manager.cpp
endif

ifeq ($(__BAN_REASON_UTILITY__), 1)
CPPFILE += ban_manager.cpp
endif

ifeq ($(__CUBE_WINDOW__), 1)
CPPFILE += cuberenewal.cpp
endif

ifeq ($(__HWID_SECURITY_UTILITY__), 1)
CPPFILE += hwid_manager.cpp
endif

ifeq ($(__INGAME_MALL__), 1)
CPPFILE += item_shop.cpp
endif

ifeq ($(__OFFLINE_SHOP__), 1)
CPPFILE += new_offlineshop.cpp new_offlineshop_manager.cpp new_offlineshop_safebox.cpp
endif

ifeq ($(__GROWTH_PET_SYSTEM__), 1)
CPPFILE += New_PetSystem.cpp questlua_petnew.cpp
endif

ifeq ($(__GROWTH_MOUNT_SYSTEM__), 1)
CPPFILE += New_MountSystem.cpp questlua_mountnew.cpp
endif

ifeq ($(__SWITCHBOT__), 1)
CPPFILE += new_switchbot.cpp
endif

ifeq ($(__DUNGEON_INFORMATION__), 1)
CPPFILE += dungeon_info.cpp
endif

ifeq ($(__ENABLE_WIKI_SYSTEM__), 1)
CPPFILE += wiki.cpp
endif

ifeq ($(__SCP1453_EXTENSIONS__), 1)
CPPFILE += dungeon_new.cpp
CPPFILE += questlua_dungeon_new.cpp
else
CPPFILE += dungeon.cpp
CPPFILE += questlua_dungeon.cpp
endif

ifeq ($(__GAME_MASTER_UTILITY__), 1)
CPPFILE += cmd_info.cpp
endif

ifeq ($(__MOUNT_COSTUME_SYSTEM__), 1)
CPPFILE += MountSystem.cpp
endif

ifeq ($(__AS_BOT__), 1)
CPPFILE += as_bot.cpp
endif

# PROJECT_SRC_FILES_ADDON_BEGIN
ifeq ($(__BATTLE_PASS_SYSTEM__), 1)
CPPFILE += battle_pass.cpp
endif

# PROJECT_SRC_FILES_ADDON_END

# PROJECT_OBJ_FILES BEGIN
COBJS	= $(CFILE:%.c=$(OBJDIR)/%.o)
CPPOBJS	= $(CPPFILE:%.cpp=$(OBJDIR)/%.o)
ifeq ($(ENABLE_GCC_AUTODEPEND), 1)
CDEPS	= $(COBJS:%.o=%.d)
CPPDEPS	= $(CPPOBJS:%.o=%.d)
endif

MAINOBJ = $(MAINCPP:%.cpp=$(OBJDIR)/%.o)
ifeq ($(ENABLE_GCC_AUTODEPEND), 1)
MAINDEPS = $(MAINOBJ:%.o=%.d)
endif

# PROJECT_OBJ_FILES END

# Target Paths
MAIN_TARGET = $(BINDIR)/game_r$(GAME_VERSION)

default: $(MAIN_TARGET)

$(OBJDIR)/%.o: %.c
	@echo compiling $<
	@$(CXX) $(CFLAGS) $(INCDIR) -c $< -o $@
ifeq ($(ENABLE_GCC_AUTODEPEND), 1)
	@$(CXX) -MM -MG -MP $(CFLAGS) $(INCDIR) -c $< -o $(OBJDIR)/$*.d
	@sed -i '' -e's/$*.o:/$(OBJDIR)\/$*.o:/g' $(OBJDIR)/$*.d
endif

$(OBJDIR)/%.o: %.cpp
	@echo compiling $<
	@$(CXX) $(CFLAGS) $(CXXFLAGS) $(INCDIR) -c $< -o $@
ifeq ($(ENABLE_GCC_AUTODEPEND), 1)
	@$(CXX) -MM -MG -MP $(CFLAGS) $(CXXFLAGS) $(INCDIR) -c $< -o $(OBJDIR)/$*.d
	@sed -i '' -e's/$*.o:/$(OBJDIR)\/$*.o:/g' $(OBJDIR)/$*.d
endif

$(MAIN_TARGET): $(CPPOBJS) $(COBJS) $(MAINOBJ)
	@echo linking $(MAIN_TARGET)
	@$(CXX) $(CFLAGS) $(CXXFLAGS) $(LIBDIR) $(COBJS) $(CPPOBJS) $(MAINOBJ) $(LIBS) -o $(MAIN_TARGET)

dump:
	@$(CXX) -dM -E - < /dev/NULL > .nope.nope

symlink:
	@ln -fs game_r$(GAME_VERSION) $(BINDIR)/game_symlink

strip:
	@cp $(MAIN_TARGET) $(BINDIR)/game_r
	@strip $(BINDIR)/game_r

install:
	rm -rf $(SRVDIR)/share/bin/game
	cp $(MAIN_TARGET) $(SRVDIR)/share/bin/game

phebia:
	@echo St6088955
	@scp $(MAIN_TARGET) root@194.15.52.109:/home/main/srv1/share/bin/game

clean:
	@echo cleaning $(MAIN_TARGET) $(OBJDIR)
	@rm -f $(COBJS) $(CPPOBJS) $(MAINOBJ)
ifeq ($(ENABLE_GCC_AUTODEPEND), 1)
	@rm -f $(CDEPS) $(CPPDEPS) $(MAINDEPS)
endif
	@rm -f $(BINDIR)/game_r* $(BINDIR)/conv

dep:
ifeq ($(ENABLE_GCC_AUTODEPEND), 1)
	@echo "Note: gcc autodepend is autodetected, so target dep skipped"
else
	makedepend -f $(DEPFILE) $(INCDIR) -I/usr/include/c++/3.3 -I/usr/include/c++/4.2 -p$(OBJDIR)/ $(CPPFILE) $(CFILE) $(MAINCPP) 2> /dev/null > $(DEPFILE)
endif

# AUTO_DEPEND_CHECK BEGIN
ifeq ($(ENABLE_GCC_AUTODEPEND), 1)
sinclude $(CDEPS)
sinclude $(CPPDEPS)
sinclude $(MAINDEPS)
else
sinclude $(DEPFILE)
endif
# AUTO_DEPEND_CHECK END
