1* Search for:
ACMD(do_slow);

2* Add below:
#ifdef __ENABLE_NEW_OFFLINESHOP__
ACMD(do_showofflineshop);
ACMD(do_offshop_change_shop_name);
ACMD(do_offshop_force_close_shop);
#endif

3* Search for
	{ "slow",		do_slow,		0,			POS_DEAD,	GM_HIGH_WIZARD	},

4* Add below:
#ifdef __ENABLE_NEW_OFFLINESHOP__
	{ "showofflineshop", do_showofflineshop, 0, POS_DEAD, GM_PLAYER },
	{ "offshop_change_shop_name", do_offshop_change_shop_name, 0,  POS_DEAD, GM_IMPLEMENTOR },
	{ "offshop_force_close_shop", do_offshop_force_close_shop, 0,  POS_DEAD, GM_IMPLEMENTOR },
#endif