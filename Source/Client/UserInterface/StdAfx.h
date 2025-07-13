1* Search for:
extern DWORD __DEFAULT_CODE_PAGE__;

2* Add below:
enum EOther
{
#ifdef __ENABLE_NEW_OFFLINESHOP__
	OFFLINE_SHOP_NAME_MAX_LEN = 40 + CHARACTER_NAME_MAX_LEN + 1,
	OFFLINE_SHOP_ITEM_MAX_LEN = 24,
#endif
};

3* Add at the end of file:
#ifdef __ENABLE_NEW_OFFLINESHOP__
void initofflineshop();
#endif