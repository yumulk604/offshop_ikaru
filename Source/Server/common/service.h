1* Add at the end of file:
/*** Ikarus work ***/
#define ENABLE_IRA_REWORK
#define __ENABLE_NEW_OFFLINESHOP__
#ifdef __ENABLE_NEW_OFFLINESHOP__
#define __ENABLE_NEW_SHOP_IN_CITIES__
#endif
#ifdef ENABLE_OFFLINESHOP_DEBUG
#ifdef __WIN32__
#define OFFSHOP_DEBUG(fmt , ...) sys_log(0,"%s:%d >> " fmt , __FUNCTION__ , __LINE__, __VA_ARGS__)
#else
#define OFFSHOP_DEBUG(fmt , args...) sys_log(0,"%s:%d >> " fmt , __FUNCTION__ , __LINE__, ##args)
#endif
#else
#define OFFSHOP_DEBUG(...)   
#endif
#define ENABLE_NEW_OFFLINESHOP_LOGS