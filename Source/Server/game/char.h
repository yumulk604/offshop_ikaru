1* Search for:
class CArena;

2* Add below:
#ifdef __ENABLE_NEW_OFFLINESHOP__
namespace offlineshop
{
	class CShop;
	class CShopSafebox;
	class CAuction;
}
#endif

3* Search for:
		void			SendEquipment(LPCHARACTER ch);

4* Add below:
#ifdef __ENABLE_NEW_OFFLINESHOP__
	public:
		offlineshop::CShop*			GetOfflineShop() {return m_pkOfflineShop;}
		void						SetOfflineShop(offlineshop::CShop* pkShop) {m_pkOfflineShop = pkShop;}

		offlineshop::CShop*			GetOfflineShopGuest() {return m_pkOfflineShopGuest;}
		void						SetOfflineShopGuest(offlineshop::CShop* pkShop) {m_pkOfflineShopGuest = pkShop;}

		offlineshop::CShopSafebox*
									GetShopSafebox() {return m_pkShopSafebox;}
		void						SetShopSafebox(offlineshop::CShopSafebox* pk);

		void						SetAuction(offlineshop::CAuction* pk) {m_pkAuction = pk;}
		void						SetAuctionGuest(offlineshop::CAuction* pk) {m_pkAuctionGuest = pk;}

		offlineshop::CAuction*		GetAuction()		{return m_pkAuction;}
		offlineshop::CAuction*		GetAuctionGuest()	{return m_pkAuctionGuest;}

		void						SetLookingOfflineshopOfferList(bool is) { m_bIsLookingOfflineshopOfferList = is; }
		bool						IsLookingOfflineshopOfferList() { return m_bIsLookingOfflineshopOfferList; }

	private:
		offlineshop::CShop*			m_pkOfflineShop;
		offlineshop::CShop*			m_pkOfflineShopGuest;
		offlineshop::CShopSafebox*	m_pkShopSafebox;
		offlineshop::CAuction*		m_pkAuction;
		offlineshop::CAuction*		m_pkAuctionGuest;

		bool						m_bIsLookingOfflineshopOfferList;
#endif