1* Search for:
		bool ClientCommand(const char * c_szCommand);
		void ServerCommand(char * c_szCommand);

2* Add below:
#ifdef __ENABLE_NEW_OFFLINESHOP__
		bool RecvOfflineshopPacket();
		bool RecvOfflineshopShopList();
		bool RecvOfflineshopShopOpen();
		bool RecvOfflineshopShopOpenOwner();
		bool RecvOfflineshopShopOpenOwnerNoShop();
		bool RecvOfflineshopShopClose();
		bool RecvOfflineshopShopFilterResult();
		bool RecvOfflineshopOfferList();
		bool RecvOfflineshopShopSafeboxRefresh();
		bool RecvOfflineshopShopBuyItemFromSearch();
		bool RecvOfflineshopAuctionList();
		bool RecvOfflineshopOpenMyAuction();
		bool RecvOfflineshopOpenMyAuctionNoAuction();
		bool RecvOfflineshopOpenAuction();
#ifdef __ENABLE_NEW_SHOP_IN_CITIES__
		bool RecvOfflineshopInsertEntity();
		bool RecvOfflineshopRemoveEntity();
		void SendOfflineshopOnClickShopEntity(DWORD dwPickedShopVID);
#endif
		void SendOfflineshopShopCreate(const offlineshop::TShopInfo& shopInfo, const std::vector<offlineshop::TShopItemInfo>& items);
		void SendOfflineshopChangeName(const char* szName);
		void SendOfflineshopForceCloseShop();
		void SendOfflineshopRequestShopList();
		void SendOfflineshopOpenShop(DWORD dwOwnerID);
		void SendOfflineshopOpenShopOwner();
		void SendOfflineshopBuyItem(DWORD dwOwnerID, DWORD dwItemID, bool isSearch);
		void SendOfflineshopAddItem(offlineshop::TShopItemInfo& itemInfo);
		void SendOfflineshopRemoveItem(DWORD dwItemID);
		void SendOfflineShopEditItem(DWORD dwItemID, const offlineshop::TPriceInfo& price);
		void SendOfflineshopFilterRequest(const offlineshop::TFilterInfo& filter);
		void SendOfflineshopOfferCreate(const offlineshop::TOfferInfo& offer);
		void SendOfflineshopOfferAccept(DWORD dwOfferID);
		void SendOfflineshopOfferCancel(DWORD dwOfferID, DWORD dwOwnerID);
		void SendOfflineshopOfferListRequest();
		void SendOfflineshopSafeboxOpen();
		void SendOfflineshopSafeboxGetItem(DWORD dwItemID);
		void SendOfflineshopSafeboxGetValutes(const offlineshop::TValutesInfo& valutes);
		void SendOfflineshopSafeboxClose();
		void SendOfflineshopAuctionListRequest();
		void SendOfflineshopAuctionOpen(DWORD dwOwnerID);
		void SendOfflineshopAuctionAddOffer(DWORD dwOwnerID, const offlineshop::TPriceInfo& price);
		void SendOfflineshopAuctionExitFrom(DWORD dwOwnerID);
		void SendOfflineshopAuctionCreate(const TItemPos& pos, const offlineshop::TPriceInfo& price, DWORD dwDuration);
		void SendOfflineshopAuctionOpenMy();
		void SendOfflineshopCloseBoard();
#endif