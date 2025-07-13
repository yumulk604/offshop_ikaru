1* Search for:
#pragma pack()

2* Add above:
#ifdef __ENABLE_NEW_OFFLINESHOP__
enum EShopPackets
{
	HEADER_CG_NEW_OFFLINESHOP = 245,
	HEADER_GC_NEW_OFFLINESHOP = 246,
};

typedef struct SPacketGCNewOfflineshop
{
	BYTE bHeader;
	uint16_t wSize;
	BYTE bSubHeader;
} TPacketGCNewOfflineshop;

typedef struct SPacketCGNewOfflineShop
{
	BYTE bHeader;
	uint16_t wSize;
	BYTE bSubHeader;
} TPacketCGNewOfflineShop;

namespace offlineshop
{
	typedef struct SFilterInfo
	{
		BYTE bType;
		BYTE bSubType;
		char szName[OFFLINE_SHOP_ITEM_MAX_LEN];
		TPriceInfo priceStart, priceEnd;
		int iLevelStart, iLevelEnd;
		DWORD dwWearFlag;
		TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_NORM_NUM];
	} TFilterInfo;

	typedef struct SShopItemInfo
	{
		TItemPos pos;
		TPriceInfo price;
	} TShopItemInfo;

	typedef struct SAuctionListElement
	{
		TAuctionInfo auction;
		TPriceInfo actual_best;
		DWORD dwOfferCount;
	} TAuctionListElement;

	typedef struct SMyOfferExtraInfo
	{
		TItemInfo item;
		char szShopName[OFFLINE_SHOP_NAME_MAX_LEN];
	} TMyOfferExtraInfo;

	enum eSubHeaderGC
	{
		SUBHEADER_GC_SHOP_LIST,
		SUBHEADER_GC_SHOP_OPEN,
		SUBHEADER_GC_SHOP_OPEN_OWNER,
		SUBHEADER_GC_SHOP_OPEN_OWNER_NO_SHOP,
		SUBHEADER_GC_SHOP_CLOSE,
		SUBHEADER_GC_SHOP_BUY_ITEM_FROM_SEARCH,
		SUBHEADER_GC_OFFER_LIST,
		SUBHEADER_GC_SHOP_FILTER_RESULT,
		SUBHEADER_GC_SHOP_SAFEBOX_REFRESH,
		SUBHEADER_GC_AUCTION_LIST,
		SUBHEADER_GC_OPEN_MY_AUCTION,
		SUBHEADER_GC_OPEN_MY_AUCTION_NO_AUCTION,
		SUBHEADER_GC_OPEN_AUCTION,
#ifdef __ENABLE_NEW_SHOP_IN_CITIES__
		SUBHEADER_GC_INSERT_SHOP_ENTITY,
		SUBHEADER_GC_REMOVE_SHOP_ENTITY,
#endif
	};

	typedef struct SSubPacketGCShopList
	{
		DWORD dwShopCount;
	} TSubPacketGCShopList;

	typedef struct SSubPacketGCShopOpen
	{
		TShopInfo shop;
	} TSubPacketGCShopOpen;

	typedef struct SSubPacketGCShopOpenOwner
	{
		TShopInfo shop;
		DWORD dwSoldCount;
		DWORD dwOfferCount;
	} TSubPacketGCShopOpenOwner;

	typedef struct SSubPacketGCShopFilterResult
	{
		DWORD dwCount;
	} TSubPacketGCShopFilterResult;

	typedef struct SSubPacketGCShopOfferList
	{
		DWORD dwOfferCount;
	} TSubPacketGCShopOfferList;

	typedef struct SSubPacketGCShopSafeboxRefresh
	{
		TValutesInfo valute;
		DWORD dwItemCount;
	} TSubPacketGCShopSafeboxRefresh;

	typedef struct SSubPacketGCShopBuyItemFromSearch
	{
		DWORD dwOwnerID;
		DWORD dwItemID;
	} TSubPacketGCShopBuyItemFromSearch;

	typedef struct SSubPacketGCAuctionList
	{
		DWORD dwCount;
	} TSubPacketGCAuctionList;

	typedef struct SSubPacketGCAuctionOpen
	{
		TAuctionInfo auction;
		DWORD dwOfferCount;
	} TSubPacketGCAuctionOpen;

#ifdef __ENABLE_NEW_SHOP_IN_CITIES__
	typedef struct SSubPacketGCInsertShopEntity
	{
		DWORD dwVID;
		char szName[OFFLINE_SHOP_NAME_MAX_LEN];
		int iType;
		long x, y, z;
	} TSubPacketGCInsertShopEntity;

	typedef struct SSubPacketGCRemoveShopEntity
	{
		DWORD dwVID;
	} TSubPacketGCRemoveShopEntity;
#endif

	enum eSubHeaderCG
	{
		SUBHEADER_CG_SHOP_CREATE_NEW,
		SUBHEADER_CG_SHOP_CHANGE_NAME,
		SUBHEADER_CG_SHOP_FORCE_CLOSE,
		SUBHEADER_CG_SHOP_REQUEST_SHOPLIST,
		SUBHEADER_CG_SHOP_OPEN,
		SUBHEADER_CG_SHOP_OPEN_OWNER,
		SUBHEADER_CG_SHOP_BUY_ITEM,
		SUBHEADER_CG_SHOP_ADD_ITEM,
		SUBHEADER_CG_SHOP_REMOVE_ITEM,
		SUBHEADER_CG_SHOP_EDIT_ITEM,
		SUBHEADER_CG_SHOP_FILTER_REQUEST,
		SUBHEADER_CG_SHOP_OFFER_CREATE,
		SUBHEADER_CG_SHOP_OFFER_ACCEPT,
		SUBHEADER_CG_SHOP_OFFER_CANCEL,
		SUBHEADER_CG_SHOP_REQUEST_OFFER_LIST,
		SUBHEADER_CG_SHOP_SAFEBOX_OPEN,
		SUBHEADER_CG_SHOP_SAFEBOX_GET_ITEM,
		SUBHEADER_CG_SHOP_SAFEBOX_GET_VALUTES,
		SUBHEADER_CG_SHOP_SAFEBOX_CLOSE,
		SUBHEADER_CG_AUCTION_LIST_REQUEST,
		SUBHEADER_CG_AUCTION_OPEN_REQUEST,
		SUBHEADER_CG_MY_AUCTION_OPEN_REQUEST,
		SUBHEADER_CG_CREATE_AUCTION,
		SUBHEADER_CG_AUCTION_ADD_OFFER,
		SUBHEADER_CG_EXIT_FROM_AUCTION,
		SUBHEADER_CG_CLOSE_BOARD,
#ifdef __ENABLE_NEW_SHOP_IN_CITIES__
		SUBHEADER_CG_CLICK_ENTITY,
#endif
	};

	typedef struct SSubPacketCGShopCreate
	{
		TShopInfo shop;
	} TSubPacketCGShopCreate;

	typedef struct SSubPacketCGShopChangeName
	{
		char szName[OFFLINE_SHOP_NAME_MAX_LEN];
	} TSubPacketCGShopChangeName;

	typedef struct SSubPacketCGShopOpen
	{
		DWORD dwOwnerID;
	} TSubPacketCGShopOpen;

	typedef struct SSubPacketCGAddItem
	{
		TItemPos pos;
		TPriceInfo price;
	} TSubPacketCGAddItem;

	typedef struct SSubPacketCGRemoveItem
	{
		DWORD dwItemID;
	} TSubPacketCGRemoveItem;

	typedef struct SSubPacketCGEditItem
	{
		DWORD dwItemID;
		TPriceInfo price;
	} TSubPacketCGEditItem;

	typedef struct SSubPacketCGFilterRequest
	{
		TFilterInfo filter;
	} TSubPacketCGFilterRequest;

	typedef struct SSubPacketCGOfferCreate
	{
		TOfferInfo offer;
	} TSubPacketCGOfferCreate;

	typedef struct SSubPacketCGOfferAccept
	{
		DWORD dwOfferID;
	} TSubPacketCGOfferAccept;

	typedef struct SSubPacketCGOfferCancel
	{
		DWORD dwOfferID;
		DWORD dwOwnerID;
	} TSubPacketCGOfferCancel;

	typedef struct SSubPacketCGShopSafeboxGetItem
	{
		DWORD dwItemID;
	} TSubPacketCGShopSafeboxGetItem;

	typedef struct SSubPacketCGShopSafeboxGetValutes
	{
		TValutesInfo valutes;
	} TSubPacketCGShopSafeboxGetValutes;

	typedef struct SSubPacketCGShopBuyItem
	{
		DWORD dwOwnerID;
		DWORD dwItemID;
		bool bIsSearch;
	} TSubPacketCGShopBuyItem;

	typedef struct SSubPacketCGAuctionOpenRequest
	{
		DWORD dwOwnerID;
	} TSubPacketCGAuctionOpenRequest;

	typedef struct SSubPacketCGAuctionCreate
	{
		DWORD dwDuration;
		TItemPos pos;
		TPriceInfo init_price;
	} TSubPacketCGAuctionCreate;

	typedef struct SSubPacketCGAuctionAddOffer
	{
		DWORD dwOwnerID;
		TPriceInfo price;
	} TSubPacketCGAuctionAddOffer;

	typedef struct SSubPacketCGAuctionExitFrom
	{
		DWORD dwOwnerID;
	} TSubPacketCGAuctionExitFrom;

#ifdef __ENABLE_NEW_SHOP_IN_CITIES__
	typedef struct SSubPacketCGShopClickEntity
	{
		DWORD dwShopVID;
	} TSubPacketCGShopClickEntity;
#endif
}
#endif
