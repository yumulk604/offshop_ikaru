1* Add at the end of file:
#define __ENABLE_NEW_OFFLINESHOP__
#define __ENABLE_NEW_SHOP_IN_CITIES__
#define ENABLE_OFFLINESHOP_DEBUG
#if defined(ENABLE_OFFLINESHOP_DEBUG) && defined(_DEBUG)
#define OFFSHOP_DEBUG(fmt , ...) Tracenf("%s:%d >> " fmt , __FUNCTION__ , __LINE__, __VA_ARGS__)
#else
#define OFFSHOP_DEBUG(...)   
#endif