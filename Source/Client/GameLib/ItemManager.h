*1 Search for:
		BOOL			GetItemDataPointer(DWORD dwItemID, CItemData ** ppItemData);

*2 Add below:
#ifdef __ENABLE_NEW_OFFLINESHOP__
		void			GetItemsNameMap(std::map<DWORD, std::string>& inMap);
#endif