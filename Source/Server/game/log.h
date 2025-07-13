1* Search for:
		void		HackShieldLog(uint32_t ErrorCode, LPCHARACTER ch);

2* Add below:
#ifdef __ENABLE_NEW_OFFLINESHOP__
    void OfflineShopWithdraw(const char* user_name, const char* item_name, DWORD item_id);
#endif