1* Search for:
#include "../CWebBrowser/CWebBrowser.h"

2* Add bellow:
#ifdef __ENABLE_NEW_OFFLINESHOP__
#pragma comment( lib, "shlwapi.lib" )
#endif

3* Search for:
	initBackground();

4* Add bellow:
#ifdef __ENABLE_NEW_OFFLINESHOP__
	initofflineshop();
#endif