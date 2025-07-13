1* Search for:
	if (!InitializeObjectTable())
	{
		sys_err("InitializeObjectTable FAILED");
		return false;
	}

2* Add below:
#ifdef __ENABLE_NEW_OFFLINESHOP__
	if (!InitializeOfflineshopTable())
	{
		sys_err("InitializeOfflineshopTable FAILED");
		return false;
	}
#endif