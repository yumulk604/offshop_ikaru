1* Search for:
	ENTITY_OBJECT,

2* Add below:
#if defined(__ENABLE_NEW_OFFLINESHOP__) && defined(__ENABLE_NEW_SHOP_IN_CITIES__)
	ENTITY_NEWSHOPS,
#endif