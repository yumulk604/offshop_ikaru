1* Search for:
#ifdef ENABLE_ENERGY_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_ENERGY_SYSTEM",	1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_ENERGY_SYSTEM",	0);
#endif

2* Add below:
#ifdef __ENABLE_NEW_OFFLINESHOP__
	PyModule_AddIntConstant(poModule, "__ENABLE_NEW_OFFLINESHOP__", 1);
#else
	PyModule_AddIntConstant(poModule, "__ENABLE_NEW_OFFLINESHOP__", 0);
#endif