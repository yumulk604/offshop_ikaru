1* Search for:
#include "p2p.h"

2* Add below:
#ifdef __ENABLE_NEW_OFFLINESHOP__
#include "new_offlineshop.h"
#include "new_offlineshop_manager.h"
#endif


3* Search for:
int32_t CInputDB::Analyze(LPDESC d, uint8_t bHeader, const char * c_pData)

4* Add above:
#ifdef __ENABLE_NEW_OFFLINESHOP__
template <class T>
const char* Decode(T*& pObj, const char* data)
{
	pObj = (T*) data;
	return data + sizeof(T);
}

void OfflineShopLoadTables(const char* data)
{
	offlineshop::TSubPacketDGLoadTables* pSubPack = nullptr;
	data = Decode(pSubPack, data);
	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	OFFSHOP_DEBUG("shop count %u , offer count %u , auction count %u, auction offers %u ",pSubPack->dwShopCount , pSubPack->dwOfferCount, pSubPack->dwAuctionCount , pSubPack->dwAuctionOfferCount);
	for (DWORD i = 0; i < pSubPack->dwShopCount; i++)
	{
		offlineshop::TShopInfo* pShop = nullptr;
#ifdef ENABLE_IRA_REWORK
		offlineshop::TShopPosition* pShopPos = nullptr;
#endif
		offlineshop::TItemInfo* pItem = nullptr;
		DWORD* pdwSoldCount= nullptr;
		data = Decode(pShop, data);
#ifdef ENABLE_IRA_REWORK
		data = Decode(pShopPos, data);
#endif
		data = Decode(pdwSoldCount, data);
		OFFSHOP_DEBUG("shop %u %s (solds %u) ",pShop->dwOwnerID , pShop->szName , *pdwSoldCount);
#ifdef ENABLE_IRA_REWORK
		offlineshop::CShop* pkShop = rManager.PutsNewShop(pShop, pShopPos);
#else
		offlineshop::CShop* pkShop = rManager.PutsNewShop(pShop);
#endif
		for (DWORD j = 0; j < pShop->dwCount; j++)
		{
			data = Decode(pItem, data);
			offlineshop::CShopItem kItem(pItem->dwItemID);
			kItem.SetOwnerID(pItem->dwOwnerID);
			kItem.SetInfo(pItem->item);
			kItem.SetPrice(pItem->price);
			kItem.SetWindow(NEW_OFFSHOP);
			OFFSHOP_DEBUG("for sale item %u ",pItem->dwItemID);
			pkShop->AddItem(kItem);
		}
		for (DWORD j = 0; j < *pdwSoldCount; j++)
		{
			data = Decode(pItem, data);
			offlineshop::CShopItem kItem(pItem->dwItemID);
			kItem.SetOwnerID(pItem->dwOwnerID);
			kItem.SetInfo(pItem->item);
			kItem.SetPrice(pItem->price);
			kItem.SetWindow(NEW_OFFSHOP);
			OFFSHOP_DEBUG("sold item %u ",pItem->dwItemID);
			pkShop->AddItemSold(kItem);
		}
	}

	offlineshop::TOfferInfo* pOffer=nullptr;

	for (DWORD i = 0; i < pSubPack->dwOfferCount; i++)
	{
		data = Decode(pOffer, data);
		OFFSHOP_DEBUG("offer shop : id %u , shopid %u, itemid %u, buyer %u ",pOffer->dwOfferID, pOffer->dwOwnerID, pOffer->dwItemID , pOffer->dwOffererID);
		offlineshop::CShop* pkShop = rManager.GetShopByOwnerID(pOffer->dwOwnerID);
		if (!pkShop)
		{
			sys_err("CANNOT FIND SHOP BY OWNERID (TOfferInfo) %d ",pOffer->dwOwnerID);
			continue;
		}
		pkShop->AddOffer(pOffer);
		rManager.PutsNewOffer(pOffer);
	}

	offlineshop::TAuctionInfo*		pTempAuction=nullptr;
	offlineshop::TAuctionOfferInfo* pTempAuctionOffer=nullptr;

	for (DWORD i = 0; i < pSubPack->dwAuctionCount; i++)
	{
		data = Decode(pTempAuction, data);
		rManager.PutsAuction(*pTempAuction);
		OFFSHOP_DEBUG("auction %u id , %s name , %u minutes ",pTempAuction->dwOwnerID , pTempAuction->szOwnerName, pTempAuction->dwDuration);
	}

	for (DWORD i = 0; i < pSubPack->dwAuctionOfferCount; i++)
	{
		data = Decode(pTempAuctionOffer, data);
		rManager.PutsAuctionOffer(*pTempAuctionOffer);
		OFFSHOP_DEBUG("offer %u shop , %s buyer ",pTempAuctionOffer->dwOwnerID, pTempAuctionOffer->szBuyerName);
	}
}

void OfflineShopBuyItemPacket(const char* data)
{
	offlineshop::TSubPacketDGBuyItem* subpack;
	data = Decode(subpack, data);
	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	rManager.RecvShopBuyDBPacket(subpack->dwBuyerID, subpack->dwOwnerID, subpack->dwItemID);
}

void OfflineShopLockedBuyItemPacket(const char* data)
{
	offlineshop::TSubPacketDGLockedBuyItem* subpack;
	data = Decode(subpack, data);
	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	rManager.RecvShopLockedBuyItemDBPacket(subpack->dwBuyerID, subpack->dwOwnerID, subpack->dwItemID);
}

void OfflineShopEditItemPacket(const char* data)
{
	offlineshop::TSubPacketDGEditItem* subpack;
	data = Decode(subpack, data);
	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	rManager.RecvShopEditItemDBPacket(subpack->dwOwnerID , subpack->dwItemID, subpack->price);
}

void OfflineShopRemoveItemPacket(const char* data)
{
	offlineshop::TSubPacketDGRemoveItem* subpack;
	data = Decode(subpack, data);
	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	rManager.RecvShopRemoveItemDBPacket(subpack->dwOwnerID , subpack->dwItemID);
}

void OfflineShopAddItemPacket(const char* data)
{
	offlineshop::TSubPacketDGAddItem* subpack;
	data = Decode(subpack, data);
	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	rManager.RecvShopAddItemDBPacket(subpack->dwOwnerID, subpack->item);
}

void OfflineShopForceClosePacket(const char* data)
{
	offlineshop::TSubPacketDGShopForceClose* subpack;
	data = Decode(subpack, data);
	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	rManager.RecvShopForceCloseDBPacket(subpack->dwOwnerID);
}

void OfflineShopShopCreateNewPacket(const char* data)
{
	offlineshop::TSubPacketDGShopCreateNew* subpack;
	data = Decode(subpack, data);
	OFFSHOP_DEBUG("shop %u , dur %u , count %u ",subpack->shop.dwOwnerID , subpack->shop.dwDuration , subpack->shop.dwCount);
	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	std::vector<offlineshop::TItemInfo> vec;
	vec.reserve(subpack->shop.dwCount);
	offlineshop::TItemInfo* pItemInfo=nullptr;
	for (DWORD i = 0; i < subpack->shop.dwCount; i++)
	{
		data = Decode(pItemInfo, data);
		vec.push_back(*pItemInfo);
		OFFSHOP_DEBUG("item id %u , item vnum %u , item count %u ",pItemInfo->dwItemID , pItemInfo->item.dwVnum , pItemInfo->item.dwCount);
	}
#ifdef ENABLE_IRA_REWORK
	rManager.RecvShopCreateNewDBPacket(subpack->shop, subpack->pos, vec);
#else
	rManager.RecvShopCreateNewDBPacket(subpack->shop, vec);
#endif
}

void OfflineShopShopChangeNamePacket(const char* data)
{
	offlineshop::TSubPacketDGShopChangeName* subpack;
	data = Decode(subpack, data);
	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	rManager.RecvShopChangeNameDBPacket(subpack->dwOwnerID , subpack->szName);
}

void OfflineShopOfferCreatePacket(const char* data)
{
	offlineshop::TSubPacketDGOfferCreate* subpack;
	data = Decode(subpack, data);
	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	rManager.RecvShopOfferNewDBPacket(subpack->offer);
}

void OfflineShopOfferNotifiedPacket(const char* data)
{
	offlineshop::TSubPacketDGOfferNotified* subpack;
	data = Decode(subpack, data);
	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	rManager.RecvShopOfferNotifiedDBPacket(subpack->dwOfferID , subpack->dwOwnerID);
}

void OfflineShopOfferAcceptPacket(const char* data)
{
	offlineshop::TSubPacketDGOfferAccept* subpack;
	data = Decode(subpack, data);
	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	rManager.RecvShopOfferAcceptDBPacket(subpack->dwOfferID , subpack->dwOwnerID);
}

void OfflineShopOfferCancelPacket(const char* data)
{
	offlineshop::TSubPacketDGOfferCancel* subpack;
	data = Decode(subpack, data);
	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	rManager.RecvShopOfferCancelDBPacket(subpack->dwOfferID , subpack->dwOwnerID, subpack->IsRemovingItem);
}

void OfflineShopSafeboxAddItemPacket(const char* data)
{
	offlineshop::TSubPacketDGSafeboxAddItem* subpack;
	data = Decode(subpack, data);
	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	rManager.RecvShopSafeboxAddItemDBPacket(subpack->dwOwnerID , subpack->dwItemID , subpack->item);
}

void OfflineShopSafeboxAddValutesPacket(const char* data)
{
	offlineshop::TSubPacketDGSafeboxAddValutes* subpack;
	data = Decode(subpack, data);
	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	rManager.RecvShopSafeboxAddValutesDBPacket(subpack->dwOwnerID , subpack->valute);
}

void OfflineShopSafeboxLoad(const char* data)
{
	offlineshop::TSubPacketDGSafeboxLoad* subpack;
	data = Decode(subpack, data);
	std::vector<DWORD> ids;
	std::vector<offlineshop::TItemInfoEx> items;
	ids.reserve(subpack->dwItemCount);
	items.reserve(subpack->dwItemCount);
	DWORD* pdwItemID=nullptr;
	offlineshop::TItemInfoEx* temp;
	for (DWORD i = 0; i < subpack->dwItemCount; i++)
	{
		data = Decode(pdwItemID, data);
		data = Decode(temp, data);
		ids.push_back(*pdwItemID);
		items.push_back(*temp);
	}
	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	rManager.RecvShopSafeboxLoadDBPacket(subpack->dwOwnerID , subpack->valute , ids, items);
}

void OfflineshopSafeboxExpiredItem(const char* data)
{
	offlineshop::TSubPacketDGSafeboxExpiredItem* subpack;
	data = Decode(subpack, data);

	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	rManager.RecvShopSafeboxExpiredItemDBPacket(subpack->dwOwnerID, subpack->dwItemID);
}

void OfflineShopAuctionCreate(const char* data)
{
	offlineshop::TSubPacketDGAuctionCreate* subpack;
	data = Decode(subpack, data);
	offlineshop::GetManager().RecvAuctionCreateDBPacket(subpack->auction);
}

void OfflineShopAuctionAddOffer(const char* data)
{
	offlineshop::TSubPacketDGAuctionAddOffer* subpack;
	data = Decode(subpack, data);
	offlineshop::GetManager().RecvAuctionAddOfferDBPacket(subpack->offer);
}

void OfflineShopAuctionExpired(const char* data)
{
	offlineshop::TSubPacketDGAuctionExpired* subpack;
	data = Decode(subpack, data);
	offlineshop::GetManager().RecvAuctionExpiredDBPacket(subpack->dwOwnerID);
}

void OfflineshopShopExpired(const char* data)
{
	offlineshop::TSubPacketDGShopExpired* subpack;
	data = Decode(subpack, data);
	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	rManager.RecvShopExpiredDBPacket(subpack->dwOwnerID);
}

void OfflineshopPacket(const char* data)
{
	TPacketDGNewOfflineShop* pPack=nullptr;
	data = Decode(pPack, data);

	OFFSHOP_DEBUG("recv subheader %d",pPack->bSubHeader);

	switch (pPack->bSubHeader)
	{
		case offlineshop::SUBHEADER_DG_LOAD_TABLES:			OfflineShopLoadTables(data);				return;
		case offlineshop::SUBHEADER_DG_BUY_ITEM:			OfflineShopBuyItemPacket(data);				return;
		case offlineshop::SUBHEADER_DG_LOCKED_BUY_ITEM:		OfflineShopLockedBuyItemPacket(data);		return;
		case offlineshop::SUBHEADER_DG_EDIT_ITEM:			OfflineShopEditItemPacket(data);			return;
		case offlineshop::SUBHEADER_DG_REMOVE_ITEM:			OfflineShopRemoveItemPacket(data);			return;
		case offlineshop::SUBHEADER_DG_ADD_ITEM:			OfflineShopAddItemPacket(data);				return;
		case offlineshop::SUBHEADER_DG_SHOP_FORCE_CLOSE:	OfflineShopForceClosePacket(data);			return;
		case offlineshop::SUBHEADER_DG_SHOP_CREATE_NEW:		OfflineShopShopCreateNewPacket(data);		return;
		case offlineshop::SUBHEADER_DG_SHOP_CHANGE_NAME:	OfflineShopShopChangeNamePacket(data);		return;
		case offlineshop::SUBHEADER_DG_SHOP_EXPIRED:		OfflineshopShopExpired(data);				return;
		case offlineshop::SUBHEADER_DG_OFFER_CREATE:		OfflineShopOfferCreatePacket(data);			return;
		case offlineshop::SUBHEADER_DG_OFFER_NOTIFIED:		OfflineShopOfferNotifiedPacket(data);		return;
		case offlineshop::SUBHEADER_DG_OFFER_ACCEPT:		OfflineShopOfferAcceptPacket(data);			return;
		case offlineshop::SUBHEADER_DG_OFFER_CANCEL:		OfflineShopOfferCancelPacket(data);			return;
		case offlineshop::SUBHEADER_DG_SAFEBOX_ADD_ITEM:	OfflineShopSafeboxAddItemPacket(data);		return;
		case offlineshop::SUBHEADER_DG_SAFEBOX_ADD_VALUTES:	OfflineShopSafeboxAddValutesPacket(data);	return;
		case offlineshop::SUBHEADER_DG_SAFEBOX_LOAD:		OfflineShopSafeboxLoad(data);				return;
		case offlineshop::SUBHEADER_DG_SAFEBOX_EXPIRED_ITEM:	OfflineshopSafeboxExpiredItem(data);	return;
		case offlineshop::SUBHEADER_DG_AUCTION_CREATE:		OfflineShopAuctionCreate(data);				return;
		case offlineshop::SUBHEADER_DG_AUCTION_ADD_OFFER:	OfflineShopAuctionAddOffer(data);			return;
		case offlineshop::SUBHEADER_DG_AUCTION_EXPIRED:		OfflineShopAuctionExpired(data);			return;
		default:
			sys_err("UKNOWN SUB HEADER %d ", pPack->bSubHeader);
			return;
	}
}
#endif


5* Search for:
	case HEADER_DG_RESPOND_CHANNELSTATUS:
		RespondChannelStatus(DESC_MANAGER::instance().FindByHandle(m_dwHandle), c_pData);
		break;

6* Add below:
#ifdef __ENABLE_NEW_OFFLINESHOP__
	case HEADER_DG_NEW_OFFLINESHOP:
		OfflineshopPacket(c_pData);
		break;
#endif
