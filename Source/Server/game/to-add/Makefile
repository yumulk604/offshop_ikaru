1* Search for:
CPPOBJS	= $(CPP:%.cpp=$(OBJDIR)/%.o)

2* Add above:
# ifeq ( $(ENABLE_OFFLINESHOP), 1 )
CPP += new_offlineshop.cpp new_offlineshop_manager.cpp new_offlineshop_safebox.cpp
# endif

/*************************************************** EXAMPLE OF MY MAKEFILE *******************************************/
CC = ccache c++
INCDIR =
LIBDIR =
BINDIR = ..
OBJDIR = .obj
$(shell if [ ! -d $(OBJDIR) ]; then mkdir $(OBJDIR); fi)

### CFLAGS
CFLAGS = -w -O3 -ggdb -g -gdwarf -std=c++14 -pipe -mtune=i386 -fstack-protector -m32 -static -D_THREAD_SAFE
### END

### LIBS FROM ../EXTERN and LOCAL/INCLUDE
# boost    (for boost, you need to install boost from PuTTY. Example: pkg install boost-libs).
INCDIR += -I/usr/local/include
# GSL: Guideline Support Library
INCDIR += -I../../../extern/gsl/include
# cryptopp (if cryptopp doesn't work, you need download src cryptopp 5.6.5 from https://github.com/weidai11/cryptopp/releases/tag/CRYPTOPP_5_6_5 and recompile with your actually compiller (g++/c++).
INCDIR += -I../../../extern/cryptopp
LIBDIR += -L../../../extern/cryptopp/lib
LIBS += -lcryptopp
# devil    (for that, you need to install devIL from PuTTY. Example: pkg install devil).
INCDIR += -I../../../local/include
LIBDIR += -L/usr/local/lib
LIBS   += -lil -lpng -ltiff -lmng -llcms -ljpeg -ljbig -llzma
# minilzo
INCDIR += -I../../../extern/minilzo
LIBDIR += -L../../../extern/minilzo/lib
LIBS   += -lminilzo -lmd
# mysql
INCDIR += -I../../../extern/mysql/mysql
LIBDIR += -L../../../extern/_library
LIBS   += -lmysqlclient -lz -pthread -lm -lssl -lcrypto
### END

### LIBS FROM ../SOURCE/LIB
# libgame
INCDIR += -I../../lib/libgame
LIBDIR += -L../../lib/libgame/lib
LIBS   += -lgame
# libpoly
INCDIR += -I../../lib/libpoly
LIBDIR += -L../../lib/libpoly/lib
LIBS   += -lpoly
# libsql
INCDIR += -I../../lib/libsql
LIBDIR += -L../../lib/libsql/lib
LIBS   += -lsql
# libthecore
INCDIR += -I../../lib/libthecore
LIBDIR += -L../../lib/libthecore/lib
LIBS   += -lthecore
# lua
INCDIR += -I../../../extern/lua/lua
LIBDIR += -L../../../extern/lua/lib
LIBS   += -llua
### END

CPP	=	affect.cpp\
		ani.cpp\
		arena.cpp\
		banword.cpp\
		battle.cpp\
		battle_arena.cpp\
		blend_item.cpp\
		blue_dragon.cpp\
		blue_dragon_binder.cpp\
		buff_on_attributes.cpp\
		buffer_manager.cpp\
		building.cpp\
		char.cpp\
		char_affect.cpp\
		char_battle.cpp\
		char_change_empire.cpp\
		char_dragonsoul.cpp\
		char_horse.cpp\
		char_item.cpp\
		char_manager.cpp\
		char_quickslot.cpp\
		char_resist.cpp\
		char_skill.cpp\
		char_state.cpp\
		cmd.cpp\
		cmd_emotion.cpp\
		cmd_general.cpp\
		cmd_gm.cpp\
		cmd_oxevent.cpp\
		config.cpp\
		constants.cpp\
		crc32.cpp\
		cube.cpp\
		db.cpp\
		desc.cpp\
		desc_client.cpp\
		desc_manager.cpp\
		desc_p2p.cpp\
		dragon_soul_table.cpp\
		dragon_lair.cpp\
		dragon_soul.cpp\
		dungeon.cpp\
		entity.cpp\
		entity_view.cpp\
		event.cpp\
		event_queue.cpp\
		exchange.cpp\
		file_loader.cpp\
		fishing.cpp\
		fsm.cpp\
		gm.cpp\
		group_text_parse_tree.cpp\
		guild.cpp\
		guild_manager.cpp\
		guild_war.cpp\
		horse_rider.cpp\
		horsename_manager.cpp\
		input.cpp\
		input_auth.cpp\
		input_db.cpp\
		input_login.cpp\
		input_main.cpp\
		input_p2p.cpp\
		item.cpp\
		item_addon.cpp\
		item_attribute.cpp\
		item_manager.cpp\
		item_manager_idrange.cpp\
		item_manager_read_tables.cpp\
		locale.cpp\
		locale_service.cpp\
		log.cpp\
		login_data.cpp\
		lzo_manager.cpp\
		main.cpp\
		map_location.cpp\
		mark_convert.cpp\
		mark_image.cpp\
		mark_manager.cpp\
		marriage.cpp\
		messenger_manager.cpp\
		mining.cpp\
		mob_manager.cpp\
		motion.cpp\
		ox_event.cpp\
		p2p.cpp\
		packet_info.cpp\
		party.cpp\
		pet_system.cpp\
		polymorph.cpp\
		priv_manager.cpp\
		pvp.cpp\
		questevent.cpp\
		questlua.cpp\
		questlua_affect.cpp\
		questlua_arena.cpp\
		questlua_battle_arena.cpp\
		questlua_building.cpp\
		questlua_danceevent.cpp\
		questlua_dragonlair.cpp\
		questlua_dragonsoul.cpp\
		questlua_dungeon.cpp\
		questlua_game.cpp\
		questlua_global.cpp\
		questlua_guild.cpp\
		questlua_horse.cpp\
		questlua_item.cpp\
		questlua_marriage.cpp\
		questlua_npc.cpp\
		questlua_oxevent.cpp\
		questlua_party.cpp\
		questlua_pc.cpp\
		questlua_pet.cpp\
		questlua_quest.cpp\
		questlua_target.cpp\
		questmanager.cpp\
		questnpc.cpp\
		questpc.cpp\
		refine.cpp\
		regen.cpp\
		safebox.cpp\
		sectree.cpp\
		sectree_manager.cpp\
		shop.cpp\
		shop_manager.cpp\
		shop_ex.cpp\
		skill.cpp\
		skill_power.cpp\
		start_position.cpp\
		target.cpp\
		text_file_loader.cpp\
		trigger.cpp\
		utils.cpp\
		vector.cpp\
		war_map.cpp\
		wedding.cpp\
		cipher.cpp\
		new_activity.cpp\
		new_admin_manager.cpp\
		new_aura_system.cpp\
		new_auto_event_manager.cpp\
		new_char_battle_zodiac.cpp\
		new_battle_pass.cpp\
		new_biolog_system.cpp\
		new_boss_tracking.cpp\
		new_char_fakepc.cpp\
		new_char_fakebuff.cpp\
		new_char_gaya.cpp\
		new_cuberenewal.cpp\
		new_daily_boss.cpp\
		new_damage_top.cpp\
		new_input_api.cpp\
		new_inventory.cpp\
		new_meley_lair.cpp\
		new_mount_system.cpp\
		new_nearby_scanner.cpp\
		new_profile.cpp\
		new_pet_system.cpp\
		new_questlua_new_pet_system.cpp\
		new_questlua_zodiac_temple.cpp\
		new_questlua_meley_lair.cpp\
		new_reborn_system.cpp\
		new_rune_system.cpp\
		new_server_maintenance.cpp\
		new_switchbot.cpp\
		new_snow_flake.cpp\
		new_whisper_admin.cpp\
		new_zodiac_temple.cpp\
		new_zodiac_temple_settings.cpp

# ifeq ( $(ENABLE_OFFLINESHOP), 1 )
CPP += new_offlineshop.cpp new_offlineshop_manager.cpp new_offlineshop_safebox.cpp
# endif

CPPOBJS	= $(CPP:%.cpp=$(OBJDIR)/%.o)

GAME_TARGET = $(BINDIR)/game

default: $(GAME_TARGET)

$(OBJDIR)/%.o: %.cpp
	@echo -e "\033[0;32m [OK] \033[0m" $<
	@$(CC) $(CFLAGS) $(INCDIR) -c $< -o $@

$(GAME_TARGET): $(CPPOBJS)
	@echo linking $(GAME_TARGET)
	@$(CC) $(CFLAGS) $(LIBDIR) $(CPPOBJS) $(LIBS) -o $(GAME_TARGET)

clean:
	@rm -f $(CPPOBJS)
	@rm -f $(BINDIR)/game* $(BINDIR)/conv

tag:
	ctags *.cpp *.h
