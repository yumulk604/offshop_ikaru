1* Search for:
	HEADER_GD_SETUP			= 0xff,

2* Add above:
#ifdef __ENABLE_NEW_OFFLINESHOP__
	HEADER_GD_NEW_OFFLINESHOP	= number_of_last_one(maximum 255),
#endif

3* /*** Example of my SourceCode ***/
	HEADER_GD_CHANGE_LANGUAGE					= 144,
#ifdef __DUNGEON_FOR_GUILD__
	HEADER_GD_GUILD_DUNGEON						= 145,
	HEADER_GD_GUILD_DUNGEON_CD					= 146,
	HEADER_GD_GUILD_DUNGEON_ST					= 147,
#endif
#ifdef __ENABLE_NEW_OFFLINESHOP__
	HEADER_GD_NEW_OFFLINESHOP					= 148,
#endif
#ifdef ENABLE_BATTLE_PASS
	HEADER_GD_SAVE_BATTLE_PASS					= 149,
	HEADER_GD_REGISTER_BP_RANKING				= 150,
	HEADER_GD_BATTLE_PASS_RANKING				= 151,
#endif
#ifdef ENABLE_SKILL_COLOR_SYSTEM
	HEADER_GD_SKILL_COLOR_SAVE					= 152,
#endif
#ifdef __ADMIN_MANAGER__
	HEADER_GD_ADMIN_MANAGER_CHAT_SEARCH_PLAYER	= 153,
#endif
	HEADER_GD_SETUP								= 0xff,

4* Search for:
	HEADER_DG_MAP_LOCATIONS		= 0xfe,

5* Add above:
#ifdef __ENABLE_NEW_OFFLINESHOP__
	HEADER_DG_NEW_OFFLINESHOP = number_of_last_one(maximum 255),
#endif

6* /*** Example of my SourceCode ***/
	HEADER_DG_NEED_LOGIN_LOG					= 177,
	HEADER_DG_UNUSED_178						= 178,
	HEADER_DG_RESULT_CHARGE_CASH				= 179,
	HEADER_DG_ITEMAWARD_INFORMER				= 180,
	HEADER_DG_RESPOND_CHANNELSTATUS				= 181,
	HEADER_DG_UNUSED_182						= 182,
#ifdef __DUNGEON_FOR_GUILD__
	HEADER_DG_GUILD_DUNGEON						= 183,
	HEADER_DG_GUILD_DUNGEON_CD					= 184,
	HEADER_DG_GUILD_DUNGEON_ST					= 185,
#endif
#ifdef __ENABLE_NEW_OFFLINESHOP__
	HEADER_DG_NEW_OFFLINESHOP					= 186,
#endif
#ifdef ENABLE_BATTLE_PASS
	HEADER_DG_BATTLE_PASS_LOAD					= 187,
	HEADER_DG_BATTLE_PASS_LOAD_RANKING			= 188,
#endif
#ifdef ENABLE_SKILL_COLOR_SYSTEM
	HEADER_DG_SKILL_COLOR_LOAD					= 189,
#endif
#ifdef __ADMIN_MANAGER__
	HEADER_DG_ADMIN_MANAGER_CHAT_SEARCH_PLAYER	= 190,
#endif
	HEADER_DG_MAP_LOCATIONS						= 0xfe,

7* Search for:
#pragma pack()

8* Add above:
#ifdef __ENABLE_NEW_OFFLINESHOP__
typedef struct SPacketGDNewOfflineShop
{
	BYTE bSubHeader;
} TPacketGDNewOfflineShop;

typedef struct SPacketDGNewOfflineShop
{
	BYTE bSubHeader;
} TPacketDGNewOfflineShop;

namespace offlineshop
{
	enum class ExpirationType
	{
		EXPIRE_NONE,
		EXPIRE_REAL_TIME,
		EXPIRE_REAL_TIME_FIRST_USE,
	};	

	typedef struct SPriceInfo
	{
		long long illYang;
#ifdef __ENABLE_CHEQUE_SYSTEM__
		int iCheque;
#endif

		SPriceInfo() : illYang(0)
#ifdef ENABLE_CHEQUE_SYSTEM
			,iCheque(0)
#endif
		{}

		bool operator < (const SPriceInfo& rItem) const
		{
			return GetTotalYangAmount() < rItem.GetTotalYangAmount();
		}

		long long GetTotalYangAmount() const
		{
			long long total = illYang;
#ifdef __ENABLE_CHEQUE_SYSTEM__
			total += (long long) YANG_PER_CHEQUE * (long long) iCheque;
#endif
			return total;
		}
	} TPriceInfo;

	typedef struct SItemInfoEx
	{
		DWORD dwVnum;
		DWORD dwCount;
		long alSockets[ITEM_SOCKET_MAX_NUM];
		TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_MAX_NUM];
#ifdef __ENABLE_CHANGELOOK_SYSTEM__
		DWORD dwTransmutation;
#endif
		ExpirationType expiration;
	} TItemInfoEx;

	typedef struct SItemInfo
	{
		DWORD dwOwnerID, dwItemID;
		TPriceInfo price;
		TItemInfoEx item;
	} TItemInfo;

	typedef struct SOfferInfo
	{
		DWORD dwOfferID, dwOwnerID, dwItemID, dwOffererID;
		TPriceInfo price;
		bool bNoticed, bAccepted;
		char szBuyerName[CHARACTER_NAME_MAX_LEN+1];
	} TOfferInfo;

	typedef struct SAuctionInfo
	{
		DWORD dwOwnerID;
		char szOwnerName[CHARACTER_NAME_MAX_LEN + 1];
		DWORD dwDuration;
		TPriceInfo init_price;
		TItemInfoEx item;
	} TAuctionInfo;

	typedef struct SAuctionOfferInfo
	{
		TPriceInfo price;
		DWORD dwOwnerID;
		DWORD dwBuyerID;
		char szBuyerName[CHARACTER_NAME_MAX_LEN + 1];
	} TAuctionOfferInfo;

	typedef struct SValutesInfoa
	{
		long long illYang;
#ifdef __ENABLE_CHEQUE_SYSTEM__
		int iCheque;
#endif

		void operator +=(const SValutesInfoa& r)
		{
			illYang += r.illYang;
#ifdef __ENABLE_CHEQUE_SYSTEM__
			iCheque += r.iCheque;
#endif
		}

		void operator -=(const SValutesInfoa& r)
		{
			illYang -= r.illYang;
#ifdef __ENABLE_CHEQUE_SYSTEM__
			iCheque -= r.iCheque;
#endif
		}

		SValutesInfoa() : illYang(0)
#ifdef ENABLE_CHEQUE_SYSTEM
			, iCheque(0)
#endif
		{}
	} TValutesInfo;

	typedef struct SShopInfo
	{
		DWORD dwOwnerID;
		DWORD dwDuration;
		char szName[OFFLINE_SHOP_NAME_MAX_LEN];
		DWORD dwCount;
	} TShopInfo;

#ifdef ENABLE_IRA_REWORK 
	typedef struct SShopPosition
	{
		long lMapIndex;
		long x, y;
		BYTE bChannel;
	} TShopPosition;
#endif

	enum eNewOfflineshopSubHeaderGD
	{
		SUBHEADER_GD_BUY_ITEM = 0,
		SUBHEADER_GD_BUY_LOCK_ITEM,
		SUBHEADER_GD_CANNOT_BUY_LOCK_ITEM,
		SUBHEADER_GD_EDIT_ITEM,
		SUBHEADER_GD_REMOVE_ITEM,
		SUBHEADER_GD_ADD_ITEM,
		SUBHEADER_GD_SHOP_FORCE_CLOSE,
		SUBHEADER_GD_SHOP_CREATE_NEW,
		SUBHEADER_GD_SHOP_CHANGE_NAME,
		SUBHEADER_GD_OFFER_CREATE,
		SUBHEADER_GD_OFFER_NOTIFIED,
		SUBHEADER_GD_OFFER_ACCEPT,
		SUBHEADER_GD_OFFER_CANCEL,
		SUBHEADER_GD_SAFEBOX_GET_ITEM,
		SUBHEADER_GD_SAFEBOX_GET_VALUTES,
		SUBHEADER_GD_SAFEBOX_ADD_ITEM,
		SUBHEADER_GD_AUCTION_CREATE,
		SUBHEADER_GD_AUCTION_ADD_OFFER,
	};

	typedef struct SSubPacketGDBuyItem
	{
		DWORD dwOwnerID, dwItemID, dwGuestID;
	} TSubPacketGDBuyItem;

	typedef struct SSubPacketGDLockBuyItem
	{
		DWORD dwOwnerID, dwItemID, dwGuestID;
	} TSubPacketGDLockBuyItem;

	typedef struct SSubPacketGDCannotBuyLockItem
	{
		DWORD dwOwnerID, dwItemID;
	} TSubPacketGDCannotBuyLockItem;

	typedef struct SSubPacketGDEditItem
	{
		DWORD dwOwnerID, dwItemID;
		TPriceInfo priceInfo;
	} TSubPacketGDEditItem;

	typedef struct SSubPacketGDRemoveItem
	{
		DWORD dwOwnerID;
		DWORD dwItemID;
	} TSubPacketGDRemoveItem;

	typedef struct SSubPacketGDAddItem
	{
		DWORD dwOwnerID;
		TItemInfo itemInfo;
	} TSubPacketGDAddItem;

	typedef struct SSubPacketGDShopForceClose
	{
		DWORD dwOwnerID;
	} TSubPacketGDShopForceClose;

	typedef struct SSubPacketGDShopCreateNew
	{
		TShopInfo shop;
#ifdef ENABLE_IRA_REWORK
		TShopPosition pos;
#endif
	} TSubPacketGDShopCreateNew;

	typedef struct SSubPacketGDShopChangeName
	{
		DWORD dwOwnerID;
		char szName[OFFLINE_SHOP_NAME_MAX_LEN];
	} TSubPacketGDShopChangeName;

	typedef struct SSubPacketGDOfferCreate
	{
		DWORD dwOwnerID, dwItemID;
		TOfferInfo offer;
	} TSubPacketGDOfferCreate;

	typedef struct SSubPacketGDOfferCancel
	{
		DWORD dwOfferID;
		DWORD dwOwnerID;
	}TSubPacketGDOfferCancel;

	typedef struct SSubPacketGDOfferNotified
	{
		DWORD dwOwnerID, dwOfferID;
	} TSubPacketGDOfferNotified;

	typedef struct SSubPacketGDOfferAccept
	{
		DWORD dwOwnerID, dwOfferID;
	} TSubPacketGDOfferAccept;

	typedef struct SSubPacketGDSafeboxGetItem
	{
		DWORD dwOwnerID;
		DWORD dwItemID;
	} TSubPacketGDSafeboxGetItem;

	typedef struct SSubPacketGDSafeboxAddItem
	{
		DWORD			dwOwnerID;
		TItemInfoEx		item;
	} TSubPacketGDSafeboxAddItem;

	typedef struct SSubPacketGDSafeboxGetValutes
	{
		DWORD dwOwnerID;
		TValutesInfo valute;
	} TSubPacketGDSafeboxGetValutes;

	typedef struct SSubPacketGDAuctionCreate
	{
		TAuctionInfo auction;
	} TSubPacketGDAuctionCreate;

	typedef struct SSubPacketGDAuctionAddOffer
	{
		TAuctionOfferInfo offer;
	} TSubPacketGDAuctionAddOffer;

	enum eSubHeaderDGNewOfflineshop
	{
		SUBHEADER_DG_BUY_ITEM,
		SUBHEADER_DG_LOCKED_BUY_ITEM,
		SUBHEADER_DG_EDIT_ITEM,
		SUBHEADER_DG_REMOVE_ITEM,
		SUBHEADER_DG_ADD_ITEM,
		SUBHEADER_DG_SHOP_FORCE_CLOSE,
		SUBHEADER_DG_SHOP_CREATE_NEW,
		SUBHEADER_DG_SHOP_CHANGE_NAME,
		SUBHEADER_DG_SHOP_EXPIRED,
		SUBHEADER_DG_OFFER_CREATE,
		SUBHEADER_DG_OFFER_NOTIFIED,
		SUBHEADER_DG_OFFER_ACCEPT,
		SUBHEADER_DG_OFFER_CANCEL,
		SUBHEADER_DG_LOAD_TABLES,
		SUBHEADER_DG_SAFEBOX_ADD_ITEM,
		SUBHEADER_DG_SAFEBOX_ADD_VALUTES,
		SUBHEADER_DG_SAFEBOX_LOAD,
		SUBHEADER_DG_SAFEBOX_EXPIRED_ITEM,
		SUBHEADER_DG_AUCTION_CREATE,
		SUBHEADER_DG_AUCTION_ADD_OFFER,
		SUBHEADER_DG_AUCTION_EXPIRED,
	};

	typedef struct SSubPacketDGBuyItem
	{
		DWORD dwOwnerID, dwItemID, dwBuyerID;
	} TSubPacketDGBuyItem;

	typedef struct SSubPacketDGLockedBuyItem
	{
		DWORD dwOwnerID, dwItemID, dwBuyerID;
	} TSubPacketDGLockedBuyItem;

	typedef struct SSubPacketDGEditItem
	{
		DWORD dwOwnerID, dwItemID;
		TPriceInfo price;
	} TSubPacketDGEditItem;

	typedef struct SSubPacketDGRemoveItem
	{
		DWORD dwOwnerID, dwItemID;
	} TSubPacketDGRemoveItem;

	typedef struct SSubPacketDGAddItem
	{
		DWORD dwOwnerID, dwItemID;
		TItemInfo item;
	} TSubPacketDGAddItem;

	typedef struct SSubPacketDGShopForceClose
	{
		DWORD dwOwnerID;
	} TSubPacketDGShopForceClose;

	typedef struct SSubPacketDGShopCreateNew
	{
		TShopInfo shop;
#ifdef ENABLE_IRA_REWORK
		TShopPosition pos;
#endif
	} TSubPacketDGShopCreateNew;

	typedef struct SSubPacketDGShopChangeName
	{
		DWORD dwOwnerID;
		char szName[OFFLINE_SHOP_NAME_MAX_LEN];
	} TSubPacketDGShopChangeName;

	typedef struct SSubPacketDGOfferCreate
	{
		DWORD dwOwnerID, dwItemID;
		TOfferInfo offer;
	} TSubPacketDGOfferCreate;

	typedef struct SSubPacketDGOfferCancel
	{
		DWORD dwOfferID;
		DWORD dwOwnerID;
		bool IsRemovingItem;
	} TSubPacketDGOfferCancel;

	typedef struct SSubPacketDGOfferNotified
	{
		DWORD dwOwnerID, dwOfferID;
	} TSubPacketDGOfferNotified;

	typedef struct SSubPacketDGOfferAccept
	{
		DWORD dwOwnerID, dwOfferID;
	} TSubPacketDGOfferAccept;

	typedef struct SSubPacketDGLoadTables
	{
		DWORD dwShopCount;
		DWORD dwOfferCount;
		DWORD dwAuctionCount;
		DWORD dwAuctionOfferCount;
	} TSubPacketDGLoadTables;

	typedef struct SSubPacketDGShopExpired
	{
		DWORD dwOwnerID;
	} TSubPacketDGShopExpired;

	typedef struct SSubPacketDGSafeboxAddItem
	{
		DWORD dwOwnerID, dwItemID;
		TItemInfoEx item;
	} TSubPacketDGSafeboxAddItem;

	typedef struct SSubPacketDGSafeboxAddValutes
	{
		DWORD dwOwnerID;
		TValutesInfo valute;
	} TSubPacketDGSafeboxAddValutes;

	typedef struct SSubPacketDGSafeboxLoad
	{
		DWORD dwOwnerID;
		TValutesInfo valute;
		DWORD dwItemCount;
	} TSubPacketDGSafeboxLoad;

	typedef struct SSubPacketDGSafeboxExpiredItem
	{
		DWORD dwOwnerID;
		DWORD dwItemID;
	} TSubPacketDGSafeboxExpiredItem;

	typedef struct SSubPacketDGAuctionCreate
	{
		TAuctionInfo auction;
	} TSubPacketDGAuctionCreate;

	typedef struct SSubPacketDGAuctionAddOffer
	{
		TAuctionOfferInfo offer;
	} TSubPacketDGAuctionAddOffer;

	typedef struct SSubPacketDGAuctionExpired
	{
		DWORD dwOwnerID;
	} TSubPacketDGAuctionExpired;
}
#endif
