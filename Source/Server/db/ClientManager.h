1* Search for:
#include "LoginData.h"

2* Add below:
#ifdef __ENABLE_NEW_OFFLINESHOP__
#include "OfflineshopCache.h"
#endif

3* Search for:
	public:
	CClientManager();
	~CClientManager();

4* Add above:
#ifdef __ENABLE_NEW_OFFLINESHOP__
public:
	bool InitializeOfflineshopTable();
	bool RecvOfflineShopBuyItemPacket(const char* data);
	bool RecvOfflineShopLockBuyItem(CPeer* peer, const char* data);
	bool RecvOfflineShopCannotBuyLockItem(const char* data);
	bool RecvOfflineShopEditItemPacket(const char* data);
	bool RecvOfflineShopRemoveItemPacket(const char* data);
	bool RecvOfflineShopAddItemPacket(const char* data);
	bool RecvOfflineShopForceClose(const char* data);
	bool RecvOfflineShopCreateNew(const char* data);
	bool RecvOfflineShopChangeName(const char* data);
	bool RecvOfflineShopOfferCreate(const char* data);
	bool RecvOfflineShopOfferNotified(const char* data);
	bool RecvOfflineShopOfferAccepted(const char* data);
	bool RecvOfflineshopOfferCancel(const char* data);
	bool RecvOfflineShopSafeboxGetItem(const char* data);
	bool RecvOfflineShopSafeboxGetValutes(const char* data);
	bool RecvOfflineShopSafeboxAddItem(const char* data);
	bool RecvOfflineShopAuctionCreate(const char* data);
	bool RecvOfflineShopAuctionAddOffer(const char* data);
	bool SendOfflineShopBuyItemPacket(DWORD dwOwner, DWORD dwGuest, DWORD dwItem);
	bool SendOfflineShopBuyLockedItemPacket(CPeer* peer,DWORD dwOwner, DWORD dwGuest, DWORD dwItem);
	bool SendOfflineShopEditItemPacket(DWORD dwOwner, DWORD dwItem, const offlineshop::TPriceInfo& price);
	bool SendOfflineShopRemoveItemPacket(DWORD dwOwner, DWORD dwItem);
	bool SendOfflineShopAddItemPacket(DWORD dwOwner, DWORD dwItemID, const offlineshop::TItemInfo& rInfo);
	bool SendOfflineShopForceClose(DWORD dwOwnerID);
#ifdef ENABLE_IRA_REWORK
	bool SendOfflineShopCreateNew(const offlineshop::TShopInfo& shop, const std::vector<offlineshop::TItemInfo>& vec, offlineshop::TShopPosition& pos);
#else
	bool SendOfflineShopCreateNew(const offlineshop::TShopInfo& shop, const std::vector<offlineshop::TItemInfo>& vec);
#endif
	bool SendOfflineShopChangeName(DWORD dwOwnerID, const char* szName);
	bool SendOfflineshopShopExpired(DWORD dwOwnerID);
	bool SendOfflineShopOfferCreate(const offlineshop::TOfferInfo& offer);
	bool SendOfflineShopOfferNotified(DWORD dwOwnerID, DWORD dwOfferID);
	bool SendOfflineShopOfferAccepted(DWORD dwOwnerID, DWORD dwOfferID);
	void SendOfflineshopTable(CPeer* peer);
	void RecvOfflineShopPacket(CPeer* peer, const char* data);
	void SendOfflineShopOfferCancel(DWORD dwOwnerID, DWORD dwOfferID, bool isRemovingItem);
	void SendOfflineshopSafeboxAddItem(DWORD dwOwnerID, DWORD dwItem, const offlineshop::TItemInfoEx& item);
	void SendOfflineshopSafeboxAddValutes(DWORD dwOwnerID, const offlineshop::TValutesInfo& valute);
	void SendOfflineshopSafeboxLoad(CPeer* peer ,DWORD dwOwnerID, const offlineshop::TValutesInfo& valute, const std::vector<offlineshop::TItemInfoEx>& items , const std::vector<DWORD>& ids);
	void SendOfflineshopSafeboxExpiredItem(DWORD dwOwnerID, DWORD itemID);
	void SendOfflineshopAuctionCreate(const offlineshop::TAuctionInfo& auction);
	void SendOfflineshopAuctionAddOffer(const offlineshop::TAuctionOfferInfo& offer);
	void SendOfflineshopAuctionExpired(DWORD dwOwnerID);
	void OfflineShopResultQuery(CPeer* peer, SQLMsg* msg, CQueryInfo* pQueryInfo);
	void OfflineShopResultAddItemQuery(CPeer* peer, SQLMsg* msg, CQueryInfo* pQueryInfo);
	void OfflineShopResultCreateShopQuery(CPeer* peer, SQLMsg* msg, CQueryInfo* pQueryInfo);
	void OfflineShopResultCreateShopAddItemQuery(CPeer* peer, SQLMsg* msg, CQueryInfo* pQueryInfo);
	void OfflineShopResultOfferAddQuery(CPeer* peer, SQLMsg* msg, CQueryInfo* pQueryInfo);
	void OfflineShopResultSafeboxAddItemQuery(CPeer* peer, SQLMsg* msg, CQueryInfo* pQueryInfo);
	void OfflineshopDurationProcess();
	void OfflineshopExpiredShop(DWORD dwID);
	void OfflineshopExpiredAuction(DWORD dwID);
	void OfflineshopLoadShopSafebox(CPeer* peer ,DWORD dwID);
	bool IsUsingOfflineshopSystem(DWORD dwID);
	void OfflineshopExpiredAuctionItem(DWORD dwOwnerID);
#ifdef ENABLE_NEW_OFFLINESHOP_LOGS
	void OfflineshopLog(const DWORD dwOwnerID, const DWORD dwItemID, const char* fmt, ...);
#endif
private:
	offlineshop::CShopCache m_offlineshopShopCache;
	offlineshop::CSafeboxCache m_offlineshopSafeboxCache;
	offlineshop::COfferCache m_offlineshopOfferCache;
	offlineshop::CAuctionCache m_offlineshopAuctionCache;
#endif