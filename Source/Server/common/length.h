1* Search for:
	BELT_INVENTORY_SLOT_COUNT = BELT_INVENTORY_SLOT_WIDTH * BELT_INVENTORY_SLOT_HEIGHT,

2* Add below:
#ifdef __ENABLE_NEW_OFFLINESHOP__
	OFFLINE_SHOP_NAME_MAX_LEN		= 40 + CHARACTER_NAME_MAX_LEN +1,
	OFFLINESHOP_DURATION_MAX_MINUTES= 8* 24* 60,
	OFFLINESHOP_MAX_SEARCH_RESULT	= 250,
	OFFLINESHOP_SECONDS_PER_SEARCH	= 15,
	OFFLINESHOP_SECONDS_PER_OFFER	= 7,
	OFFLINE_SHOP_ITEM_MAX_LEN 		= 24,
#ifdef __ENABLE_CHEQUE_SYSTEM__
	YANG_PER_CHEQUE 				= 100000000,
#endif
#endif

3* Search for in enum EWindows:
};

4* Add above:
#ifdef __ENABLE_NEW_OFFLINESHOP__
	NEW_OFFSHOP,
	SHOP_SAFEBOX,
#endif

/*** Example of enum EWindows ***/
enum EWindows
{
	RESERVED_WINDOW,
	INVENTORY,
	EQUIPMENT,
	SAFEBOX,
	MALL,
	DRAGON_SOUL_INVENTORY,
	BELT_INVENTORY,
#ifdef ENABLE_SWITCHBOT
	SWITCHBOT,
#endif
	DESTROYED_WINDOW,
#ifdef ENABLE_SPECIAL_STORAGE
	SKILL_BOOK_INVENTORY,
	UPGRADE_ITEMS_INVENTORY,
	STONE_INVENTORY,
	BONUS_INVENTORY,
	CHEST_INVENTORY,
#endif
	GROUND,
	WINDOW_TYPE_MAX,
#ifdef __ENABLE_NEW_OFFLINESHOP__
	NEW_OFFSHOP,
	SHOP_SAFEBOX,
#endif
};