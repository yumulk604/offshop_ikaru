1* Search for:
			float							fHeight;

2* Add below:
#ifdef __ENABLE_NEW_OFFLINESHOP__
#	ifdef __ENABLE_NEW_SHOP_IN_CITIES__
			bool  bIsShop;
			bool  bRender;
#	endif
#endif

3* Search for:
		void Clear();

4* Add below:
#ifdef __ENABLE_NEW_SHOP_IN_CITIES__
		void RegisterShopInstanceTextTail(DWORD dwVirtualID, const char* c_szName, CGraphicObjectInstance* pOwner);
		void DeleteShopTextTail(DWORD VirtualID);
		TTextTail * RegisterShopTextTail(DWORD dwVirtualID, const char * c_szText, CGraphicObjectInstance * pOwner);
		bool GetPickedNewShop(DWORD* pdwVID);
#endif

5* Search for:
		TTextTailList				m_ItemTextTailList;

6* Add below:
#ifdef __ENABLE_NEW_SHOP_IN_CITIES__
		TTextTailMap m_ShopTextTailMap;
#endif
