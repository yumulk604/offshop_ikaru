1* Search for:
#include "shop_manager.h"

2* Add below:
#ifdef __ENABLE_NEW_OFFLINESHOP__
#include "new_offlineshop.h"
#include "new_offlineshop_manager.h"
#endif

3* Search for:
	ilInit();

4* Add above:
#ifdef __ENABLE_NEW_OFFLINESHOP__
#ifndef ENABLE_IRA_REWORK
	if(!Offlineshop_InitializeLibrary("REMOVED", "shit-lib"))
	{
		fprintf(stderr, "\nCANNOT INITIALIZE OFFLINESHOP LIBRARY : (you can comment this RETURN to works without physical shops) recommended to text to ikarus.\n");
		return 0;
	}
#endif
#endif

5* Search for:
	MessengerManager::instance().Initialize();

6* Add above:
#ifdef __ENABLE_NEW_OFFLINESHOP__
	offlineshop::CShopManager offshopManager;
#endif

7* Search for:
	while (idle());

8* Add below:
#ifdef __ENABLE_NEW_OFFLINESHOP__
#ifndef ENABLE_IRA_REWORK
	Offlineshop_CleanUpLibrary();
#endif
#endif