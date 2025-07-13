1* Search for:
#include "affect.h"

2* Add below:
#ifdef __ENABLE_NEW_OFFLINESHOP__
#include "new_offlineshop.h"
#include "new_offlineshop_manager.h"
#endif

3* Search for:
	m_dwFlyTargetID = 0;


4* Add below:
#ifdef __ENABLE_NEW_OFFLINESHOP__
	m_pkOfflineShop = NULL;
	m_pkShopSafebox	= NULL;
	m_pkAuction		= NULL;
	m_pkAuctionGuest= NULL;
	m_pkOfflineShopGuest = NULL;
	m_bIsLookingOfflineshopOfferList = false;
#endif

5* Search for:
	if (!CHARACTER_MANAGER::instance().FlushDelayedSave(this))
	{
		SaveReal();
	}

6* Add below:
#ifdef __ENABLE_NEW_OFFLINESHOP__
	offlineshop::GetManager().RemoveSafeboxFromCache(GetPlayerID());
	offlineshop::GetManager().RemoveGuestFromShops(this);

	if(m_pkAuctionGuest)
	{
		m_pkAuctionGuest->RemoveGuest(this);
	}

	if (GetOfflineShop())
	{
		SetOfflineShop(NULL);
	}

	SetShopSafebox(NULL);

	m_pkAuction		= NULL;
	m_pkAuctionGuest= NULL;
	m_bIsLookingOfflineshopOfferList = false;
#endif

7* Add to the end of file:
#ifdef __ENABLE_NEW_OFFLINESHOP__
void CHARACTER::SetShopSafebox(offlineshop::CShopSafebox* pk) 
{
	if(m_pkShopSafebox && pk==NULL)
	{
		m_pkShopSafebox->SetOwner(NULL);
	}

	else if(m_pkShopSafebox==NULL && pk)
	{
		pk->SetOwner(this);
	}

	m_pkShopSafebox  = pk;
}
#endif