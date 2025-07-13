1* Search for:
int32_t CInputMain::Analyze(LPDESC d, uint8_t bHeader, const char * c_pData)

2* Add above:
#ifdef __ENABLE_NEW_OFFLINESHOP__
#include "new_offlineshop.h"
#include "new_offlineshop_manager.h"
template <class T>
bool CanDecode(T* p, int buffleft)
{
	return buffleft >= (int)sizeof(T);
}

template <class T>
const char* Decode(T*& pObj, const char* data, int* pbufferLeng = nullptr, int* piBufferLeft=nullptr)
{
	pObj = (T*) data;

	if(pbufferLeng)
		*pbufferLeng += sizeof(T);

	if(piBufferLeft)
		*piBufferLeft -= sizeof(T);

	return data + sizeof(T);
}

int OfflineshopPacketCreateNewShop(LPCHARACTER ch, const char* data, int iBufferLeft)
{
	offlineshop::TSubPacketCGShopCreate* pack = nullptr;
	if(!CanDecode(pack, iBufferLeft))
		return -1;

	int iExtra=0;
	data = Decode(pack, data, &iExtra, &iBufferLeft);

	offlineshop::TShopInfo& rShopInfo = pack->shop;

	std::vector<offlineshop::TShopItemInfo> vec;
	vec.reserve(rShopInfo.dwCount);

	offlineshop::TShopItemInfo* pItem=nullptr;

	for (DWORD i = 0; i < rShopInfo.dwCount; ++i)
	{
		if(!CanDecode(pItem, iBufferLeft))
			return -1;

		data = Decode(pItem, data, &iExtra, &iBufferLeft);
		vec.push_back(*pItem);
	}

	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	if(!rManager.RecvShopCreateNewClientPacket(ch, rShopInfo, vec))
		offlineshop::SendChatPacket(ch, offlineshop::CHAT_PACKET_CANNOT_CREATE_SHOP);

	return iExtra;
}

int OfflineshopPacketChangeShopName(LPCHARACTER ch, const char* data, int iBufferLeft)
{
	offlineshop::TSubPacketCGShopChangeName* pack = nullptr;
	if(!CanDecode(pack, iBufferLeft))
		return -1;

	int iExtra=0;
	data = Decode(pack,data, &iExtra, &iBufferLeft);

	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	if(!rManager.RecvShopChangeNameClientPacket(ch, pack->szName))
		offlineshop::SendChatPacket(ch, offlineshop::CHAT_PACKET_CANNOT_CHANGE_NAME);

	return iExtra;
}

int OfflineshopPacketForceCloseShop(LPCHARACTER ch, const char* data, int iBufferLeft)
{
	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	if(!rManager.RecvShopForceCloseClientPacket(ch))
		offlineshop::SendChatPacket(ch, offlineshop::CHAT_PACKET_CANNOT_FORCE_CLOSE);

	return 0;
}

int OfflineshopPacketRequestShopList(LPCHARACTER ch, const char* data, int iBufferLeft)
{
	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	rManager.RecvShopRequestListClientPacket(ch);
	return 0;
}

int OfflineshopPacketOpenShop(LPCHARACTER ch, const char* data, int iBufferLeft)
{
	offlineshop::TSubPacketCGShopOpen* pack = nullptr;
	if(!CanDecode(pack, iBufferLeft))
		return -1;

	int iExtra=0;
	data = Decode(pack,data, &iExtra, &iBufferLeft);

	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	if(!rManager.RecvShopOpenClientPacket(ch,pack->dwOwnerID))
		offlineshop::SendChatPacket(ch, offlineshop::CHAT_PACKET_CANNOT_OPEN_SHOP);

	return iExtra;
}

int OfflineshopPacketOpenShowOwner(LPCHARACTER ch, const char* data, int iBufferLeft)
{
	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	if(!rManager.RecvShopOpenMyShopClientPacket(ch))
		offlineshop::SendChatPacket(ch, offlineshop::CHAT_PACKET_CANNOT_OPEN_SHOP_OWNER);

	return 0;
}

int OfflineshopPacketBuyItem(LPCHARACTER ch, const char* data, int iBufferLeft)
{
	offlineshop::TSubPacketCGShopBuyItem* pack = nullptr;
	if(!CanDecode(pack, iBufferLeft))
		return -1;

	int iExtra=0;
	data = Decode(pack,data, &iExtra, &iBufferLeft);

	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	if(!rManager.RecvShopBuyItemClientPacket(ch , pack->dwOwnerID, pack->dwItemID, pack->bIsSearch))
		offlineshop::SendChatPacket(ch, offlineshop::CHAT_PACKET_CANNOT_BUY_ITEM);

	return iExtra;
}

int OfflineshopPacketAddItem(LPCHARACTER ch, const char* data, int iBufferLeft)
{
	offlineshop::TSubPacketCGAddItem* pack = nullptr;
	if(!CanDecode(pack, iBufferLeft))
		return -1;

	int iExtra=0;
	data = Decode(pack,data, &iExtra, &iBufferLeft);
	
	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	if(!rManager.RecvShopAddItemClientPacket(ch, pack->pos, pack->price))
		offlineshop::SendChatPacket(ch, offlineshop::CHAT_PACKET_CANNOT_ADD_ITEM);

	return iExtra;
}

int OfflineshopPacketRemoveItem(LPCHARACTER ch, const char* data, int iBufferLeft)
{
	offlineshop::TSubPacketCGRemoveItem* pack = nullptr;
	if(!CanDecode(pack, iBufferLeft))
		return -1;

	int iExtra=0;
	data = Decode(pack,data, &iExtra, &iBufferLeft);

	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	if(!rManager.RecvShopRemoveItemClientPacket(ch, pack->dwItemID))
		offlineshop::SendChatPacket(ch, offlineshop::CHAT_PACKET_CANNOT_REMOVE_ITEM);

	return iExtra;
}

int OfflineshopPacketEditItem(LPCHARACTER ch, const char* data, int iBufferLeft)
{
	offlineshop::TSubPacketCGEditItem* pack = nullptr;
	if(!CanDecode(pack, iBufferLeft))
		return -1;

	int iExtra=0;
	data = Decode(pack,data, &iExtra, &iBufferLeft);

	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	if(!rManager.RecvShopEditItemClientPacket(ch, pack->dwItemID, pack->price))
		offlineshop::SendChatPacket(ch, offlineshop::CHAT_PACKET_CANNOT_EDIT_ITEM);

	return iExtra;
}

int OfflineshopPacketFilterRequest(LPCHARACTER ch, const char* data, int iBufferLeft)
{
	offlineshop::TSubPacketCGFilterRequest* pack = nullptr;
	if(!CanDecode(pack, iBufferLeft))
		return -1;

	int iExtra=0;
	data = Decode(pack,data, &iExtra, &iBufferLeft);

	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	if(!rManager.RecvShopFilterRequestClientPacket(ch, pack->filter))
		offlineshop::SendChatPacket(ch, offlineshop::CHAT_PACKET_CANNOT_FILTER);

	return iExtra;
}

int OfflineshopPacketCreateOffer(LPCHARACTER ch, const char* data, int iBufferLeft)
{
	offlineshop::TSubPacketCGOfferCreate* pack = nullptr;
	if(!CanDecode(pack, iBufferLeft))
		return -1;

	int iExtra=0;
	data = Decode(pack,data, &iExtra, &iBufferLeft);

	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	if(!rManager.RecvShopCreateOfferClientPacket(ch, pack->offer))
		offlineshop::SendChatPacket(ch, offlineshop::CHAT_PACKET_CANNOT_CREATE_OFFER);

	return iExtra;
}

int OfflineshopPacketAcceptOffer(LPCHARACTER ch, const char* data, int iBufferLeft)
{
	offlineshop::TSubPacketCGOfferAccept* pack = nullptr;
	if(!CanDecode(pack, iBufferLeft))
		return -1;

	int iExtra=0;
	data = Decode(pack,data, &iExtra, &iBufferLeft);

	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	if(!rManager.RecvShopAcceptOfferClientPacket(ch, pack->dwOfferID))
		offlineshop::SendChatPacket(ch, offlineshop::CHAT_PACKET_CANNOT_ACCEPT_OFFER);

	return iExtra;
}

int OfflineshopPacketOfferCancel(LPCHARACTER ch, const char* data, int iBufferLeft)
{
	offlineshop::TSubPacketCGOfferCancel* pack = nullptr;
	if(!CanDecode(pack, iBufferLeft))
		return -1;

	int iExtra=0;
	data = Decode(pack,data, &iExtra, &iBufferLeft);

	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	if(!rManager.RecvShopCancelOfferClientPacket(ch, pack->dwOfferID, pack->dwOwnerID))
		offlineshop::SendChatPacket(ch, offlineshop::CHAT_PACKET_CANNOT_ACCEPT_OFFER);

	return iExtra;
}

int OfflineshopPacketOfferListRequest(LPCHARACTER ch)
{
	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	rManager.RecvOfferListRequestPacket(ch);
	return 0;
}

int OfflineshopPacketOpenSafebox(LPCHARACTER ch, const char* data, int iBufferLeft)
{
	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	if(!rManager.RecvShopSafeboxOpenClientPacket(ch))
		offlineshop::SendChatPacket(ch, offlineshop::CHAT_PACKET_CANNOT_OPEN_SAFEBOX);

	return 0;
}

int OfflineshopPacketCloseBoard(LPCHARACTER ch)
{
	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	rManager.RecvCloseBoardClientPacket(ch);
	return 0;
}

int OfflineshopPacketGetItemSafebox(LPCHARACTER ch, const char* data, int iBufferLeft)
{
	offlineshop::TSubPacketCGShopSafeboxGetItem* pack = nullptr;
	if(!CanDecode(pack, iBufferLeft))
		return -1;

	int iExtra=0;
	data = Decode(pack,data, &iExtra, &iBufferLeft);

	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	if(!rManager.RecvShopSafeboxGetItemClientPacket(ch, pack->dwItemID))
		offlineshop::SendChatPacket(ch, offlineshop::CHAT_PACKET_CANNOT_SAFEBOX_GET_ITEM);

	return iExtra;
}

int OfflineshopPacketGetValutesSafebox(LPCHARACTER ch, const char* data, int iBufferLeft)
{
	offlineshop::TSubPacketCGShopSafeboxGetValutes* pack = nullptr;
	if(!CanDecode(pack, iBufferLeft))
		return -1;

	int iExtra=0;
	data = Decode(pack,data, &iExtra, &iBufferLeft);

	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	if(!rManager.RecvShopSafeboxGetValutesClientPacket(ch, pack->valutes))
		offlineshop::SendChatPacket(ch, offlineshop::CHAT_PACKET_CANNOT_SAFEBOX_GET_VALUTES);

	return iExtra;
}

int OfflineshopPacketCloseSafebox(LPCHARACTER ch, const char* data, int iBufferLeft)
{
	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	if(!rManager.RecvShopSafeboxCloseClientPacket(ch))
		offlineshop::SendChatPacket(ch, offlineshop::CHAT_PACKET_CANNOT_SAFEBOX_CLOSE);

	return 0;
}

int OfflineshopPacketListRequest(LPCHARACTER ch, const char* data, int iBufferLeft)
{
	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	if(!rManager.RecvAuctionListRequestClientPacket(ch))
		offlineshop::SendChatPacket(ch, offlineshop::CHAT_PACKET_AUCTION_CANNOT_SEND_LIST);

	return 0;
}

int OfflineshopPacketOpenAuctionRequest(LPCHARACTER ch, const char* data, int iBufferLeft)
{
	offlineshop::TSubPacketCGAuctionOpenRequest* pack = nullptr;
	if(!CanDecode(pack, iBufferLeft))
		return -1;

	int iExtra=0;
	data = Decode(pack,data, &iExtra, &iBufferLeft);

	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	if(!rManager.RecvAuctionOpenRequestClientPacket(ch, pack->dwOwnerID))
		offlineshop::SendChatPacket(ch, offlineshop::CHAT_PACKET_AUCTION_CANNOT_OPEN_AUCTION);

	return iExtra;
}

int OfflineshopPacketOpenMyAuctionRequest(LPCHARACTER ch, const char* data, int iBufferLeft)
{
	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	if(!rManager.RecvMyAuctionOpenRequestClientPacket(ch))
		offlineshop::SendChatPacket(ch, offlineshop::CHAT_PACKET_AUCTION_CANNOT_SEND_LIST);

	return 0;
}

int OfflineshopPacketCreateAuction(LPCHARACTER ch, const char* data, int iBufferLeft)
{
	offlineshop::TSubPacketCGAuctionCreate* pack = nullptr;
	if(!CanDecode(pack, iBufferLeft))
		return -1;

	int iExtra=0;
	data = Decode(pack,data, &iExtra, &iBufferLeft);

	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	if(!rManager.RecvAuctionCreateClientPacket(ch, pack->dwDuration, pack->init_price, pack->pos))
		offlineshop::SendChatPacket(ch, offlineshop::CHAT_PACKET_AUCTION_CANNOT_CREATE_AUCTION);

	return iExtra;
}

int OfflineshopPacketAddOffer(LPCHARACTER ch, const char* data, int iBufferLeft)
{
	offlineshop::TSubPacketCGAuctionAddOffer* pack = nullptr;
	if(!CanDecode(pack, iBufferLeft))
		return -1;

	int iExtra=0;
	data = Decode(pack,data, &iExtra, &iBufferLeft);

	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	if(!rManager.RecvAuctionAddOfferClientPacket(ch, pack->dwOwnerID, pack->price))
		offlineshop::SendChatPacket(ch, offlineshop::CHAT_PACKET_AUCTION_CANNOT_ADD_OFFER);

	return iExtra;
}

int OfflineshopPacketExitFromAuction(LPCHARACTER ch, const char* data, int iBufferLeft)
{
	offlineshop::TSubPacketCGAuctionExitFrom* pack = nullptr;
	if(!CanDecode(pack, iBufferLeft))
		return -1;

	int iExtra=0;
	data = Decode(pack,data, &iExtra, &iBufferLeft);

	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	rManager.RecvAuctionExitFromAuction(ch, pack->dwOwnerID);
	return iExtra;
}

#ifdef __ENABLE_NEW_SHOP_IN_CITIES__
int OfflineshopPacketClickEntity(LPCHARACTER ch, const char* data, int iBufferLeft)
{
	offlineshop::TSubPacketCGShopClickEntity* pack = nullptr;
	if(!CanDecode(pack, iBufferLeft))
		return -1;

	int iExtra=0;
	data = Decode(pack, data, &iExtra, &iBufferLeft);

	offlineshop::CShopManager& rManager = offlineshop::GetManager();
	rManager.RecvShopClickEntity(ch, pack->dwShopVID);
	return iExtra;
}
#endif

int OfflineshopPacket(const char* data , LPCHARACTER ch, long iBufferLeft)
{
	if(iBufferLeft < (int) sizeof(TPacketCGNewOfflineShop))
		return -1;

	TPacketCGNewOfflineShop* pPack=nullptr;
	iBufferLeft -= sizeof(TPacketCGNewOfflineShop);
	data = Decode(pPack, data);

	switch (pPack->bSubHeader)
	{
		case offlineshop::SUBHEADER_CG_SHOP_CREATE_NEW:				return OfflineshopPacketCreateNewShop(ch,data,iBufferLeft);
		case offlineshop::SUBHEADER_CG_SHOP_CHANGE_NAME:			return OfflineshopPacketChangeShopName(ch,data,iBufferLeft);
		case offlineshop::SUBHEADER_CG_SHOP_FORCE_CLOSE:			return OfflineshopPacketForceCloseShop(ch,data,iBufferLeft);
		case offlineshop::SUBHEADER_CG_SHOP_REQUEST_SHOPLIST:		return OfflineshopPacketRequestShopList(ch,data,iBufferLeft);
		case offlineshop::SUBHEADER_CG_SHOP_OPEN:					return OfflineshopPacketOpenShop(ch,data,iBufferLeft);
		case offlineshop::SUBHEADER_CG_SHOP_OPEN_OWNER:				return OfflineshopPacketOpenShowOwner(ch,data,iBufferLeft);
		case offlineshop::SUBHEADER_CG_SHOP_BUY_ITEM:				return OfflineshopPacketBuyItem(ch, data , iBufferLeft);
		case offlineshop::SUBHEADER_CG_SHOP_ADD_ITEM:				return OfflineshopPacketAddItem(ch,data,iBufferLeft);
		case offlineshop::SUBHEADER_CG_SHOP_REMOVE_ITEM:			return OfflineshopPacketRemoveItem(ch,data,iBufferLeft);
		case offlineshop::SUBHEADER_CG_SHOP_EDIT_ITEM:				return OfflineshopPacketEditItem(ch,data,iBufferLeft);
		case offlineshop::SUBHEADER_CG_SHOP_FILTER_REQUEST:			return OfflineshopPacketFilterRequest(ch,data,iBufferLeft);
		case offlineshop::SUBHEADER_CG_SHOP_OFFER_CREATE:			return OfflineshopPacketCreateOffer(ch,data,iBufferLeft);
		case offlineshop::SUBHEADER_CG_SHOP_OFFER_ACCEPT:			return OfflineshopPacketAcceptOffer(ch,data,iBufferLeft);
		case offlineshop::SUBHEADER_CG_SHOP_REQUEST_OFFER_LIST:		return OfflineshopPacketOfferListRequest(ch);
		case offlineshop::SUBHEADER_CG_SHOP_OFFER_CANCEL:			return OfflineshopPacketOfferCancel(ch, data,iBufferLeft);
		case offlineshop::SUBHEADER_CG_SHOP_SAFEBOX_OPEN:			return OfflineshopPacketOpenSafebox(ch,data,iBufferLeft);
		case offlineshop::SUBHEADER_CG_SHOP_SAFEBOX_GET_ITEM:		return OfflineshopPacketGetItemSafebox(ch,data,iBufferLeft);
		case offlineshop::SUBHEADER_CG_SHOP_SAFEBOX_GET_VALUTES:	return OfflineshopPacketGetValutesSafebox(ch,data,iBufferLeft);
		case offlineshop::SUBHEADER_CG_SHOP_SAFEBOX_CLOSE:			return OfflineshopPacketCloseSafebox(ch,data,iBufferLeft);
		case offlineshop::SUBHEADER_CG_AUCTION_LIST_REQUEST:		return OfflineshopPacketListRequest(ch, data, iBufferLeft);
		case offlineshop::SUBHEADER_CG_AUCTION_OPEN_REQUEST:		return OfflineshopPacketOpenAuctionRequest(ch, data, iBufferLeft);
		case offlineshop::SUBHEADER_CG_MY_AUCTION_OPEN_REQUEST:		return OfflineshopPacketOpenMyAuctionRequest(ch, data, iBufferLeft);
		case offlineshop::SUBHEADER_CG_CREATE_AUCTION:				return OfflineshopPacketCreateAuction(ch, data, iBufferLeft);
		case offlineshop::SUBHEADER_CG_AUCTION_ADD_OFFER:			return OfflineshopPacketAddOffer(ch, data, iBufferLeft);
		case offlineshop::SUBHEADER_CG_EXIT_FROM_AUCTION:			return OfflineshopPacketExitFromAuction(ch, data, iBufferLeft);
		case offlineshop::SUBHEADER_CG_CLOSE_BOARD:					return OfflineshopPacketCloseBoard(ch);
#ifdef __ENABLE_NEW_SHOP_IN_CITIES__
		case offlineshop::SUBHEADER_CG_CLICK_ENTITY:				return OfflineshopPacketClickEntity(ch, data, iBufferLeft);
#endif

		default:
			sys_err("UNKNOWN SUBHEADER %d ",pPack->bSubHeader);
			return -1;
	}
}
#endif

3* Search for:
		case HEADER_CG_DRAGON_SOUL_REFINE:
			{
				TPacketCGDragonSoulRefine* p = reinterpret_cast <TPacketCGDragonSoulRefine*>((void*)c_pData);
				switch(p->bSubType)
				{
				case DS_SUB_HEADER_CLOSE:
					ch->DragonSoul_RefineWindow_Close();
					break;
				case DS_SUB_HEADER_DO_REFINE_GRADE:
					{
						DSManager::instance().DoRefineGrade(ch, p->ItemGrid);
					}
					break;
				case DS_SUB_HEADER_DO_REFINE_STEP:
					{
						DSManager::instance().DoRefineStep(ch, p->ItemGrid);
					}
					break;
				case DS_SUB_HEADER_DO_REFINE_STRENGTH:
					{
						DSManager::instance().DoRefineStrength(ch, p->ItemGrid);
					}
					break;
				}
			}
			break;

4* Add below:
#ifdef __ENABLE_NEW_OFFLINESHOP__
		case HEADER_CG_NEW_OFFLINESHOP:
			if((iExtraLen = OfflineshopPacket(c_pData, ch, m_iBufferLeft))< 0)
				return -1;
			break;
#endif