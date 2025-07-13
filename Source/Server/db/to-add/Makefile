1* Search for CPP =	name_of_something.cpp\ or how you have in youR server

2* Add above:
#Offlineshop (0 = DISABLED , 1 = ENABLED)
# ENABLE_OFFLINESHOP = 1

# ifeq ( $(ENABLE_OFFLINESHOP), 1 )
CFLAGS += -D__ENABLE_NEW_OFFLINESHOP__
# endif

3* Search for:
CPPOBJS = $(CPP:%.cpp=$(OBJDIR)/%.o)

4* Add above:
# ifeq ( $(ENABLE_OFFLINESHOP), 1 )
CPP += OfflineshopCache.cpp ClientManagerOfflineshop.cpp
# endif

/*************************************************** EXAMPLE OF MY MAKEFILE *******************************************/
CC = ccache c++
INCDIR =
LIBDIR =
BINDIR = ..
OBJDIR = .obj
$(shell if [ ! -d $(OBJDIR) ]; then mkdir $(OBJDIR); fi)

### CFLAGS
CFLAGS = -w -O3 -ggdb3 -g3 -std=c++14 -pipe -mtune=i386 -fstack-protector -static -m32 -D_THREAD_SAFE -pthread
### END

### LIBS FROM ../EXTERN and LOCAL/INCLUDE
# boost    (for this, you need to install boost from PuTTY. Example: pkg install boost-libs).
INCDIR += -I/usr/local/include
# cryptopp (if cryptopp doesn't work, you need download src cryptopp 5.6.5 from https://github.com/weidai11/cryptopp/releases/tag/CRYPTOPP_5_6_5 and recompile with your actually compiller (g++/c++).
INCDIR += -I../../../extern/cryptopp/cryptopp
LIBDIR += -L../../../extern/cryptopp/lib
LIBS += -lcryptopp

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
### END

#Offlineshop (0 = DISABLED , 1 = ENABLED)
# ENABLE_OFFLINESHOP = 1

# ifeq ( $(ENABLE_OFFLINESHOP), 1 )
CFLAGS += -D__ENABLE_NEW_OFFLINESHOP__
# endif

CPP =	Cache.cpp\
		ClientManager.cpp\
		ClientManagerBoot.cpp\
		ClientManagerEventFlag.cpp\
		ClientManagerGuild.cpp\
		ClientManagerHorseName.cpp\
		ClientManagerLogin.cpp\
		ClientManagerParty.cpp\
		ClientManagerPlayer.cpp\
		Config.cpp\
		CsvReader.cpp\
		DBManager.cpp\
		Grid.cpp\
		GuildManager.cpp\
		ItemAwardManager.cpp\
		ItemIDRangeManager.cpp\
		Lock.cpp\
		LoginData.cpp\
		Main.cpp\
		Marriage.cpp\
		MoneyLog.cpp\
		NetBase.cpp\
		Peer.cpp\
		PeerBase.cpp\
		PrivManager.cpp\
		ProtoReader.cpp\
		
# ifeq ( $(ENABLE_OFFLINESHOP), 1 )
CPP += OfflineshopCache.cpp ClientManagerOfflineshop.cpp
# endif

CPPOBJS = $(CPP:%.cpp=$(OBJDIR)/%.o)

DB_TARGET = $(BINDIR)/db

default: $(DB_TARGET)

$(OBJDIR)/%.o: %.cpp
	@echo -e "\033[0;32m [OK] \033[0m" $<
	@$(CC) $(CFLAGS) $(INCDIR) -c $< -o $@

$(DB_TARGET): $(CPPOBJS)
	@echo linking $(DB_TARGET)
	@$(CC) $(CFLAGS) $(LIBDIR) $(CPPOBJS) $(LIBS) -o $(DB_TARGET)

clean:
	@rm -f $(CPPOBJS) $(BINDIR)/db*
