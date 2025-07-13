1* Search for:
#include "pvp.h"

2* Add below:
#ifdef __ENABLE_NEW_OFFLINESHOP__
#include "new_offlineshop.h"
#include "new_offlineshop_manager.h"
#endif

3* Search for:
	ch->StartCheckSpeedHackEvent();

4* Add below:
#ifdef __ENABLE_NEW_OFFLINESHOP__
	if (ch->IsPC())
	{
		offlineshop::CShop* pkShop= offlineshop::GetManager().GetShopByOwnerID(ch->GetPlayerID());
		if(pkShop)
		{
			ch->SetOfflineShop(pkShop);
		}

		offlineshop::CAuction* auction = offlineshop::GetManager().GetAuctionByOwnerID(ch->GetPlayerID());
		if(auction)
		{
			ch->SetAuction(auction);
		}
	}
#endif