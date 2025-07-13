#include "stdafx.h"
#include "../../common/tables.h"
#include "packet.h"
#include "item.h"
#include "char.h"
#include "item_manager.h"
#include "desc.h"
#include "char_manager.h"
#include "banword.h"
#include "buffer_manager.h"
#include "desc_client.h"
#include "config.h"
#include "event.h"
#include "locale_service.h"
#include <fstream>
#include "sectree_manager.h"
#include "sectree.h"
#include "config.h"
#include "new_offlineshop.h"
#include "new_offlineshop_manager.h"
#include "log.h"

#define __ENABLE_OFFLINESHOP_GM_PROTECTION__
#ifdef __ENABLE_OFFLINESHOP_GM_PROTECTION__
	#define _IS_VALID_GM_LEVEL(ch) (ch->GetGMLevel() == GM_PLAYER || ch->GetGMLevel() == GM_WIZARD || ch->GetGMLevel() == GM_IMPLEMENTOR)
#else
#define _IS_VALID_GM_LEVEL(ch) (true)
#endif

#ifdef __ENABLE_NEW_OFFLINESHOP__
bool MatchWearFlag(DWORD dwWearFilter, DWORD dwWearTable)
{
	if(dwWearFilter==0)
	{
		return true;
	}

	static const DWORD flags[] = {
		ITEM_ANTIFLAG_MALE,
		ITEM_ANTIFLAG_FEMALE,
		ITEM_ANTIFLAG_WARRIOR,
		ITEM_ANTIFLAG_ASSASSIN,
		ITEM_ANTIFLAG_SURA,
		ITEM_ANTIFLAG_SHAMAN,
#ifdef ENABLE_WOLFMAN
		ITEM_ANTIFLAG_WOLFMAN,
#endif
	};

	const size_t counts = sizeof(flags)/sizeof(DWORD);

	for(size_t i=0; i < counts; i++)
		if(IS_SET(dwWearFilter, flags[i]) &&!IS_SET(dwWearTable, flags[i]))
			return false;
	return true;
}

bool MatchAttributes(const TPlayerItemAttribute* pAttributesFilter,const TPlayerItemAttribute* pAttributesItem)
{
	for (int i = 0; i < ITEM_ATTRIBUTE_NORM_NUM; i++)
	{
		if(pAttributesFilter[i].bType == 0)
			continue;

		bool bFound=false;

		BYTE type	= pAttributesFilter[i].bType;
		int  val	= pAttributesFilter[i].sValue;

		for (int i = 0; i < ITEM_ATTRIBUTE_NORM_NUM; i++)
		{
			if (pAttributesItem[i].bType == type)
			{
				bFound = pAttributesItem[i].sValue >= val;
				break;
			}
		}

		if(!bFound)
			return false;
	}

	return true;
}

std::string StringToLower(const char* name, size_t len)
{
	std::string res;
	res.resize(len);
	for(size_t i=0; i < len; i++)
		res[i] = tolower(*(name + i));
	return res;
}

bool MatchItemName(std::string stName, const char* table , const size_t tablelen)
{
	std::string stTable= StringToLower(table, tablelen);
	return stTable.find(stName) != std::string::npos;
}

bool CheckCharacterActions(LPCHARACTER ch)
{
	if(!ch)
		return false;

	if(ch->GetExchange())
		return false;

	if(ch->GetSafebox())
		return false;

	if(ch->GetMyShop())
		return false;

#if defined(ENABLE_ACCE_SYSTEM) || defined(ENABLE_ACCE_COSTUME_SYSTEM) || defined(ENABLE_SASH_SYSTEM) || defined(__SASH_SYSTEM) || defined(ACCE_SYSTEM) || defined(__ACCE_SYSTEM__) || defined(ENABLE_SHOULDER_SASH_SYSTEM)
	if(ch->IsUpgradingSash())
		return false;
#endif

	return true;
}

long long GetTotalAmountFromPrice(const offlineshop::TPriceInfo& price)
{
	long long total =0;
	total += price.illYang;
	return total;
}

bool CheckNewAuctionOfferPrice(const offlineshop::TPriceInfo& price, const offlineshop::TPriceInfo& best)
{
	long long totalValueIn =0, totalValueBest=0;

	totalValueIn	= GetTotalAmountFromPrice(price);
	totalValueBest	= GetTotalAmountFromPrice(best);

	if(totalValueBest< 1000)
	{
		totalValueBest += 1000;
	}
	else
	{
		const float percentage = (float)OFFLINESHOP_AUCTION_RAISE_PERCENTAGE/100.0;
		totalValueBest	+=(long long)(((long double)(totalValueBest))*percentage);
	}

	return totalValueIn >= totalValueBest;
}

namespace offlineshop
{
	EVENTINFO(offlineshopempty_info)
	{
		int empty;

		offlineshopempty_info()
			: empty(0)
		{
		}
	};

	EVENTFUNC(func_offlineshop_update_duration)
	{
		offlineshop::GetManager().UpdateShopsDuration();
		offlineshop::GetManager().UpdateAuctionsDuration();
		offlineshop::GetManager().ClearSearchTimeMap();
		offlineshop::GetManager().ClearOfferTimeMap();
		return OFFLINESHOP_DURATION_UPDATE_TIME;
	}

	offlineshop::CShopManager& GetManager()
	{
		return offlineshop::CShopManager::instance();
	}

#ifdef ENABLE_IRA_REWORK
	offlineshop::CShop * CShopManager::PutsNewShop(TShopInfo * pInfo, TShopPosition * pPosInfo)
#else
	offlineshop::CShop * CShopManager::PutsNewShop(TShopInfo * pInfo)
#endif
	{
		OFFSHOP_DEBUG("puts new shop %s ", pInfo->szName);

		SHOPMAP::iterator it = m_mapShops.insert(std::make_pair(pInfo->dwOwnerID, offlineshop::CShop())).first;
		offlineshop::CShop& rShop = it->second;

		rShop.SetDuration(pInfo->dwDuration);
		rShop.SetOwnerPID(pInfo->dwOwnerID);
		rShop.SetName(pInfo->szName);
#ifdef ENABLE_IRA_REWORK
		rShop.SetPosInfo(*pPosInfo);
#endif

#ifdef __ENABLE_NEW_SHOP_IN_CITIES__
#ifdef ENABLE_IRA_REWORK
		CreateNewShopEntities(rShop, *pPosInfo);
#else
		CreateNewShopEntities(rShop);
#endif
#endif
		return &rShop;
	}

	void CShopManager::PutsAuction(const TAuctionInfo& auction)
	{
		CAuction& obj = m_mapAuctions[auction.dwOwnerID];
		obj.SetInfo(auction);
		return;
	}

	void CShopManager::PutsAuctionOffer(const TAuctionOfferInfo& offer)
	{
		itertype(m_mapAuctions) it;
		if((it=m_mapAuctions.find(offer.dwOwnerID)) == m_mapAuctions.end())
			return;

		CAuction& obj = it->second;
		obj.AddOffer(offer);
	}

	offlineshop::CShop* CShopManager::GetShopByOwnerID(DWORD dwPID) 
	{
		SHOPMAP::iterator it=m_mapShops.find(dwPID);
		if(it == m_mapShops.end())
			return nullptr;

		return &(it->second);
	}

	offlineshop::CAuction* CShopManager::GetAuctionByOwnerID(DWORD dwPID)
	{
		AUCTIONMAP::iterator it=m_mapAuctions.find(dwPID);
		if(it == m_mapAuctions.end())
			return nullptr;

		return &(it->second);
	}

	void CShopManager::RemoveSafeboxFromCache(DWORD dwOwnerID)
	{
		SAFEBOXMAP::iterator it = m_mapSafeboxs.find(dwOwnerID);
		if(it==m_mapSafeboxs.end())
			return;

		m_mapSafeboxs.erase(it);
	}

	void CShopManager::RemoveGuestFromShops(LPCHARACTER ch)
	{
		if(ch->GetOfflineShopGuest())
			ch->GetOfflineShopGuest()->RemoveGuest(ch);

		ch->SetOfflineShopGuest(NULL);

		if(ch->GetOfflineShop())
			ch->GetOfflineShop()->RemoveGuest(ch);

		ch->SetOfflineShop(NULL);
	}

#ifdef ENABLE_IRA_REWORK
	int CShopManager::GetMapIndexAllowsList(int iMapIndex)
	{
		int index = 0;

		for(auto it = s_set_offlineshop_map_allows.begin(); it != s_set_offlineshop_map_allows.end(); it++)
		{
			if (*it == iMapIndex)
				return index;

			index++;
		}

		return -1;
	}
#endif

	CShopManager::CShopManager()
	{
		offlineshopempty_info* info = AllocEventInfo<offlineshopempty_info>();
		m_eventShopDuration = event_create(func_offlineshop_update_duration, info, OFFLINESHOP_DURATION_UPDATE_TIME);
#ifdef ENABLE_IRA_REWORK
		for (auto iMapIndex : { 1, 3, 41, 43 } )
			s_set_offlineshop_map_allows.insert(iMapIndex);

		m_vecCities.resize(s_set_offlineshop_map_allows.size());
#else
		m_vecCities.resize(Offlineshop_GetMapCount());
#endif
	}

	CShopManager::~CShopManager()
	{
		Destroy();
	}

	void CShopManager::Destroy()
	{
		if(m_eventShopDuration)
			event_cancel(&m_eventShopDuration);

		m_eventShopDuration = nullptr;

		m_mapOffer.clear();
		m_mapSafeboxs.clear();
		m_mapShops.clear();

#ifdef __ENABLE_NEW_SHOP_IN_CITIES__
		for (itertype(m_vecCities) itCities = m_vecCities.begin(); itCities != m_vecCities.end(); itCities++)
		{
			TCityShopInfo& city = *itCities;

			for (itertype(city.entitiesByPID) it = city.entitiesByPID.begin(); it != city.entitiesByPID.end(); it++)
				delete(it->second);

			city.entitiesByPID.clear();
			city.entitiesByVID.clear();
		}

		m_vecCities.clear();
#endif
	}

#ifdef __ENABLE_NEW_SHOP_IN_CITIES__
	bool IsEmptyString(const std::string& st)
	{
		return st.find_first_not_of(" \t\r\n") == std::string::npos;
	}

#ifndef ENABLE_IRA_REWORK
	bool CShopManager::__CanUseCity(size_t index)
	{
		int map_index=0;
		Offlineshop_GetMapIndex(index, &map_index);
		return SECTREE_MANAGER::instance().GetMap(map_index) != NULL;
	}

	bool CShopManager::__CheckEntitySpawnPos(const long x, const long y, const TCityShopInfo& city)
	{
		const SHOPENTITIES_MAP& entitiesMap = city.entitiesByPID;

		for (itertype(entitiesMap) it = entitiesMap.begin(); it != entitiesMap.end(); it++)
		{
			const ShopEntity& entity = *(it->second);
			const PIXEL_POSITION pos = entity.GetXYZ();

			if(!Offlineshop_CheckPositionDistance(pos.x, pos.y, x, y))
				return false;
		}

		return true;
	}
#endif

	void CShopManager::__UpdateEntity(const offlineshop::CShop& rShop)
	{
		itertype(m_vecCities) it = m_vecCities.begin();
		for (; it != m_vecCities.end(); it++)
		{
			itertype(it->entitiesByPID) itMap = it->entitiesByPID.find(rShop.GetOwnerPID());
			if(itMap == it->entitiesByPID.end())
				continue;

			ShopEntity& ent = *(itMap->second);
			ent.SetShopName(rShop.GetName());

			if (ent.GetSectree())
				ent.ViewReencode();
		}
	}

#ifdef ENABLE_IRA_REWORK
	void CShopManager::CreateNewShopEntities(offlineshop::CShop& rShop, TShopPosition& pos) 
	{ 
		if (pos.bChannel == g_bChannel) 
		{ 
			int cityIndex = GetMapIndexAllowsList(pos.lMapIndex); 
 
			if (cityIndex != -1 && cityIndex < m_vecCities.size()) 
			{ 
				long x = pos.x; 
				long y = pos.y; 
 
				LPSECTREE sectree = SECTREE_MANAGER::Instance().Get(pos.lMapIndex, x, y); 
 
				if (sectree) 
				{ 
					OFFSHOP_DEBUG("map ok! map_index %d (%d, %d)", pos.lMapIndex, x, y); 
					ShopEntity* pEntity = new ShopEntity(); 
					pEntity->SetShopName(rShop.GetName()); 
					pEntity->SetShopType(0); //TODO: add differents shop skins  
					pEntity->SetMapIndex(pos.lMapIndex); 
					pEntity->SetXYZ(x, y, 0); 
					pEntity->SetShop(&rShop); 
					sectree->InsertEntity(pEntity); 
					pEntity->UpdateSectree(); 
					m_vecCities[cityIndex].entitiesByPID.insert(std::make_pair(rShop.GetOwnerPID(), pEntity)); 
					m_vecCities[cityIndex].entitiesByVID.insert(std::make_pair(pEntity->GetVID(), pEntity)); 
				} 
				else 
				{ 
					OFFSHOP_DEBUG("map error! map_index %d (%d, %d)", pos.lMapIndex, pos.x, pos.y); 
				} 
			} 
		} 
	} 
#else
	void CShopManager::CreateNewShopEntities(offlineshop::CShop& rShop)
	{
	#define PI 3.14159265
	#define RADIANS_PER_DEGREE (PI/180.0)
	#define TORAD(a)	((a)*RADIANS_PER_DEGREE)
		int index = 0;
		itertype(m_vecCities) it = m_vecCities.begin();
		for (; it != m_vecCities.end(); it++, index++)
		{
			TCityShopInfo& city = *it;

			long shop_pos_x = 0, shop_pos_y = 0;
			int iCheckCount = 0;

			int map_index = 0;
			Offlineshop_GetMapIndex(index, &map_index);

			size_t ent_count = it->entitiesByPID.size();

			do {
				Offlineshop_GetNewPos(index, ent_count, &shop_pos_x, &shop_pos_y);

			} while (!__CheckEntitySpawnPos(shop_pos_x, shop_pos_y, city) && iCheckCount++ < 10);

			LPSECTREE sectree = SECTREE_MANAGER::Instance().Get(map_index, shop_pos_x, shop_pos_y);

			if (sectree)
			{
				ShopEntity* pEntity = new ShopEntity();

				pEntity->SetShopName(rShop.GetName());
				pEntity->SetShopType(0);
				pEntity->SetMapIndex(map_index);
				pEntity->SetXYZ(shop_pos_x, shop_pos_y, 0);
				pEntity->SetShop(&rShop);

				sectree->InsertEntity(pEntity);
				pEntity->UpdateSectree();

				city.entitiesByPID.insert(std::make_pair(rShop.GetOwnerPID(), pEntity));
				city.entitiesByVID.insert(std::make_pair(pEntity->GetVID(), pEntity));
			}
		}
	}
#endif

	void CShopManager::DestroyNewShopEntities(const offlineshop::CShop& rShop)
	{
		itertype(m_vecCities) it = m_vecCities.begin();
		for (; it != m_vecCities.end(); it++)
		{
			TCityShopInfo& city = *it;

			itertype(city.entitiesByPID) iter = city.entitiesByPID.find(rShop.GetOwnerPID());

			if (iter == city.entitiesByPID.end())
			{
				//sys_err("CANNOT FOUND NEW SHOP ENTITY : %u ",rShop.GetOwnerPID());
				continue;
			}

			ShopEntity* entity = iter->second;
			DWORD dwVID = entity->GetVID();

			if (entity->GetSectree())
			{
				entity->ViewCleanup();
				entity->GetSectree()->RemoveEntity(entity);
			}

#ifdef ENABLE_IRA_REWORK
			entity->Destroy();
#endif
			delete(entity);
			city.entitiesByPID.erase(iter);
			city.entitiesByVID.erase(city.entitiesByVID.find(dwVID));
		}
	}

	void CShopManager::EncodeInsertShopEntity(ShopEntity& shop, LPCHARACTER ch)
	{
		if(!ch->GetDesc())
			return;

		TPacketGCNewOfflineshop pack;
		pack.bHeader	= HEADER_GC_NEW_OFFLINESHOP;
		pack.bSubHeader	= SUBHEADER_GC_INSERT_SHOP_ENTITY;
		pack.wSize		= sizeof(pack)+ sizeof(TSubPacketGCInsertShopEntity);

		const PIXEL_POSITION pos = shop.GetXYZ();

		TSubPacketGCInsertShopEntity subpack;
		subpack.dwVID = shop.GetVID();
		subpack.iType = shop.GetShopType();
		
		subpack.x = pos.x;
		subpack.y = pos.y;
		subpack.z = pos.z;

		strncpy(subpack.szName, shop.GetShopName(), sizeof(subpack.szName));

		ch->GetDesc()->BufferedPacket(&pack, sizeof(pack));
		ch->GetDesc()->Packet(&subpack, sizeof(subpack));
	}

	void CShopManager::EncodeRemoveShopEntity(ShopEntity& shop, LPCHARACTER ch)
	{
		if(!ch->GetDesc())
			return;

		TPacketGCNewOfflineshop pack;
		pack.bHeader	= HEADER_GC_NEW_OFFLINESHOP;
		pack.bSubHeader	= SUBHEADER_GC_REMOVE_SHOP_ENTITY;
		pack.wSize		= sizeof(pack)+ sizeof(TSubPacketGCRemoveShopEntity);

		TSubPacketGCRemoveShopEntity subpack;
		subpack.dwVID = shop.GetVID();

		ch->GetDesc()->BufferedPacket(&pack, sizeof(pack));
		ch->GetDesc()->Packet(&subpack, sizeof(subpack));
	}
#endif

	CShopSafebox* CShopManager::GetShopSafeboxByOwnerID(DWORD dwPID)
	{
		SAFEBOXMAP::iterator it = m_mapSafeboxs.find(dwPID);
		if(it == m_mapSafeboxs.end())
			return nullptr;
		return &(it->second);
	}

	bool CShopManager::PutsNewOffer(const TOfferInfo* pInfo)
	{
		OFFERSMAP::iterator it= m_mapOffer.find(pInfo->dwOffererID);

		if (it == m_mapOffer.end())
		{
			it = m_mapOffer.insert(std::make_pair(pInfo->dwOffererID, std::vector<TOfferInfo>())).first;
		}
		else
		{
			itertype(it->second) itVec = it->second.begin();
			for (; itVec != it->second.end(); itVec++)
			{
				if(itVec->dwOfferID == pInfo->dwOfferID)
					return false;
			}
		}

		it->second.push_back(*pInfo);
		return true;
	}

	void CShopManager::SendShopBuyDBPacket(DWORD dwBuyerID, DWORD dwOwnerID, DWORD dwItemID)
	{
		TPacketGDNewOfflineShop pack;
		pack.bSubHeader	= SUBHEADER_GD_BUY_ITEM;

		TSubPacketGDBuyItem subpack;
		subpack.dwGuestID	= dwBuyerID;
		subpack.dwOwnerID	= dwOwnerID;
		subpack.dwItemID	= dwItemID;

		TEMP_BUFFER buff;
		buff.write(&pack, sizeof(pack));
		buff.write(&subpack, sizeof(subpack));

		OFFSHOP_DEBUG("sending for shop %u and item %u (buyer %u) ",dwOwnerID, dwItemID, dwBuyerID);
#ifdef ENABLE_NEW_OFFLINESHOP_LOGS
		LogManager::instance().OfflineshopLog(dwOwnerID, dwItemID, "%u is buying the item", dwBuyerID);
#endif
		db_clientdesc->DBPacket(HEADER_GD_NEW_OFFLINESHOP, 0, buff.read_peek(), buff.size());
	}

	bool CShopManager::RecvShopBuyDBPacket(DWORD dwBuyerID, DWORD dwOwnerID,DWORD dwItemID)
	{
		OFFSHOP_DEBUG("buyer %u , owner %u , itemid %u ",dwBuyerID, dwOwnerID, dwItemID);

		CShop* pkShop = GetShopByOwnerID(dwOwnerID);
		if(!pkShop)
			return false;

		CShopItem* pItem = nullptr;
		if(!pkShop->GetItem(dwItemID, &pItem))
			return false;

		OFFSHOP_DEBUG("checked %s" , "successful");

		LPCHARACTER ch = CHARACTER_MANAGER::instance().FindByPID(dwBuyerID);
		LPCHARACTER owner = CHARACTER_MANAGER::instance().FindByPID(dwOwnerID);

		if (ch)
		{
			OFFSHOP_DEBUG("buyer is online , name %s , item id %u ",ch->GetName(), dwItemID);

			LPITEM pkItem = pItem->CreateItem();
			if (!pkItem)
			{
				sys_err("cannot create item ( dwItemID %u , dwVnum %u, dwShopOwner %u, dwBuyer %u ) ",dwItemID, pItem->GetInfo()->dwVnum, dwOwnerID, dwBuyerID );
				return false;
			}

			if (owner)
			{
				owner->ChatPacket(CHAT_TYPE_COMMAND, "OfflineShop_NotifySoldItem %s %u", ch->GetName(), pItem->GetInfo()->dwVnum);
			}

			TItemPos pos;
			if (!ch->CanTakeInventoryItem(pkItem, &pos))
			{
				M2_DESTROY_ITEM(pkItem);

				CShopSafebox* pSafebox = ch->GetShopSafebox()? ch->GetShopSafebox() : GetShopSafeboxByOwnerID(ch->GetPlayerID());
				if (!pSafebox)
					return false;

				SendShopSafeboxAddItemDBPacket(ch->GetPlayerID(), *pItem);
				SendChatPacket(ch, CHAT_PACKET_RECV_ITEM_SAFEBOX);
			}

			else
			{
				pkItem->AddToCharacter(ch, pos);
			}
			
			DWORD dwItemID = pItem->GetID();
			pkShop->BuyItem(dwItemID);
		}
		else
		{
			OFFSHOP_DEBUG("buyer isn't online , item removed %u (shop %u)",dwItemID, pkShop->GetOwnerPID());

			DWORD dwItemID = pItem->GetID();
			pkShop->BuyItem(dwItemID);
		}

		return true;
	}

void CShopManager::SendShopEditItemDBPacket(DWORD dwOwnerID, DWORD dwItemID, const TPriceInfo& rPrice)
	{
		TPacketGDNewOfflineShop pack;
		pack.bSubHeader	= SUBHEADER_GD_EDIT_ITEM;

		TSubPacketGDEditItem subpack;
		subpack.dwOwnerID	= dwOwnerID;
		subpack.dwItemID	= dwItemID;
		CopyObject(subpack.priceInfo , rPrice);

		TEMP_BUFFER buff;
		buff.write(&pack, sizeof(pack));
		buff.write(&subpack, sizeof(subpack));

		OFFSHOP_DEBUG("item id %u, owner shop %u",dwItemID, dwOwnerID);
#ifdef ENABLE_NEW_OFFLINESHOP_LOGS
		LogManager::instance().OfflineshopLog(dwOwnerID, dwItemID, "change the price of item to %lld yang "
#ifdef __ENABLE_CHEQUE_SYSTEM__
			" and %d cheque "
#endif
			, rPrice.illYang
#ifdef __ENABLE_CHEQUE_SYSTEM__
			, pPrice.iCheque
#endif
		);
#endif
		db_clientdesc->DBPacket(HEADER_GD_NEW_OFFLINESHOP, 0, buff.read_peek(), buff.size());
	}

	bool CShopManager::RecvShopEditItemDBPacket(DWORD dwOwnerID, DWORD dwItemID, const TPriceInfo& rPrice)
	{
		CShop* pkShop = GetShopByOwnerID(dwOwnerID);
		if(!pkShop)
			return false;

		CShopItem* pItem = nullptr;
		if(!pkShop->GetItem(dwItemID, &pItem))
			return false;

		OFFSHOP_DEBUG("owner id %u , item id %u ",dwOwnerID , dwItemID);

		CShopItem newItem(*pItem);
		newItem.SetPrice(rPrice);

		pkShop->ModifyItem(dwItemID,newItem);
		return true;
	}

void CShopManager::SendShopRemoveItemDBPacket(DWORD dwOwnerID, DWORD dwItemID)
	{
		TPacketGDNewOfflineShop pack;
		pack.bSubHeader	= SUBHEADER_GD_REMOVE_ITEM;

		TSubPacketGDRemoveItem subpack;
		subpack.dwOwnerID	= dwOwnerID;
		subpack.dwItemID	= dwItemID;

		TEMP_BUFFER buff;
		buff.write(&pack, sizeof(pack));
		buff.write(&subpack, sizeof(subpack));

		OFFSHOP_DEBUG("owner %u , item %u ",dwOwnerID, dwItemID);
#ifdef ENABLE_NEW_OFFLINESHOP_LOGS
		LogManager::instance().OfflineshopLog(dwOwnerID, dwItemID, "the item is removed");
#endif
		db_clientdesc->DBPacket(HEADER_GD_NEW_OFFLINESHOP, 0, buff.read_peek(), buff.size());
	}

	bool CShopManager::RecvShopRemoveItemDBPacket(DWORD dwOwnerID, DWORD dwItemID)
	{
		CShop* pkShop = GetShopByOwnerID(dwOwnerID);
		if(!pkShop)
			return false;

		OFFSHOP_DEBUG("owner %u , item %u", dwOwnerID, dwItemID);
		return pkShop->RemoveItem(dwItemID);
	}

	void CShopManager::SendShopAddItemDBPacket(DWORD dwOwnerID, const TItemInfo& rItemInfo)
	{
		TPacketGDNewOfflineShop pack;
		pack.bSubHeader	= SUBHEADER_GD_ADD_ITEM;

		TSubPacketGDAddItem subpack;
		subpack.dwOwnerID	= dwOwnerID;
		CopyObject(subpack.itemInfo, rItemInfo);
		
		TEMP_BUFFER buff;
		buff.write(&pack, sizeof(pack));
		buff.write(&subpack, sizeof(subpack));

		OFFSHOP_DEBUG("owner %u , item vnum %u , item count %u ",dwOwnerID, rItemInfo.item.dwVnum , rItemInfo.item.dwCount);
		db_clientdesc->DBPacket(HEADER_GD_NEW_OFFLINESHOP, 0, buff.read_peek(), buff.size());
	}

	bool CShopManager::RecvShopAddItemDBPacket(DWORD dwOwnerID, const TItemInfo& rItemInfo)
	{
		CShop* pkShop = GetShopByOwnerID(dwOwnerID);
		if(!pkShop)
			return false;

		CShopItem newItem(rItemInfo.dwItemID);
		newItem.SetInfo(rItemInfo.item);
		newItem.SetPrice(rItemInfo.price);
		newItem.SetOwnerID(rItemInfo.dwOwnerID);

		OFFSHOP_DEBUG("owner %u , item id %u ",dwOwnerID, rItemInfo.dwItemID);
		return pkShop->AddItem(newItem);
	}

	void CShopManager::SendShopForceCloseDBPacket(DWORD dwPID)
	{
		TPacketGDNewOfflineShop pack;
		pack.bSubHeader	= SUBHEADER_GD_SHOP_FORCE_CLOSE;

		TSubPacketGDShopForceClose subpack;
		subpack.dwOwnerID = dwPID;

		TEMP_BUFFER buff;
		buff.write(&pack, sizeof(pack));
		buff.write(&subpack, sizeof(subpack));

		OFFSHOP_DEBUG("shop %u ",dwPID);
		db_clientdesc->DBPacket(HEADER_GD_NEW_OFFLINESHOP, 0, buff.read_peek(), buff.size());
	}

	bool CShopManager::RecvShopForceCloseDBPacket(DWORD dwPID)
	{
		CShop* pkShop = GetShopByOwnerID(dwPID);
		if(!pkShop)
			return false;

		LPCHARACTER ch = CHARACTER_MANAGER::instance().FindByPID(dwPID);
		
		if (ch)
			ch->SetOfflineShop(NULL);
		
		CShop::LISTGUEST* guests = pkShop->GetGuests();
		
		for (CShop::LISTGUEST::iterator it = guests->begin(); it != guests->end(); it++)
		{
			LPCHARACTER chGuest= GUEST_PTR(*it);
			if(!chGuest)
				continue;

			if(ch && ch == chGuest)
				SendShopOpenMyShopNoShopClientPacket(chGuest);
			else
				SendShopListClientPacket(chGuest);

			chGuest->SetOfflineShopGuest(NULL);
		}

		std::set<DWORD> setPids;

		CShop::VECSHOPOFFER& vec = *pkShop->GetOffers();
		for (DWORD i = 0; i < vec.size(); i++)
		{
			TOfferInfo& offer = vec[i];
			DWORD buyer = offer.dwOffererID;

			itertype(m_mapOffer) itOffer = m_mapOffer.find(buyer);
			if (itOffer != m_mapOffer.end())
			{
				CShop::VECSHOPOFFER& buyerOffers = itOffer->second;
				for (itertype(buyerOffers) itBuyer = buyerOffers.begin(); itBuyer != buyerOffers.end(); itBuyer++)
				{
					if (itBuyer->dwOfferID == offer.dwOfferID)
					{
						buyerOffers.erase(itBuyer);
						setPids.insert(buyer);
						break;
					}
				}
			}
		}

		for (itertype(setPids) itpid = setPids.begin(); itpid != setPids.end(); itpid++)
		{
			LPCHARACTER chBuyer = CHARACTER_MANAGER::instance().FindByPID(*itpid);
			if(chBuyer)
				RecvOfferListRequestPacket(chBuyer);
		}

#ifdef __ENABLE_NEW_SHOP_IN_CITIES__
		DestroyNewShopEntities(*pkShop);
#endif
		pkShop->Clear();

		m_mapShops.erase(m_mapShops.find(pkShop->GetOwnerPID()));
		return true;
	}

	void CShopManager::SendShopLockBuyItemDBPacket(DWORD dwBuyerID, DWORD dwOwnerID, DWORD dwItemID)
	{
		TPacketGDNewOfflineShop pack;
		pack.bSubHeader = SUBHEADER_GD_BUY_LOCK_ITEM;

		TSubPacketGDLockBuyItem subpack;
		subpack.dwGuestID = dwBuyerID;
		subpack.dwOwnerID = dwOwnerID;
		subpack.dwItemID  = dwItemID;

		TEMP_BUFFER buff;
		buff.write(&pack,	 sizeof(pack));
		buff.write(&subpack, sizeof(subpack));

		OFFSHOP_DEBUG("shop %u , buyer %u , item %u (size %u) ",dwOwnerID, dwBuyerID, dwItemID, buff.size());
		db_clientdesc->DBPacket(HEADER_GD_NEW_OFFLINESHOP, 0, buff.read_peek(), buff.size());
	}

	bool CShopManager::RecvShopLockedBuyItemDBPacket(DWORD dwBuyerID, DWORD dwOwnerID,DWORD dwItemID)
	{
		CShop* pkShop	= GetShopByOwnerID(dwOwnerID);
		LPCHARACTER ch	= CHARACTER_MANAGER::instance().FindByPID(dwBuyerID);

		if(!ch || !pkShop)
			return false;

		OFFSHOP_DEBUG("found shop %u ",dwBuyerID);

		CShopItem* pkItem = nullptr;
		if(!pkShop->GetItem(dwItemID, &pkItem))
			return false;
		
		OFFSHOP_DEBUG("found item %u",dwItemID);

		if(!pkItem->CanBuy(ch))
			return false;

		OFFSHOP_DEBUG("can buy %u",dwItemID);

		TPriceInfo* pPrice = pkItem->GetPrice();
		ch->ChangeGold(-pPrice->illYang);
#ifdef __ENABLE_CHEQUE_SYSTEM__
		ch->PointChange(POINT_CHEQUE, -pPrice->iCheque);
#endif

		const TItemTable* itemTable = ITEM_MANAGER::instance().GetTable(pkItem->GetInfo()->dwVnum);
		if (itemTable)
		{
			char logBuf[256]; 
			LPCHARACTER chShopOwner = CHARACTER_MANAGER::instance().FindByPID(dwBuyerID);
			if (chShopOwner)
			{
				snprintf(logBuf, sizeof(logBuf), "ShopOwner: %s Name: %s, Count: %d, Price: %lld", chShopOwner->GetName(), itemTable->szLocaleName, pkItem->GetInfo()->dwCount, (long long) pPrice->illYang); 
			}
			else
			{
				snprintf(logBuf, sizeof(logBuf), "ShopOwner: %d Name: %s, Count: %d, Price: %lld", dwBuyerID, itemTable->szLocaleName, pkItem->GetInfo()->dwCount, (long long) pPrice->illYang); 			
			}
			LogManager::Instance().ItemLog(ch, (int) pkItem->GetID(), pkItem->GetInfo()->dwVnum, "OFFLINESHOP_BUY_ITEM", logBuf);
		}
		
		SendShopBuyDBPacket(dwBuyerID, dwOwnerID, dwItemID);
		return true;
	}

	void CShopManager::SendShopCannotBuyLockedItemDBPacket(DWORD dwOwnerID, DWORD dwItemID)
	{
		TPacketGDNewOfflineShop pack;
		pack.bSubHeader = SUBHEADER_GD_CANNOT_BUY_LOCK_ITEM;

		TSubPacketGDCannotBuyLockItem subpack;
		subpack.dwOwnerID = dwOwnerID;
		subpack.dwItemID = dwItemID;

		TEMP_BUFFER buff;
		buff.write(&pack, sizeof(pack));
		buff.write(&subpack, sizeof(subpack));

		OFFSHOP_DEBUG("item id %u, owner shop %u", dwItemID, dwOwnerID);
		db_clientdesc->DBPacket(HEADER_GD_NEW_OFFLINESHOP, 0, buff.read_peek(), buff.size());
	}

	bool CShopManager::RecvShopExpiredDBPacket(DWORD dwPID) 
	{
		CShop* pkShop = GetShopByOwnerID(dwPID);
		if (!pkShop)
			return false;

		LPCHARACTER ch = CHARACTER_MANAGER::instance().FindByPID(dwPID);

		if (ch)
			ch->SetOfflineShop(NULL);


		//*getting the guest list before to remove the shop
		//*that is necessary to send the shop list packets
		CShop::LISTGUEST guests = *pkShop->GetGuests();


		std::set<DWORD> setPids;

		//offers check
		CShop::VECSHOPOFFER& vec = *pkShop->GetOffers();
		for (DWORD i = 0; i < vec.size(); i++)
		{
			//for each offer removing from buyer
			TOfferInfo& offer = vec[i];
			DWORD buyer = offer.dwOffererID;

			//searching buyer into map
			itertype(m_mapOffer) itOffer = m_mapOffer.find(buyer);
			if (itOffer != m_mapOffer.end())
			{
				//searching offer id in vec
				CShop::VECSHOPOFFER& buyerOffers = itOffer->second;
				for (itertype(buyerOffers) itBuyer = buyerOffers.begin(); itBuyer != buyerOffers.end(); itBuyer++)
				{
					if (itBuyer->dwOfferID == offer.dwOfferID)
					{
						buyerOffers.erase(itBuyer);
						setPids.insert(buyer);
						break;
					}
				}
			}
		}

		for (itertype(setPids) itpid = setPids.begin(); itpid != setPids.end(); itpid++)
		{
			LPCHARACTER chBuyer = CHARACTER_MANAGER::instance().FindByPID(*itpid);
			if (chBuyer)
				RecvOfferListRequestPacket(chBuyer);
		}



#ifdef __ENABLE_NEW_SHOP_IN_CITIES__
		DestroyNewShopEntities(*pkShop);
#endif
		pkShop->Clear();
		m_mapShops.erase(m_mapShops.find(pkShop->GetOwnerPID()));


		for (CShop::LISTGUEST::iterator it = guests.begin(); it != guests.end(); it++)
		{
			LPCHARACTER chGuest = GUEST_PTR(*it);
			if(!chGuest)
				continue;

			if (ch && ch == chGuest)
				SendShopOpenMyShopNoShopClientPacket(chGuest);
			else
				SendShopListClientPacket(chGuest);

			chGuest->SetOfflineShopGuest(NULL);
		}

		return true;
	}
	
#ifdef ENABLE_IRA_REWORK
	void CShopManager::SendShopCreateNewDBPacket(const TShopInfo& shop, const TShopPosition& pos, std::vector<TItemInfo>& vec)
#else
	void CShopManager::SendShopCreateNewDBPacket(const TShopInfo& shop, std::vector<TItemInfo>& vec)
#endif
	{
		TPacketGDNewOfflineShop pack;
		pack.bSubHeader	= SUBHEADER_GD_SHOP_CREATE_NEW;

		TSubPacketGDShopCreateNew subpack;
		CopyObject(subpack.shop, shop);
#ifdef ENABLE_IRA_REWORK
		CopyObject(subpack.pos, pos);
#endif
		TEMP_BUFFER buff;
		buff.write(&pack, sizeof(pack));
		buff.write(&subpack, sizeof(subpack));

		for(DWORD i =0; i<vec.size(); i++)
			buff.write(&vec[i], sizeof(TItemInfo));

		db_clientdesc->DBPacket(HEADER_GD_NEW_OFFLINESHOP, 0, buff.read_peek(), buff.size());
	}

#ifdef ENABLE_IRA_REWORK
	bool CShopManager::RecvShopCreateNewDBPacket(const TShopInfo& shop, TShopPosition& pos, std::vector<TItemInfo>& vec)
#else
	bool CShopManager::RecvShopCreateNewDBPacket(const TShopInfo& shop, std::vector<TItemInfo>& vec)
#endif
	{
		OFFSHOP_DEBUG("shop %s , shop id %u ", shop.szName, shop.dwOwnerID);

		if(m_mapShops.find(shop.dwOwnerID)!= m_mapShops.end())
			return false;

		CShop newShop;
		newShop.SetOwnerPID(shop.dwOwnerID);
		newShop.SetDuration(shop.dwDuration);
		newShop.SetName(shop.szName);

		std::vector<CShopItem> items;
		items.reserve(vec.size());

		for (DWORD i = 0; i < vec.size(); i++)
		{
			const TItemInfo& rItem = vec[i];

			CShopItem shopItem(rItem.dwItemID);

			shopItem.SetOwnerID(rItem.dwOwnerID);
			shopItem.SetPrice(rItem.price);
			shopItem.SetInfo(rItem.item);

			OFFSHOP_DEBUG("item id %u , item vnum %u , item count %u ",rItem.dwItemID, rItem.item.dwVnum , rItem.item.dwCount);

			items.push_back(shopItem);
		}

		newShop.SetItems(&items);

		OFFSHOP_DEBUG("shop %s , shop id %u inserted into map (items count %d)", shop.szName, shop.dwOwnerID, shop.dwCount);
		SHOPMAP::iterator it = m_mapShops.insert(std::make_pair(newShop.GetOwnerPID(), newShop)).first;

#ifdef __ENABLE_NEW_SHOP_IN_CITIES__
#ifdef ENABLE_IRA_REWORK
		CreateNewShopEntities(it->second, pos);
#else
		CreateNewShopEntities(it->second);
#endif
#endif

		LPCHARACTER chOwner = it->second.FindOwnerCharacter();
		if (chOwner)
		{
			chOwner->SetOfflineShop(&(it->second));
			chOwner->SetOfflineShopGuest(&(it->second));

			it->second.AddGuest(chOwner);
			SendShopOpenMyShopClientPacket(chOwner);
		}

		return true;
	}

	void CShopManager::SendShopChangeNameDBPacket(DWORD dwOwnerID, const char* szName)
	{
		TPacketGDNewOfflineShop pack;
		pack.bSubHeader	= SUBHEADER_GD_SHOP_CHANGE_NAME;

		TSubPacketGDShopChangeName subpack;
		subpack.dwOwnerID	= dwOwnerID;
		strncpy(subpack.szName, szName, sizeof(subpack.szName));

		TEMP_BUFFER buff;
		buff.write(&pack, sizeof(pack));
		buff.write(&subpack, sizeof(subpack));

		OFFSHOP_DEBUG("shop id %u , name [%s]",dwOwnerID, szName);
		db_clientdesc->DBPacket(HEADER_GD_NEW_OFFLINESHOP, 0, buff.read_peek(), buff.size());
	}

	bool CShopManager::RecvShopChangeNameDBPacket(DWORD dwOwnerID, const char* szName)
	{
		CShop* pkShop = GetShopByOwnerID(dwOwnerID);
		if(!pkShop)
			return false;

		pkShop->SetName(szName);
		pkShop->RefreshToOwner();

#ifdef __ENABLE_NEW_SHOP_IN_CITIES__
		__UpdateEntity(*pkShop);
#endif

		OFFSHOP_DEBUG("id %u , name %s ",dwOwnerID, szName);
		return true;
	}

	void CShopManager::SendShopOfferNewDBPacket(const TOfferInfo& offer)
	{
		TPacketGDNewOfflineShop pack;
		pack.bSubHeader	= SUBHEADER_GD_OFFER_CREATE;

		TSubPacketGDOfferCreate subpack;
		subpack.dwOwnerID	= offer.dwOwnerID;
		subpack.dwItemID	= offer.dwItemID;
		CopyObject(subpack.offer, offer);

		TEMP_BUFFER buff;
		buff.write(&pack, sizeof(pack));
		buff.write(&subpack, sizeof(subpack));

		OFFSHOP_DEBUG("offerer %u , shop %u ",offer.dwOffererID , offer.dwOwnerID);
		db_clientdesc->DBPacket(HEADER_GD_NEW_OFFLINESHOP, 0, buff.read_peek(), buff.size());
	}

	bool CShopManager::RecvShopOfferNewDBPacket(const TOfferInfo& offer)
	{
		CShop* pkShop = GetShopByOwnerID(offer.dwOwnerID);
		if(!pkShop)
			return false;

		OFFSHOP_DEBUG("offerer %u , shop %u ", offer.dwOffererID , offer.dwOwnerID);
		if(!pkShop->AddOffer(&offer))
			return false;

		if(!PutsNewOffer(&offer))
			return false;

		LPCHARACTER ch = CHARACTER_MANAGER::instance().FindByPID(offer.dwOffererID);
		if(ch)
			SendChatPacket(ch, CHAT_PACKET_OFFER_CREATE);

		return true;
	}

	void CShopManager::SendShopOfferNotifiedDBPacket(DWORD dwOfferID, DWORD dwOwnerID)
	{
		TPacketGDNewOfflineShop pack;
		pack.bSubHeader	= SUBHEADER_GD_OFFER_NOTIFIED;

		TSubPacketGDOfferNotified subpack;
		subpack.dwOfferID	= dwOfferID;
		subpack.dwOwnerID	= dwOwnerID;

		TEMP_BUFFER buff;
		buff.write(&pack, sizeof(pack));
		buff.write(&subpack, sizeof(subpack));

		db_clientdesc->DBPacket(HEADER_GD_NEW_OFFLINESHOP, 0, buff.read_peek(), buff.size());
	}

	bool CShopManager::RecvShopOfferNotifiedDBPacket(DWORD dwOfferID, DWORD dwOwnerID)
	{
		CShop* pkShop = GetShopByOwnerID(dwOwnerID);
		if(!pkShop)
			return false;

		DWORD dwBuyer=0;

		CShop::VECSHOPOFFER* vec = pkShop->GetOffers();
		for (DWORD i = 0; i < vec->size(); i++)
		{
			TOfferInfo& offer = vec->at(i);
			if (offer.dwOfferID == dwOfferID)
			{
				OFFSHOP_DEBUG("notified offer successful %u , %u ",dwOfferID, dwOwnerID);

				offer.bNoticed = true;
				dwBuyer = offer.dwOffererID;
				break;
			}
		}

		if(dwBuyer==0)
			return false;

		OFFSHOP_DEBUG("searching dwBuyer %u in map",dwBuyer);

		itertype(m_mapOffer) it = m_mapOffer.find(dwBuyer);
		if(it==m_mapOffer.end())
			return false;
		
		OFFSHOP_DEBUG("found buyer successful");

		CShop::VECSHOPOFFER& vecBuyer = it->second;

		for (itertype(vecBuyer) itVec = vecBuyer.begin(); itVec != vecBuyer.end(); itVec++)
		{
			if(itVec->dwOfferID!=dwOfferID)
				continue;

			OFFSHOP_DEBUG("found offer successful");
			itVec->bNoticed=true;
			break;
		}

		return true;
	}

	void CShopManager::SendShopOfferAcceptDBPacket(const TOfferInfo& offer)
	{
		TPacketGDNewOfflineShop pack;
		pack.bSubHeader	= SUBHEADER_GD_OFFER_ACCEPT;

		TSubPacketGDOfferNotified subpack;
		subpack.dwOwnerID	= offer.dwOwnerID;
		subpack.dwOfferID	= offer.dwOfferID;

		TEMP_BUFFER buff;
		buff.write(&pack, sizeof(pack));
		buff.write(&subpack, sizeof(subpack));

		db_clientdesc->DBPacket(HEADER_GD_NEW_OFFLINESHOP, 0, buff.read_peek(), buff.size());
	}

	void CShopManager::SendShopOfferCancelDBPacket(const TOfferInfo& offer)
	{
		TPacketGDNewOfflineShop pack;
		pack.bSubHeader	= SUBHEADER_GD_OFFER_CANCEL;

		TSubPacketGDOfferCancel subpack;
		subpack.dwOwnerID	= offer.dwOwnerID;
		subpack.dwOfferID	= offer.dwOfferID;

		TEMP_BUFFER buff;
		buff.write(&pack, sizeof(pack));
		buff.write(&subpack, sizeof(subpack));

		db_clientdesc->DBPacket(HEADER_GD_NEW_OFFLINESHOP, 0, buff.read_peek(), buff.size());
	}

	bool CShopManager::RecvShopOfferCancelDBPacket(DWORD dwOfferID, DWORD dwOwnerID, bool isRemovingItem)
	{
		OFFSHOP_DEBUG("dwOfferID : %u , dwOwnerID %u ",dwOfferID, dwOwnerID);

		CShop* pkShop = GetShopByOwnerID(dwOwnerID);
		if(!pkShop)
			return false;

		CShop::VECSHOPOFFER& vecOffers		= *pkShop->GetOffers();
		CShop::VECSHOPOFFER::iterator it	= vecOffers.begin();

		TOfferInfo * pInfo=nullptr;

		for (; it != vecOffers.end(); it++)
		{
			if (it->dwOfferID == dwOfferID)
			{
				pInfo = &(*it);
				break;
			}
		}

		if(!pInfo)
			return false;

		OFFSHOP_DEBUG("found offer successful : %u ",dwOfferID);

		DWORD dwBuyerID = pInfo->dwOffererID;
		vecOffers.erase(it);

		itertype(m_mapOffer) iter = m_mapOffer.find(dwBuyerID);
		if (iter != m_mapOffer.end())
		{
			OFFSHOP_DEBUG("removing offer from offer vector by buyer %u ",dwBuyerID);

			std::vector<TOfferInfo>& vec = iter->second;
			itertype(vec) iterVec= vec.begin();

			for (; iterVec != vec.end(); iterVec++)
			{
				if (iterVec->dwOfferID == dwOfferID)
				{
					vec.erase(iterVec);
					break;
				}
			}
		}

		if(iter->second.empty())
			m_mapOffer.erase(iter);

		if (!isRemovingItem)
		{
			LPCHARACTER chOwner = CHARACTER_MANAGER::Instance().FindByPID(dwOwnerID);
			if(chOwner && chOwner->GetOfflineShopGuest() && chOwner->GetOfflineShopGuest()==chOwner->GetOfflineShop())
				SendShopOpenMyShopClientPacket(chOwner);
		}

		LPCHARACTER chBuyer = CHARACTER_MANAGER::Instance().FindByPID(dwBuyerID);
		if (chBuyer && chBuyer->IsLookingOfflineshopOfferList())
			RecvOfferListRequestPacket(chBuyer);

		return true;
	}

	bool CShopManager::RecvShopOfferAcceptDBPacket(DWORD dwOfferID, DWORD dwOwnerID)
	{
		CShop* pkShop = GetShopByOwnerID(dwOwnerID);
		if(!pkShop)
			return false;

		CShop::VECSHOPOFFER& vecOffers		= *pkShop->GetOffers();
		CShop::VECSHOPOFFER::iterator it	= vecOffers.begin();

		TOfferInfo * pInfo=nullptr;

		for (; it != vecOffers.end(); it++)
		{
			if (it->dwOfferID == dwOfferID)
			{
				pInfo = &(*it);
				break;
			}
		}

		if(!pInfo)
			return false;

		pkShop->AcceptOffer(pInfo);

		LPCHARACTER chOwner = CHARACTER_MANAGER::instance().FindByPID(pkShop->GetOwnerPID());
		if(chOwner && chOwner->GetOfflineShop()==pkShop && chOwner->GetOfflineShopGuest()==pkShop)
			SendShopOpenMyShopClientPacket(chOwner);

		OFFERSMAP::iterator itMap = m_mapOffer.find(pInfo->dwOffererID);
		if (itMap != m_mapOffer.end())
		{
			std::vector<TOfferInfo>& vec = itMap->second;
			itertype(vec) itVec = vec.begin();

			for (; itVec != vec.end(); itVec++)
			{
				if (itVec->dwOfferID == dwOfferID)
				{
					LPCHARACTER chBuyer = CHARACTER_MANAGER::instance().FindByPID(itVec->dwOffererID);
					itVec->bAccepted = true;

					if(chBuyer && chBuyer->IsLookingOfflineshopOfferList())
						RecvOfferListRequestPacket(chBuyer);

					break;
				}
			}
		}

		return true;
	}

	bool CShopManager::RecvShopSafeboxAddItemDBPacket(DWORD dwOwnerID, DWORD dwItemID, const TItemInfoEx& item)
	{
		LPCHARACTER ch = CHARACTER_MANAGER::instance().FindByPID(dwOwnerID);
		CShopSafebox* pkSafebox = ch && ch->GetShopSafebox() ? ch->GetShopSafebox() : GetShopSafeboxByOwnerID(dwOwnerID);

		if(!pkSafebox)
			return false;

		CShopItem shopItem(dwItemID);
		shopItem.SetInfo(item);
		shopItem.SetOwnerID(dwOwnerID);
		
		pkSafebox->AddItem(&shopItem);
		if(ch && ch->GetShopSafebox())
			pkSafebox->RefreshToOwner(ch);

		OFFSHOP_DEBUG("safebox owner %u , item %u ",dwOwnerID, dwItemID);
		return true;
	}

	bool CShopManager::SendShopSafeboxAddItemDBPacket(DWORD dwOwnerID, const CShopItem& item) {
		TPacketGDNewOfflineShop pack;
		pack.bSubHeader = SUBHEADER_GD_SAFEBOX_ADD_ITEM;


		TSubPacketGDSafeboxAddItem subpack;
		subpack.dwOwnerID = dwOwnerID;
		CopyObject(subpack.item , *item.GetInfo());


		TEMP_BUFFER buff;
		buff.write(&pack, sizeof(pack));
		buff.write(&subpack, sizeof(subpack));

		db_clientdesc->DBPacket(HEADER_GD_NEW_OFFLINESHOP, 0, buff.read_peek(), buff.size());
		return true;
	}

	bool CShopManager::RecvShopSafeboxAddValutesDBPacket(DWORD dwOwnerID, const TValutesInfo& valute)
	{
		LPCHARACTER ch = CHARACTER_MANAGER::instance().FindByPID(dwOwnerID);
		CShopSafebox* pkSafebox = ch && ch->GetShopSafebox() ? ch->GetShopSafebox() : GetShopSafeboxByOwnerID(dwOwnerID);

		if(!pkSafebox)
			return false;
		
		char logBuf[256]; 
		snprintf(logBuf, sizeof(logBuf), "Price: %lld",  (long long) valute.illYang); 

		pkSafebox->AddValute(valute);
		if(ch && ch->GetShopSafebox())
		{
			pkSafebox->RefreshToOwner(ch);
			LogManager::Instance().CharLog(ch, 0, "OFFLINESHOP_SAFEBOX_RECEIVE_MONEY", logBuf);
		}
		else
		{
			LogManager::Instance().CharLog(dwOwnerID, 0, 0, 0, "OFFLINESHOP_SAFEBOX_RECEIVE_MONEY", logBuf, "");
		}
	
		return true;
	}

	void CShopManager::SendShopSafeboxGetItemDBPacket(DWORD dwOwnerID, DWORD dwItemID)
	{
		TPacketGDNewOfflineShop pack;
		pack.bSubHeader	= SUBHEADER_GD_SAFEBOX_GET_ITEM;

		TSubPacketGDSafeboxGetItem subpack;
		subpack.dwOwnerID	= dwOwnerID;
		subpack.dwItemID	= dwItemID;

		TEMP_BUFFER buff;
		buff.write(&pack, sizeof(pack));
		buff.write(&subpack, sizeof(subpack));

		OFFSHOP_DEBUG(" owner % u , item %u ",dwOwnerID , dwItemID);
		db_clientdesc->DBPacket(HEADER_GD_NEW_OFFLINESHOP, 0, buff.read_peek(), buff.size());
	}

	void CShopManager::SendShopSafeboxGetValutesDBPacket(DWORD dwOwnerID, const TValutesInfo& valutes)
	{
		TPacketGDNewOfflineShop pack;
		pack.bSubHeader	= SUBHEADER_GD_SAFEBOX_GET_VALUTES;

		TSubPacketGDSafeboxGetValutes subpack;
		subpack.dwOwnerID	= dwOwnerID;
		CopyObject(subpack.valute , valutes);

		TEMP_BUFFER buff;
		buff.write(&pack, sizeof(pack));
		buff.write(&subpack, sizeof(subpack));

		db_clientdesc->DBPacket(HEADER_GD_NEW_OFFLINESHOP, 0, buff.read_peek(), buff.size());
	}

	bool CShopManager::RecvShopSafeboxLoadDBPacket(DWORD dwOwnerID, const TValutesInfo& valute, const std::vector<DWORD>& ids, const std::vector<TItemInfoEx>& items)
	{
		if(GetShopSafeboxByOwnerID(dwOwnerID))
			return false;

		CShopSafebox::VECITEM vec;
		vec.reserve(ids.size());

		for (DWORD i = 0; i < ids.size(); i++)
		{
			CShopItem item(ids[i]);
			item.SetInfo(items[i]);
			item.SetOwnerID(dwOwnerID);
			
			vec.push_back(item);
		}

		CShopSafebox safebox;
		safebox.SetItems(&vec);
		safebox.SetValuteAmount(valute);

		m_mapSafeboxs.insert(std::make_pair(dwOwnerID, safebox));
		return true;
	}

	bool CShopManager::RecvShopSafeboxExpiredItemDBPacket(DWORD dwOwnerID, DWORD dwItemID) {
		CShopSafebox* data = GetShopSafeboxByOwnerID(dwOwnerID);
		if (data) {
			data->RemoveItem(dwItemID);
			data->RefreshToOwner();
		} return true;
	}

	void CShopManager::SendAuctionCreateDBPacket(const TAuctionInfo& auction)
	{
		OFFSHOP_DEBUG("auction %u, name %s, duration %u ",auction.dwOwnerID, auction.szOwnerName, auction.dwDuration);

		TPacketGDNewOfflineShop pack;
		pack.bSubHeader = SUBHEADER_GD_AUCTION_CREATE;

		TSubPacketGDAuctionCreate subpack;
		CopyObject(subpack.auction , auction);

		TEMP_BUFFER buff;
		buff.write(&pack, sizeof(pack));
		buff.write(&subpack, sizeof(subpack));

		db_clientdesc->DBPacket(HEADER_GD_NEW_OFFLINESHOP , 0 , buff.read_peek() , buff.size());
	}

	void CShopManager::SendAuctionAddOfferDBPacket(const TAuctionOfferInfo& offer)
	{
		TPacketGDNewOfflineShop pack;
		pack.bSubHeader = SUBHEADER_GD_AUCTION_ADD_OFFER;

		TSubPacketGDAuctionAddOffer subpack;
		CopyObject(subpack.offer , offer);

		TEMP_BUFFER buff;
		buff.write(&pack, sizeof(pack));
		buff.write(&subpack, sizeof(subpack));

		db_clientdesc->DBPacket(HEADER_GD_NEW_OFFLINESHOP , 0 , buff.read_peek() , buff.size());
	}

	bool CShopManager::RecvAuctionCreateDBPacket(const TAuctionInfo& auction)
	{
		OFFSHOP_DEBUG("auction %u, name %s, duration %u ",auction.dwOwnerID, auction.szOwnerName, auction.dwDuration);

		if(m_mapAuctions.find(auction.dwOwnerID) != m_mapAuctions.end())
			return false;

		CAuction& obj = m_mapAuctions[auction.dwOwnerID];
		obj.SetInfo(auction);

		LPCHARACTER ch = CHARACTER_MANAGER::instance().FindByPID(auction.dwOwnerID);
		if (ch)
		{
			const TItemTable* itemTable = ITEM_MANAGER::instance().GetTable(auction.item.dwVnum);
			if (itemTable)
			{
				char logBuf[256]; 
				LPCHARACTER chShopOwner = CHARACTER_MANAGER::instance().FindByPID(auction.dwOwnerID);
				if (chShopOwner)
				{
					snprintf(logBuf, sizeof(logBuf), "AuctionOwner: %s Name: %s, Count: %d, Price: %lld", chShopOwner->GetName(), itemTable->szLocaleName, auction.item.dwCount, (long long) auction.init_price.illYang); 
				}
				else
				{
					snprintf(logBuf, sizeof(logBuf), "AuctionOwner: %d Name: %s, Count: %d, Price: %lld", auction.dwOwnerID, itemTable->szLocaleName, auction.item.dwCount, (long long) auction.init_price.illYang); 			
				}
				LogManager::Instance().ItemLog(ch, 0, auction.item.dwVnum, "OFFLINESHOP_ADD_AUCTION_ITEM", logBuf);
			}

			ch->SetAuction(&obj);
			SendAuctionOpenAuctionClientPacket(ch, obj.GetInfo(), std::vector<TAuctionOfferInfo>());
		}

		return true;
	}

	bool CShopManager::RecvAuctionAddOfferDBPacket(const TAuctionOfferInfo& offer)
	{
		OFFSHOP_DEBUG("offer : %u auction, %u buyer",offer.dwOwnerID, offer.dwBuyerID);

		itertype(m_mapAuctions) it;
		if((it=m_mapAuctions.find(offer.dwOwnerID))==m_mapAuctions.end())
			return false;

		CAuction& obj = it->second;
		obj.AddOffer(offer);

		if (obj.GetInfo().dwDuration == 0)
			obj.IncreaseDuration();

		return true;
	}

	bool CShopManager::RecvAuctionExpiredDBPacket(DWORD dwID)
	{
		OFFSHOP_DEBUG("id : %u",dwID);

		CShop::LISTGUEST tempGuestList;

		itertype(m_mapAuctions) it = m_mapAuctions.find(dwID);
		if (it != m_mapAuctions.end())
		{
			CAuction& auct = it->second;

			OFFSHOP_DEBUG("found auction %u (guest count %u) ",dwID, auct.GetGuests().size());

			CShop::LISTGUEST& guestList = auct.GetGuests();
			for (itertype(guestList) itGuest = guestList.begin(); itGuest != guestList.end(); itGuest++)
			{
				LPCHARACTER chGuest = GUEST_PTR(*itGuest);
				if(!chGuest)
					continue;
				chGuest->SetAuctionGuest(NULL);
				OFFSHOP_DEBUG("removing guest from auction %s ",chGuest->GetName());
				tempGuestList.push_back(*itGuest);
			}
			
			m_mapAuctions.erase(it);
		}

		LPCHARACTER owner = CHARACTER_MANAGER::instance().FindByPID(dwID);
		if(owner)
			owner->SetAuction(NULL);

		for (itertype(tempGuestList) itGuests = tempGuestList.begin(); itGuests != tempGuestList.end(); itGuests++)
		{
			LPCHARACTER chGuest = GUEST_PTR(*itGuests);
			RecvAuctionListRequestClientPacket(chGuest);
		}

		return false;
	}

	bool CShopManager::RecvShopCreateNewClientPacket(LPCHARACTER ch, TShopInfo& rShopInfo, std::vector<TShopItemInfo> & vec)
	{
		if(!ch || ch->GetOfflineShop())
			return false;

		if (!_IS_VALID_GM_LEVEL(ch))
			return false;

		if (!ch->CanHandleItem()|| !CheckCharacterActions(ch))
		{
			SendChatPacket(ch,CHAT_PACKET_CANNOT_DO_NOW);
			return true;
		}

		OFFSHOP_DEBUG("ch name %s , item count %u , duration %u ", ch->GetName(), rShopInfo.dwCount, rShopInfo.dwDuration);

		static char szNameChecked[OFFLINE_SHOP_NAME_MAX_LEN];

		strncpy(szNameChecked, rShopInfo.szName, sizeof(szNameChecked));
		if (CBanwordManager::instance().CheckString(szNameChecked, strlen(szNameChecked)))
			return false;

		snprintf(rShopInfo.szName, sizeof(rShopInfo.szName), "%s@%s" , ch->GetName(), szNameChecked );
		
		std::vector<TItemInfo> vecItem;
		vecItem.reserve(vec.size());

		rShopInfo.dwOwnerID = ch->GetPlayerID();
#ifdef ENABLE_IRA_REWORK
		TShopPosition rShopPos;
		rShopPos.lMapIndex = ch->GetMapIndex();
		rShopPos.x = ch->GetX();
		rShopPos.y = ch->GetY();
		rShopPos.bChannel = g_bChannel;
#endif
		TItemInfo itemInfo; 

		for (DWORD i = 0; i < vec.size(); i++)
		{
			TShopItemInfo& rShopItem = vec[i];

			LPITEM item = ch->GetItem(rShopItem.pos);
			if(!item)
				return false;
			
			if(IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_GIVE))
				return false;

			if(IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_MYSHOP))
				return false;

			if (item->isLocked() || item->IsEquipped() || item->IsExchanging())
			{
				SendChatPacket(ch,CHAT_PACKET_CANNOT_DO_NOW);
				return true;
			}
			
#ifdef ENABLE_BINDING_SYSTEM
			if (item->IsSealed()){
				SendChatPacket(ch, CHAT_PACKET_CANNOT_DO_NOW);
				return true;
			}
#endif

			for (DWORD j = 0; j < vec.size(); j++)
			{
				if(i==j)
					continue;

				TShopItemInfo& rShopItemCheck = vec[j];
				if(rShopItemCheck.pos == rShopItem.pos)
					return false;
			}

			ZeroObject(itemInfo);
			
			itemInfo.dwOwnerID = ch->GetPlayerID();
			memcpy(itemInfo.item.aAttr ,	item->GetAttributes(),	sizeof(itemInfo.item.aAttr));
			memcpy(itemInfo.item.alSockets,	item->GetSockets(),		sizeof(itemInfo.item.alSockets));

			itemInfo.item.dwVnum	= item->GetVnum();
			itemInfo.item.dwCount	= item->GetCount();
			itemInfo.item.expiration = GetItemExpiration(item);
#ifdef __ENABLE_CHANGELOOK_SYSTEM__
			itemInfo.item.dwTransmutation = item->GetTransmutation();
#endif
#ifdef ENABLE_NEW_OFFLINESHOP_LOGS
			LogManager::instance().OfflineshopLog(ch->GetPlayerID(), 0, "trying to open shop , adding item vnum %u count %u original id %u ", itemInfo.item.dwVnum, itemInfo.item.dwCount, item->GetID());
#endif
			CopyObject(itemInfo.price, rShopItem.price);
			vecItem.push_back(itemInfo);
		}

		for (DWORD i = 0; i < vec.size(); i++)
		{
			TShopItemInfo& rShopItem = vec[i];
			LPITEM item = ch->GetItem(rShopItem.pos);
			M2_DESTROY_ITEM(item->RemoveFromCharacter());
		}

		OFFSHOP_DEBUG("ch name %s , checked successful , send to db ", ch->GetName());

		rShopInfo.dwDuration = MIN(rShopInfo.dwDuration , OFFLINESHOP_DURATION_MAX_MINUTES);
#ifdef ENABLE_IRA_REWORK
		SendShopCreateNewDBPacket(rShopInfo, rShopPos, vecItem);
#else
		SendShopCreateNewDBPacket(rShopInfo, vecItem);
#endif
		return true;
	}

	bool CShopManager::RecvShopChangeNameClientPacket(LPCHARACTER ch, const char* szName)
	{
		if(!ch || !ch->GetOfflineShop())
			return false;

		static char szNameChecked[OFFLINE_SHOP_NAME_MAX_LEN];
		static char szFullName[OFFLINE_SHOP_NAME_MAX_LEN];

		//cheching about bandword
		strncpy(szNameChecked, szName, sizeof(szNameChecked));
		if (CBanwordManager::instance().CheckString(szNameChecked, strlen(szNameChecked)))
			return false;

		//making full name
		snprintf(szFullName, sizeof(szFullName), "%s@%s" , ch->GetName(), szNameChecked );

#ifdef ENABLE_NEW_OFFLINESHOP_LOGS
		LogManager::instance().OfflineshopLog(ch->GetPlayerID(), 0, "changing shop name : %s -> %s ", ch->GetOfflineShop()->GetName(), szFullName);
#endif

		SendShopChangeNameDBPacket(ch->GetPlayerID(), szFullName);
		return true;
	}

	bool CShopManager::RecvShopForceCloseClientPacket(LPCHARACTER ch)
	{
		if(!ch || !ch->GetOfflineShop())
			return false;

#ifdef ENABLE_NEW_OFFLINESHOP_LOGS
		LogManager::instance().OfflineshopLog(ch->GetPlayerID(), 0, "player asked to close shop (remain items count %u) ", ch->GetOfflineShop()->GetItems()->size());
#endif

		SendShopForceCloseDBPacket(ch->GetPlayerID());
		return true;
	}

	bool CShopManager::RecvShopRequestListClientPacket(LPCHARACTER ch)
	{
		if(!ch || !ch->GetDesc())
			return false;

		SendShopListClientPacket(ch);
		return true;
	}

	bool CShopManager::RecvShopOpenClientPacket(LPCHARACTER ch, DWORD dwOwnerID)
	{
		if(!ch || !ch->GetDesc())
			return false;

		CShop* pkShop = GetShopByOwnerID(dwOwnerID);
		if(!pkShop)
			return false;

		if (ch->GetOfflineShopGuest())
			ch->GetOfflineShopGuest()->RemoveGuest(ch);

		if(ch->GetPlayerID() == dwOwnerID)
			SendShopOpenMyShopClientPacket(ch);
		else
			SendShopOpenClientPacket(ch , pkShop);

		pkShop->AddGuest(ch);
		ch->SetOfflineShopGuest(pkShop);
		return true;
	}

	bool CShopManager::RecvShopOpenMyShopClientPacket(LPCHARACTER ch)
	{
		if(!ch || !ch->GetDesc())
			return false;

		if (!ch->GetOfflineShop())
		{
			SendShopOpenMyShopNoShopClientPacket(ch);
		}
		else
		{
			SendShopOpenMyShopClientPacket(ch);
			ch->GetOfflineShop()->AddGuest(ch);
			ch->SetOfflineShopGuest(ch->GetOfflineShop());
		}
		return true;
	}

	bool CShopManager::RecvShopBuyItemClientPacket(LPCHARACTER ch, DWORD dwOwnerID, DWORD dwItemID, bool isSearch)
	{
		OFFSHOP_DEBUG("owner %u , item id %u ", dwOwnerID, dwItemID);

		if(!ch)
			return false;

		if (!_IS_VALID_GM_LEVEL(ch))
			return false;

		if (!ch->CanHandleItem()|| !CheckCharacterActions(ch))
		{
			SendChatPacket(ch,CHAT_PACKET_CANNOT_DO_NOW);
			return true;
		}

		CShop* pkShop=nullptr;
		if(!(pkShop=GetShopByOwnerID(dwOwnerID)))
			return false;

		OFFSHOP_DEBUG("phase 1 %s", "successful");

		CShopItem* pitem=nullptr;
		if(!pkShop->GetItem(dwItemID, &pitem))
			return false;

		if(!pitem->CanBuy(ch))
			return false;

		OFFSHOP_DEBUG("sending packet to db (buyer %u , owner %u , item %u )",ch->GetPlayerID() , dwOwnerID, dwItemID);
		
		if(isSearch)
			SendShopBuyItemFromSearchClientPacket(ch, dwOwnerID, dwItemID);

		SendShopLockBuyItemDBPacket(ch->GetPlayerID(), dwOwnerID, dwItemID);
		return true;
	}

#ifdef __ENABLE_NEW_SHOP_IN_CITIES__
	bool CShopManager::RecvShopClickEntity(LPCHARACTER ch, DWORD dwShopEntityVID)
	{
		for (itertype(m_vecCities) it = m_vecCities.begin(); it != m_vecCities.end(); it++)
		{

			itertype(it->entitiesByVID) iterMap = it->entitiesByVID.find(dwShopEntityVID);
			if(it->entitiesByVID.end() == iterMap)
				continue;

			DWORD dwPID = iterMap->second->GetShop()->GetOwnerPID();
			
			RecvShopOpenClientPacket(ch, dwPID);
			return true;
		}

		sys_err("cannot found clicked entity , %s vid %u ",ch->GetName(), dwShopEntityVID);
		return false;
	}
#endif

	void CShopManager::SendShopListClientPacket(LPCHARACTER ch)
	{
		if(!ch->GetDesc())
			return;

		TEMP_BUFFER buff;
		TPacketGCNewOfflineshop pack;
		pack.bHeader	= HEADER_GC_NEW_OFFLINESHOP;
		pack.bSubHeader	= SUBHEADER_GC_SHOP_LIST;
		pack.wSize		= sizeof(pack) + sizeof(TSubPacketGCShopList) + (m_mapShops.size() * sizeof(TShopInfo)) ;

		buff.write(&pack, sizeof(pack));

		TSubPacketGCShopList subPack;
		subPack.dwShopCount = m_mapShops.size();
		buff.write(&subPack, sizeof(subPack));

		TShopInfo shopInfo;

		itertype(m_mapShops) it=m_mapShops.begin();
		for (; it != m_mapShops.end(); it++)
		{
			const CShop& rShop	= it->second;
			DWORD dwOwner		= it->first;
			ZeroObject(shopInfo);
			shopInfo.dwCount	= rShop.GetItems()->size();
			shopInfo.dwDuration	= rShop.GetDuration();
			shopInfo.dwOwnerID	= dwOwner;
			strncpy(shopInfo.szName, rShop.GetName(), sizeof(shopInfo.szName));

			buff.write(&shopInfo, sizeof(shopInfo));
		}
		
		ch->GetDesc()->Packet(buff.read_peek() , buff.size());
	}

	void CShopManager::SendShopOpenClientPacket(LPCHARACTER ch, CShop* pkShop)
	{
		if(!ch || !ch->GetDesc() || !pkShop)
			return;

		CShop::VECSHOPITEM* pVec = pkShop->GetItems();
		TEMP_BUFFER buff;
		TPacketGCNewOfflineshop pack;
		pack.bHeader	= HEADER_GC_NEW_OFFLINESHOP;
		pack.bSubHeader	= SUBHEADER_GC_SHOP_OPEN;
		pack.wSize		= sizeof(pack) + sizeof(TSubPacketGCShopOpen) + sizeof(TItemInfo)*pVec->size();

		buff.write(&pack, sizeof(pack));

		TSubPacketGCShopOpen subPack;
		subPack.shop.dwCount	= pVec->size();
		subPack.shop.dwDuration	= pkShop->GetDuration();
		subPack.shop.dwOwnerID	= pkShop->GetOwnerPID();
		strncpy(subPack.shop.szName, pkShop->GetName(), sizeof(subPack.shop.szName));

		buff.write(&subPack, sizeof(subPack));

		TItemInfo itemInfo;

		for (DWORD i = 0; i < pVec->size(); i++)
		{
			CShopItem& rItem = pVec->at(i);
			ZeroObject(itemInfo);

			itemInfo.dwItemID	= rItem.GetID();
			itemInfo.dwOwnerID	= pkShop->GetOwnerPID();
			CopyObject(itemInfo.item, *(rItem.GetInfo()));
			CopyObject(itemInfo.price,*(rItem.GetPrice()));

			buff.write(&itemInfo, sizeof(itemInfo));
		}
		
		ch->GetDesc()->Packet(buff.read_peek(), buff.size());
	}

	void CShopManager::SendShopOpenMyShopNoShopClientPacket(LPCHARACTER ch)
	{
		if(!ch->GetDesc())
			return;

		TPacketGCNewOfflineshop pack;
		pack.bHeader	= HEADER_GC_NEW_OFFLINESHOP;
		pack.bSubHeader	= SUBHEADER_GC_SHOP_OPEN_OWNER_NO_SHOP;
		pack.wSize		= sizeof(pack);
		ch->GetDesc()->Packet(&pack, sizeof(pack));
	}

	void CShopManager::SendShopBuyItemFromSearchClientPacket(LPCHARACTER ch, DWORD dwOwnerID, DWORD dwItemID)
	{
		if(!ch->GetDesc())
			return;

		TPacketGCNewOfflineshop pack;
		pack.bHeader	= HEADER_GC_NEW_OFFLINESHOP;
		pack.bSubHeader	= SUBHEADER_GC_SHOP_BUY_ITEM_FROM_SEARCH;
		pack.wSize		= sizeof(pack) + sizeof(TSubPacketGCShopBuyItemFromSearch);
		TSubPacketGCShopBuyItemFromSearch subpack;
		subpack.dwOwnerID = dwOwnerID;
		subpack.dwItemID  = dwItemID;
		TEMP_BUFFER buff;
		buff.write(&pack,		sizeof(pack));
		buff.write(&subpack,	sizeof(subpack));
		ch->GetDesc()->Packet(buff.read_peek(), buff.size());
	}

	void CShopManager::SendShopOpenMyShopClientPacket(LPCHARACTER ch)
	{
		if(!ch->GetDesc())
			return;

		if(!ch->GetOfflineShop())
			return;

		CShop* pkShop	= ch->GetOfflineShop();
		DWORD dwOwnerID	= ch->GetPlayerID();

		CShop::VECSHOPITEM*  pVec		= pkShop->GetItems();
		CShop::VECSHOPITEM*  pVecSold	= pkShop->GetItemsSold();
		CShop::VECSHOPOFFER* pVecOffer	= pkShop->GetOffers();

		TEMP_BUFFER buff;
		TPacketGCNewOfflineshop pack;
		pack.bHeader	= HEADER_GC_NEW_OFFLINESHOP;
		pack.bSubHeader	= SUBHEADER_GC_SHOP_OPEN_OWNER;
		pack.wSize		= sizeof(pack) + sizeof(TSubPacketGCShopOpenOwner) + sizeof(TItemInfo)*pVec->size() + sizeof(TItemInfo)* pVecSold->size() + sizeof(TOfferInfo)*pVecOffer->size();
		
		buff.write(&pack, sizeof(pack));

		TSubPacketGCShopOpenOwner subPack;
		subPack.shop.dwCount	= pVec->size();
		subPack.shop.dwDuration	= pkShop->GetDuration();
		subPack.shop.dwOwnerID	= dwOwnerID;
		subPack.dwSoldCount		= pVecSold->size();
		subPack.dwOfferCount	= pVecOffer->size();

		strncpy(subPack.shop.szName, pkShop->GetName(), sizeof(subPack.shop.szName));
		
		OFFSHOP_DEBUG("owner %u , item count %u , duration %u offer count %u ",subPack.shop.dwOwnerID, subPack.shop.dwCount , subPack.shop.dwDuration, subPack.dwOfferCount);
		
		buff.write(&subPack, sizeof(subPack));

		TItemInfo itemInfo;

		for (DWORD i = 0; i < pVec->size(); i++)
		{
			CShopItem& rItem = pVec->at(i);
			ZeroObject(itemInfo);

			itemInfo.dwItemID	= rItem.GetID();
			itemInfo.dwOwnerID	= dwOwnerID;
			CopyObject(itemInfo.item, *(rItem.GetInfo()));
			CopyObject(itemInfo.price,*(rItem.GetPrice()));

			OFFSHOP_DEBUG("item id %u , item vnum %u , item count %u ",itemInfo.dwItemID, itemInfo.item.dwVnum , itemInfo.item.dwCount);
			buff.write(&itemInfo, sizeof(itemInfo));
		}

		for (DWORD i = 0; i < pVecSold->size(); i++)
		{
			CShopItem& rItem = pVecSold->at(i);
			ZeroObject(itemInfo);

			itemInfo.dwItemID	= rItem.GetID();
			itemInfo.dwOwnerID	= dwOwnerID;
			CopyObject(itemInfo.item, *(rItem.GetInfo()));
			CopyObject(itemInfo.price,*(rItem.GetPrice()));

			OFFSHOP_DEBUG("item id %u , item vnum %u , item count %u ",itemInfo.dwItemID, itemInfo.item.dwVnum , itemInfo.item.dwCount);
			buff.write(&itemInfo, sizeof(itemInfo));
		}

		if(!pVecOffer->empty())
			buff.write(&pVecOffer->at(0), sizeof(TOfferInfo) * pVecOffer->size());

		ch->GetDesc()->Packet(buff.read_peek(), buff.size());

		for (itertype(*pVecOffer) it = pVecOffer->begin(); it != pVecOffer->end(); it++)
		{
			TOfferInfo& offer = *it;
			if (!offer.bAccepted && !offer.bNoticed)
				SendShopOfferNotifiedDBPacket(offer.dwOfferID, offer.dwOwnerID);
		}
	}

	void CShopManager::SendShopForceClosedClientPacket(DWORD dwOwnerID)
	{
		LPCHARACTER ch = CHARACTER_MANAGER::instance().FindByPID(dwOwnerID);
		if(!ch || !ch->GetDesc())
			return;

		TPacketGCNewOfflineshop pack;
		pack.bHeader	= HEADER_GC_NEW_OFFLINESHOP;
		pack.bSubHeader	= SUBHEADER_GC_SHOP_OPEN_OWNER;

		pack.wSize = sizeof(pack);
		ch->GetDesc()->Packet(&pack , sizeof(pack));
	}

	bool CShopManager::RecvShopAddItemClientPacket(LPCHARACTER ch, const TItemPos& pos, const TPriceInfo& price)
	{
		if(!ch || !ch->GetOfflineShop())
			return false;

		if (!_IS_VALID_GM_LEVEL(ch))
			return false;

		if (!ch->CanHandleItem()|| !CheckCharacterActions(ch))
		{
			SendChatPacket(ch,CHAT_PACKET_CANNOT_DO_NOW);
			return true;
		}

		LPITEM pkItem = ch->GetItem(pos);
		if(!pkItem)
			return false;

		if (pkItem->isLocked() || pkItem->IsEquipped() || pkItem->IsExchanging())
		{
			SendChatPacket(ch,CHAT_PACKET_CANNOT_DO_NOW);
			return true;
		}
		
#ifdef ENABLE_BINDING_SYSTEM
		if (pkItem->IsSealed()) {
			SendChatPacket(ch, CHAT_PACKET_CANNOT_DO_NOW);
			return true;
		}
#endif
		
		if(IS_SET(pkItem->GetAntiFlag(), ITEM_ANTIFLAG_GIVE))
			return false;
		if(IS_SET(pkItem->GetAntiFlag(), ITEM_ANTIFLAG_MYSHOP))
			return false;

		TItemInfo itemInfo;
		ZeroObject(itemInfo);

		itemInfo.dwOwnerID		= ch->GetPlayerID();
		itemInfo.item.dwVnum	= pkItem->GetVnum();
		itemInfo.item.dwCount	= (DWORD)pkItem->GetCount();
		itemInfo.item.expiration = GetItemExpiration(pkItem);
		memcpy(itemInfo.item.aAttr,		pkItem->GetAttributes(),	sizeof(itemInfo.item.aAttr));
		memcpy(itemInfo.item.alSockets,	pkItem->GetSockets(),		sizeof(itemInfo.item.alSockets));

#ifdef __ENABLE_CHANGELOOK_SYSTEM__
		itemInfo.item.dwTransmutation = pkItem->GetTransmutation();
#endif

		CopyObject(itemInfo.price, price);

#ifdef ENABLE_NEW_OFFLINESHOP_LOGS
		LogManager::instance().OfflineshopLog(ch->GetPlayerID(), 0, "adding new item to the shop vnum %u count %u (original item ID %u) ", itemInfo.item.dwVnum, itemInfo.item.dwCount, pkItem->GetID());
#endif

		M2_DESTROY_ITEM( pkItem->RemoveFromCharacter() );

		char logBuf[256];
		snprintf(logBuf, sizeof(logBuf), "Name: %s, Count: %d", pkItem->GetName(), pkItem->GetCount());
		LogManager::Instance().ItemLog(ch, pkItem, "OFFLINESHOP_ADD_ITEM", logBuf);

		SendShopAddItemDBPacket(ch->GetPlayerID(), itemInfo);
		return true;
	}

	bool CShopManager::RecvShopRemoveItemClientPacket(LPCHARACTER ch, DWORD dwItemID)
	{
		if(!ch || !ch->GetOfflineShop())
			return false;

		if(!ch || !ch->GetOfflineShop())
			return false;

		CShop* pkShop		= ch->GetOfflineShop();
		CShopItem* pItem	= nullptr;

		OFFSHOP_DEBUG("owner %u , item id %u ",ch->GetPlayerID() , dwItemID);

		if (pkShop->GetItems()->size() == 1)
		{
			SendChatPacket(ch, CHAT_PACKET_CANNOT_REMOVE_LAST_ITEM);
			return false;
		}

		if(!pkShop->GetItem(dwItemID, &pItem))
			return false;

		const TItemTable* itemTable =  ITEM_MANAGER::instance().GetTable(pItem->GetInfo()->dwVnum);
		if (itemTable)
		{
			char logBuf[256];
			snprintf(logBuf, sizeof(logBuf), "Name: %s, Count: %d", itemTable->szLocaleName, pItem->GetInfo()->dwCount);
			LogManager::Instance().ItemLog(ch, pItem->GetID(), pItem->GetInfo()->dwVnum, "OFFLINESHOP_REMOVE_ITEM", logBuf);
		}

		SendShopRemoveItemDBPacket(pkShop->GetOwnerPID(), pItem->GetID());
		return true;
	}

	bool CShopManager::RecvShopEditItemClientPacket(LPCHARACTER ch, DWORD dwItemID, const TPriceInfo& price)
	{
		if(!ch || !ch->GetOfflineShop())
			return false;

		if (!_IS_VALID_GM_LEVEL(ch))
			return false;

		CShop* pkShop		= ch->GetOfflineShop();
		CShopItem* pItem	= nullptr;

		if(!pkShop->GetItem(dwItemID, &pItem))
			return false;

		TPriceInfo* pPrice = pItem->GetPrice();

		if(price.illYang == pPrice->illYang)
			return true;

		SendShopEditItemDBPacket(pkShop->GetOwnerPID(), dwItemID, price);
		return true;
	}

	bool CShopManager::RecvShopFilterRequestClientPacket(LPCHARACTER ch, const TFilterInfo& filter)
	{
		if(!ch)
			return false;

		std::vector<TItemInfo> vec;
		
		if (!CheckSearchTime(ch->GetPlayerID()))
		{
			SendShopFilterResultClientPacket(ch, vec);
			SendChatPacket(ch,CHAT_PACKET_CANNOT_SEARCH_YET);
			return true;
		}

		std::string stName = StringToLower(filter.szName, strnlen(filter.szName, sizeof(filter.szName)));


		itertype(m_mapShops) cit= m_mapShops.begin();
		for (; cit != m_mapShops.end(); cit++)
		{
			const CShop& rcShop = cit->second;

			if(rcShop.GetOwnerPID() == ch->GetPlayerID())
				continue;

			CShop::VECSHOPITEM* pShopItems = rcShop.GetItems();

			itertype(*pShopItems) cItemIter = pShopItems->begin();
			for (; cItemIter != pShopItems->end(); cItemIter++)
			{
				const CShopItem&	rItem		= *cItemIter;
				const TItemInfoEx&	rItemInfo	= *rItem.GetInfo();
				const TPriceInfo&	rItemPrice	= *rItem.GetPrice();

				TItemTable* pTable = ITEM_MANAGER::instance().GetTable(rItemInfo.dwVnum);
				if (!pTable)
				{
					sys_err("CANNOT FIND ITEM TABLE [%d]");
					continue;
				}

				if(filter.bType != ITEM_NONE && filter.bType != pTable->bType)
					continue;

				if(filter.bType != ITEM_NONE && filter.bSubType != SUBTYPE_NOSET && filter.bSubType != pTable->bSubType)
					continue;

				int iLimitLevel = pTable->aLimits[0].bType == LIMIT_LEVEL?pTable->aLimits[0].lValue : pTable->aLimits[1].bType == LIMIT_LEVEL? pTable->aLimits[1].lValue : 0;

				if ((filter.iLevelStart != 0 || filter.iLevelEnd != 0))
				{
					if(iLimitLevel == 0)
						continue;

					if(iLimitLevel < filter.iLevelStart && filter.iLevelStart!=0)
						continue;

					if(iLimitLevel > filter.iLevelEnd && filter.iLevelEnd!=0)
						continue;
				}

				if(filter.priceStart.illYang != 0)
					if(GetTotalAmountFromPrice(rItemPrice) < filter.priceStart.illYang)
						continue;

				if(filter.priceEnd.illYang != 0)
					if(GetTotalAmountFromPrice(rItemPrice) > filter.priceEnd.illYang)
						continue;

#ifdef ENABLE_GLOBAL_LANGUAGE
				if(strnlen(filter.szName, sizeof(filter.szName)) != 0 )
				{
					const std::string& item_name = find_itemnames(rItemInfo.dwVnum, ch->GetLang());
					if(!MatchItemName(stName, item_name.c_str(), item_name.length()))
					{
						continue;
					}
				}
#else
				if(strnlen(filter.szName, sizeof(filter.szName)) != 0 && !MatchItemName(stName , pTable->szLocaleName , strnlen(pTable->szLocaleName, ITEM_NAME_MAX_LEN)))
				{
					continue;
				}
#endif

				if(!MatchWearFlag(filter.dwWearFlag, pTable->dwAntiFlags))
					continue;

				if(!MatchAttributes(filter.aAttr, rItemInfo.aAttr))
					continue;

				TItemInfo itemInfo;
				CopyObject(itemInfo.item, rItemInfo);
				CopyObject(itemInfo.price,rItemPrice);

				itemInfo.dwItemID	= rItem.GetID();
				itemInfo.dwOwnerID	= rcShop.GetOwnerPID();

				vec.push_back(itemInfo);

				if(vec.size() >= OFFLINESHOP_MAX_SEARCH_RESULT)
					break;
			}

			if(vec.size() >= OFFLINESHOP_MAX_SEARCH_RESULT)
				break;
		}

		SendShopFilterResultClientPacket(ch, vec);
		return true;
	}

	void CShopManager::SendShopFilterResultClientPacket(LPCHARACTER ch, const std::vector<TItemInfo>& items)
	{
		if(!ch || !ch->GetDesc())
			return;

		TEMP_BUFFER buff;

		TPacketGCNewOfflineshop pack;
		pack.bHeader	= HEADER_GC_NEW_OFFLINESHOP;
		pack.bSubHeader	= SUBHEADER_GC_SHOP_FILTER_RESULT;
		pack.wSize		= sizeof(pack) + sizeof(TSubPacketGCShopFilterResult) + sizeof(TItemInfo)*items.size();
		buff.write(&pack, sizeof(pack));

		TSubPacketGCShopFilterResult subpack;
		subpack.dwCount	= items.size();
		buff.write(&subpack, sizeof(subpack));

		for (DWORD i = 0; i < items.size(); i++)
		{
			const TItemInfo& rItemInfo= items[i];
			buff.write(&rItemInfo, sizeof(rItemInfo));
		}

		ch->GetDesc()->Packet(buff.read_peek(), buff.size());
	}

	bool CShopManager::RecvShopCreateOfferClientPacket(LPCHARACTER ch, TOfferInfo& offer)
	{
		if(!ch)
			return false;

		if (!_IS_VALID_GM_LEVEL(ch))
			return false;

		if(ch->GetPlayerID() == offer.dwOwnerID)
			return false;

		CShop* pkShop = GetShopByOwnerID(offer.dwOwnerID);
		if(!pkShop)
			return false;

		CShopItem* item = nullptr;
		if(!pkShop->GetItem(offer.dwItemID, &item))
			return false;

#ifndef __ENABLE_CHEQUE_SYSTEM__
		if (offer.price.illYang <= 0)
			return false;
#else
		if (offer.price.illYang <= 0 && offer.price.iCheque == 0)
			return false;
#endif

		if( (long long) ch->GetGold() < offer.price.illYang)
			return false;

#ifdef __ENABLE_CHEQUE_SYSTEM__
		if ( ch->GetCheque() < offer.price.iCheque)
			return false;
#endif

		ch->ChangeGold(-offer.price.illYang);
#ifdef __ENABLE_CHEQUE_SYSTEM__
		ch->PointChange(POINT_CHEQUE, -offer.price.iCheque);
		offer.price.illYang = offer.price.GetTotalYangAmount();
		offer.price.iCheque =0;
#endif

		offer.bAccepted		= false;
		offer.bNoticed		= false;
		offer.dwOffererID	= ch->GetPlayerID();

		strncpy(offer.szBuyerName, ch->GetName(), sizeof(offer.szBuyerName));

		SendShopOfferNewDBPacket(offer);
		return true;
	}

	bool CShopManager::RecvShopEditOfferClientPacket(LPCHARACTER ch, const TOfferInfo& offer)
	{
		if(!ch)
			return false;

		if (!_IS_VALID_GM_LEVEL(ch))
			return false;

		return true;
	}

	bool CShopManager::RecvShopAcceptOfferClientPacket(LPCHARACTER ch, DWORD dwOfferID)
	{
		if(!ch || !ch->GetOfflineShop())
			return false;

		if (!_IS_VALID_GM_LEVEL(ch))
			return false;

		CShop* pkShop	= ch->GetOfflineShop();
		CShopItem* item	= nullptr;

		TOfferInfo* info = nullptr;
		CShop::VECSHOPOFFER& vec = *(pkShop->GetOffers());
		for (DWORD i = 0; i < vec.size(); i++)
		{
			if (dwOfferID == vec[i].dwOfferID)
			{
				info = &vec[i];
				break;
			}
		}

		if(!info || info->bAccepted)
			return false;

		if(ch->GetPlayerID() != info->dwOwnerID)
			return false;

		if(!pkShop->GetItem(info->dwItemID, &item))
			return false;

		info->bAccepted = true;
		SendShopOfferAcceptDBPacket(*info);
		return true;
	}

	bool CShopManager::RecvShopCancelOfferClientPacket(LPCHARACTER ch, DWORD dwOfferID, DWORD dwOwnerID)
	{
		if(!ch)
			return false;

		if (!_IS_VALID_GM_LEVEL(ch))
			return false;

		CShop* pkShop	= GetShopByOwnerID(dwOwnerID);

		if(!pkShop)
			return false;

		CShopItem* item	= nullptr;

		TOfferInfo* info = nullptr;
		CShop::VECSHOPOFFER& vec = *(pkShop->GetOffers());
		for (DWORD i = 0; i < vec.size(); i++)
		{
			if (dwOfferID == vec[i].dwOfferID)
			{
				info = &vec[i];
				break;
			}
		}

		if(!info || info->bAccepted)
			return false;

		if(!pkShop->GetItem(info->dwItemID, &item))
			return false;

		if(ch->GetPlayerID() != pkShop->GetOwnerPID() && ch->GetPlayerID() != info->dwOffererID)
			return false;

		OFFSHOP_DEBUG("success %u offer , %u shop ", dwOfferID, dwOwnerID);
		SendShopOfferCancelDBPacket(*info);
		return true;
	}

	bool CShopManager::RecvOfferListRequestPacket(LPCHARACTER ch)
	{
		if (!ch->GetDesc())
			return false;

		TPacketGCNewOfflineshop pack;
		pack.bHeader	= HEADER_GC_NEW_OFFLINESHOP;
		pack.bSubHeader	= SUBHEADER_GC_OFFER_LIST;
		pack.wSize		= sizeof(pack) + sizeof(TSubPacketGCShopOfferList);

		TSubPacketGCShopOfferList subpack;
		subpack.dwOfferCount = 0;

		TEMP_BUFFER buff;

		OFFERSMAP::iterator it = m_mapOffer.find(ch->GetPlayerID());
		if (it == m_mapOffer.end() || it->second.empty())
		{
			buff.write(&pack, sizeof(pack));
			buff.write(&subpack, sizeof(subpack));

			OFFSHOP_DEBUG("return because not found or empty vec : found > %s  (id %u) ",it!=m_mapOffer.end()?"TRUE":"FALSE" , ch->GetPlayerID());

			ch->GetDesc()->Packet(buff.read_peek() , buff.size());
			return true;
		}

		const std::vector<TOfferInfo>& vec = it->second;
		pack.wSize += sizeof(TOfferInfo)*vec.size();

		std::vector<TMyOfferExtraInfo> extrainfo;
		extrainfo.resize(vec.size());
		subpack.dwOfferCount = vec.size();

		OFFSHOP_DEBUG("found %u in map, size %u ",ch->GetPlayerID(), vec.size());

		for (DWORD i = 0; i < vec.size(); i++)
		{
			const TOfferInfo& offer = vec[i];
			CShop* pkShop = GetShopByOwnerID(offer.dwOwnerID);
			if (!pkShop)
			{
				sys_err("cannot find item's shop %u , offer id %u ",offer.dwOwnerID, offer.dwOfferID);
				return false;
			}

			CShopItem* pkItem=nullptr;
			if(!pkShop->GetItem(offer.dwItemID, &pkItem) && !pkShop->GetItemSold(offer.dwItemID, &pkItem))
			{
				sys_err("cannot find item info %u , offer id %u ",offer.dwItemID, offer.dwOfferID);
				return false;
			}

			TItemInfo& itemInfo = extrainfo[i].item;
			itemInfo.dwItemID	= offer.dwItemID;
			itemInfo.dwOwnerID	= offer.dwOwnerID;

			CopyObject(itemInfo.item, *pkItem->GetInfo());
			CopyObject(itemInfo.price, *pkItem->GetPrice());

			strncpy(extrainfo[i].szShopName , pkShop->GetName(), OFFLINE_SHOP_NAME_MAX_LEN);
		}

		pack.wSize += sizeof(TMyOfferExtraInfo)*extrainfo.size();

		buff.write(&pack,			sizeof(pack));
		buff.write(&subpack,		sizeof(subpack));
		buff.write(&vec[0],			sizeof(TOfferInfo)*vec.size());
		buff.write(&extrainfo[0] ,	sizeof(TMyOfferExtraInfo)*extrainfo.size());

		ch->SetLookingOfflineshopOfferList(true);
		ch->GetDesc()->Packet(buff.read_peek(), buff.size());
		return true;
	}

	bool CShopManager::RecvShopSafeboxOpenClientPacket(LPCHARACTER ch)
	{
		if(!ch || ch->GetShopSafebox())
			return false;

		CShopSafebox* pkSafebox = GetShopSafeboxByOwnerID(ch->GetPlayerID());
		if(!pkSafebox)
			return false;

		ch->SetShopSafebox(pkSafebox);
		pkSafebox->RefreshToOwner(ch);
		return true;
	}

	bool CShopManager::RecvShopSafeboxGetItemClientPacket(LPCHARACTER ch, DWORD dwItemID)
	{
		if(!ch || !ch->GetShopSafebox())
			return false;

		CShopSafebox* pkSafebox = ch->GetShopSafebox();
		CShopItem* pItem=nullptr;

		if(!pkSafebox->GetItem(dwItemID, &pItem))
			return false;

		LPITEM pkItem = pItem->CreateItem();
		if(!pkItem)
			return false;

		TItemPos itemPos;
		if (!ch->CanTakeInventoryItem(pkItem, &itemPos))
		{
			M2_DESTROY_ITEM(pkItem);
			return false;
		}

		if (pkSafebox->RemoveItem(dwItemID))
		{
			pkSafebox->RefreshToOwner();
			pkItem->AddToCharacter(ch, itemPos);
		}
		
		char logBuf[256];
		snprintf(logBuf, sizeof(logBuf), "Name: %s, Count: %d", pkItem->GetName(), pkItem->GetCount());
		LogManager::Instance().ItemLog(ch, pkItem, "OFFLINESHOP_SAFEBOX_ITEMS_WITHDRAW", logBuf);

		SendShopSafeboxGetItemDBPacket(ch->GetPlayerID(), dwItemID);
		return true;
	}

	bool CShopManager::RecvShopSafeboxGetValutesClientPacket(LPCHARACTER ch, const TValutesInfo& valutes)
	{
		if(!ch ||!ch->GetShopSafebox())
			return false;

#if !defined(ENABLE_FULL_YANG) && !defined(FULL_YANG)
		if((long long) ch->GetGold() + valutes.illYang > (long long) yang_max)
			return false;
#endif
#ifdef __ENABLE_CHEQUE_SYSTEM__
		if (ch->GetCheque() + valutes.iCheque >= CHEQUE_MAX)
			return false;
#endif

		CShopSafebox* pkSafebox		= ch->GetShopSafebox();
		CShopSafebox::SValuteAmount peekAmount(valutes);

		if(!pkSafebox->RemoveValute(peekAmount))
			return false;

		ch->ChangeGold(valutes.illYang);
#ifdef __ENABLE_CHEQUE_SYSTEM__
		ch->PointChange(POINT_CHEQUE, valutes.iCheque);
#endif

		pkSafebox->RefreshToOwner();

		char logBuf[256];
		snprintf(logBuf, sizeof(logBuf), "Yang: %lld", valutes.illYang);
		LogManager::Instance().CharLog(ch, 0, "OFFLINESHOP_SAFEBOX_YANG_WITHDRAW", logBuf);

		SendShopSafeboxGetValutesDBPacket(ch->GetPlayerID(), valutes);
		return true;
	}

	bool CShopManager::RecvShopSafeboxCloseClientPacket(LPCHARACTER ch)
	{
		if(!ch || !ch->GetShopSafebox())
			return false;
		
		ch->SetShopSafebox(NULL);
		return true;
	}

	void CShopManager::SendShopSafeboxRefresh(LPCHARACTER ch, const TValutesInfo& valute, const std::vector<CShopItem>& vec)
	{
		if(!ch->GetDesc())
			return;

		if(!ch || !ch->GetShopSafebox())
			return;

		TPacketGCNewOfflineshop pack;
		pack.bHeader	= HEADER_GC_NEW_OFFLINESHOP;
		pack.wSize		= sizeof(pack) + sizeof(TSubPacketGCShopSafeboxRefresh) + ((sizeof(DWORD) + sizeof(TItemInfoEx))*vec.size());
		pack.bSubHeader	= SUBHEADER_GC_SHOP_SAFEBOX_REFRESH;

		TSubPacketGCShopSafeboxRefresh subpack;
		subpack.dwItemCount	= vec.size();
		CopyObject(subpack.valute , valute);

		TEMP_BUFFER buff;
		buff.write(&pack,		sizeof(pack));	
		buff.write(&subpack,	sizeof(subpack));
		
		TItemInfoEx item;
		DWORD dwItemID=0;

		for (itertype(vec) it = vec.begin(); it != vec.end(); it++)
		{
			const CShopItem& shopitem = *it;
			
			dwItemID = shopitem.GetID();
			CopyObject(item, *shopitem.GetInfo());

			buff.write(&dwItemID,	sizeof(dwItemID));
			buff.write(&item,		sizeof(item));
		}

		ch->GetDesc()->Packet(buff.read_peek() , buff.size());
	}

	bool CShopManager::RecvAuctionListRequestClientPacket(LPCHARACTER ch)
	{
		if(!ch)
			return false;

		TAuctionListElement temp;
		std::vector<TAuctionListElement> vec;
		vec.reserve(m_mapAuctions.size());

		for (itertype(m_mapAuctions) it = m_mapAuctions.begin(); it != m_mapAuctions.end(); it++)
		{
			const CAuction& rAuction = it->second;

			CopyObject(temp.actual_best, rAuction.GetBestOffer());
			CopyObject(temp.auction, rAuction.GetInfo());

			temp.dwOfferCount = rAuction.GetOffers().size();
			vec.push_back(temp);
		}

		SendAuctionListClientPacket(ch, vec);
		return true;
	}

	bool CShopManager::RecvAuctionOpenRequestClientPacket(LPCHARACTER ch, DWORD dwOwnerID)
	{
		itertype(m_mapAuctions) it = m_mapAuctions.find(dwOwnerID);
		if(it == m_mapAuctions.end())
			return false;

		it->second.AddGuest(ch);
		return true;
	}

	bool CShopManager::RecvMyAuctionOpenRequestClientPacket(LPCHARACTER ch)
	{
		OFFSHOP_DEBUG("pid %u , exist %s ",ch->GetPlayerID(), m_mapAuctions.find(ch->GetPlayerID()) != m_mapAuctions.end() ? "TRUE" : "FALSE" );

		if (!ch->GetAuction())
		{
			itertype(m_mapAuctions) it = m_mapAuctions.find(ch->GetPlayerID());

			if (it == m_mapAuctions.end())
				SendAuctionOpenMyAuctionNoAuctionClientPacket(ch);
				
			else
			{
				it->second.AddGuest(ch);
			}
			
		}

		else
		{
			CAuction* pkAuction = ch->GetAuction();
			pkAuction->AddGuest(ch);
		}

		return true;
	}

	bool CShopManager::RecvAuctionCreateClientPacket(LPCHARACTER ch, DWORD dwDuration, const TPriceInfo& init_price, const TItemPos& pos)
	{
		if(!ch || ch->GetAuction())
			return false;

		if (!_IS_VALID_GM_LEVEL(ch))
			return false;

		if (!ch->CanHandleItem() || !CheckCharacterActions(ch))
		{
			SendChatPacket(ch,CHAT_PACKET_CANNOT_DO_NOW);
			return true;
		}

		OFFSHOP_DEBUG("owner %u , duration %u ",ch->GetPlayerID(), dwDuration);

		dwDuration = MIN(OFFLINESHOP_DURATION_MAX_MINUTES, dwDuration);

		LPITEM item = ch->GetItem(pos);
		if(!item)
			return false;

		if(item->IsEquipped() || item->IsExchanging() || item->isLocked())
			return false;
#ifdef ENABLE_BINDING_SYSTEM
		if (item->IsSealed()) {
			return false;
		}
#endif
		TItemTable* pItemTable= ITEM_MANAGER::instance().GetTable(item->GetVnum());
		if(!pItemTable)
			return false;


		if(IS_SET(pItemTable->dwAntiFlags, ITEM_ANTIFLAG_GIVE) || IS_SET(pItemTable->dwAntiFlags , ITEM_ANTIFLAG_MYSHOP))
			return false;

		TAuctionInfo auction;
		auction.dwDuration = dwDuration;
		auction.dwOwnerID  = ch->GetPlayerID();
		strncpy(auction.szOwnerName, ch->GetName(), sizeof(auction.szOwnerName));
		CopyObject(auction.init_price, init_price);

		TItemInfoEx& itemInfo = auction.item;
		itemInfo.dwCount	= item->GetCount();
		itemInfo.dwVnum		= item->GetVnum();
		itemInfo.expiration = GetItemExpiration(item);
		memcpy(itemInfo.aAttr ,		item->GetAttributes(),	sizeof(itemInfo.aAttr));
		memcpy(itemInfo.alSockets,	item->GetSockets(),		sizeof(itemInfo.alSockets));
		
#ifdef __ENABLE_CHANGELOOK_SYSTEM__
		itemInfo.dwTransmutation = item->GetTransmutation();
#endif

		M2_DESTROY_ITEM(item->RemoveFromCharacter());
		SendAuctionCreateDBPacket(auction);
		return true;
	}

	bool CShopManager::RecvAuctionAddOfferClientPacket(LPCHARACTER ch, DWORD dwOwnerID, const TPriceInfo& price)
	{
		if(!ch || !ch->GetAuctionGuest() || ch->GetAuctionGuest()->GetInfo().dwOwnerID != dwOwnerID)
			return false;

		if (!_IS_VALID_GM_LEVEL(ch))
			return false;

		if(ch->GetPlayerID() == dwOwnerID)
			return false;

		if((long long) ch->GetGold() < price.illYang)
			return false;

#ifdef __ENABLE_CHEQUE_SYSTEM__
		if ( ch->GetCheque() < price.iCheque)
			return false;
#endif

		if (!CheckLastOfferTime(ch->GetPlayerID()))
		{
			SendChatPacket(ch, CHAT_PACKET_CANNOT_DO_NOW);
			return false;
		}

		CAuction* pAuction = ch->GetAuctionGuest();
		if(pAuction->GetBestBuyer() == ch->GetPlayerID())
			return false;
		
		OFFSHOP_DEBUG("pAuction->GetBestBuyer() = %u ",pAuction->GetBestBuyer());

		if (pAuction->GetOffers().empty())
		{
			if (price < pAuction->GetInfo().init_price)
				return false;
		}
		else
		{
			const TPriceInfo& bestOffer = pAuction->GetBestOffer();
			if(!CheckNewAuctionOfferPrice(price,bestOffer))
				return false;
		}

		ch->ChangeGold(-price.illYang);
#ifdef __ENABLE_CHEQUE_SYSTEM__
		ch->PointChange(POINT_CHEQUE, -price.iCheque);
#endif

		TAuctionOfferInfo offer;
		offer.dwBuyerID	= ch->GetPlayerID();
		offer.dwOwnerID	= dwOwnerID;
		CopyObject(offer.price	, price);
#ifdef __ENABLE_CHEQUE_SYSTEM__
		offer.price.illYang = offer.price.GetTotalYangAmount();
		offer.price.iCheque=0;
#endif
		strncpy(offer.szBuyerName, ch->GetName(), sizeof(offer.szBuyerName));
		SendAuctionAddOfferDBPacket(offer);
		return true;
	}

	bool CShopManager::RecvAuctionExitFromAuction(LPCHARACTER ch, DWORD dwOwnerID)
	{
		itertype(m_mapAuctions) it = m_mapAuctions.find(ch->GetPlayerID());
		if(it == m_mapAuctions.end())
			return false;
		it->second.RemoveGuest(ch);
		return true;
	}

	void CShopManager::SendAuctionListClientPacket(LPCHARACTER ch, const std::vector<TAuctionListElement>& auctionVec)
	{
		if(!ch->GetDesc())
			return;

		TPacketGCNewOfflineshop pack;
		pack.bHeader	= HEADER_GC_NEW_OFFLINESHOP;
		pack.bSubHeader	= SUBHEADER_GC_AUCTION_LIST;
		pack.wSize		= sizeof(pack) + sizeof(TSubPacketGCAuctionList) + (sizeof(TAuctionListElement)*auctionVec.size());
		TSubPacketGCAuctionList subpack;
		subpack.dwCount = auctionVec.size();
		TEMP_BUFFER buff;
		buff.write(&pack,		sizeof(pack));
		buff.write(&subpack,	sizeof(subpack));
		if(!auctionVec.empty())
			buff.write(&auctionVec[0], sizeof(auctionVec[0]) * auctionVec.size());
		ch->GetDesc()->Packet(buff.read_peek(), buff.size());
	}

	void CShopManager::SendAuctionOpenAuctionClientPacket(LPCHARACTER ch, const TAuctionInfo& auction, const std::vector<TAuctionOfferInfo>& vec)
	{
		if(!ch->GetDesc())
			return;

		TPacketGCNewOfflineshop pack;
		pack.bHeader	= HEADER_GC_NEW_OFFLINESHOP;
		pack.bSubHeader	= ch->GetPlayerID()!=auction.dwOwnerID ? SUBHEADER_GC_OPEN_AUCTION: SUBHEADER_GC_OPEN_MY_AUCTION;
		pack.wSize		= sizeof(pack)+ sizeof(TSubPacketGCAuctionOpen) + (sizeof(TAuctionOfferInfo) * vec.size());

		TSubPacketGCAuctionOpen subpack;
		CopyObject(subpack.auction, auction);
		subpack.dwOfferCount = vec.size();

		TEMP_BUFFER buff;
		buff.write(&pack,		sizeof(pack));
		buff.write(&subpack,	sizeof(subpack));

		if(!vec.empty())
			buff.write(&vec[0], sizeof(vec[0]) * vec.size());

		ch->GetDesc()->Packet(buff.read_peek(), buff.size());
	}

	void CShopManager::SendAuctionOpenMyAuctionNoAuctionClientPacket(LPCHARACTER ch)
	{
		if(!ch->GetDesc())
			return;

		TPacketGCNewOfflineshop pack;
		pack.bHeader	= HEADER_GC_NEW_OFFLINESHOP;
		pack.bSubHeader	= SUBHEADER_GC_OPEN_MY_AUCTION_NO_AUCTION;
		pack.wSize		= sizeof(pack);
		ch->GetDesc()->Packet(&pack, sizeof(pack));
	}

	void CShopManager::RecvCloseBoardClientPacket(LPCHARACTER ch)
	{
		if(!ch || !ch->GetDesc())
			return;

		if (ch->GetAuctionGuest())
		{
			ch->GetAuctionGuest()->RemoveGuest(ch);
			ch->SetAuctionGuest(NULL);
		}

		if(ch->GetShopSafebox())
			ch->SetShopSafebox(NULL);

		if (ch->GetOfflineShopGuest())
		{
			ch->GetOfflineShopGuest()->RemoveGuest(ch);
			ch->SetOfflineShopGuest(NULL);
		}

		if(ch->GetOfflineShop())
			ch->GetOfflineShop()->RemoveGuest(ch);

		ch->SetLookingOfflineshopOfferList(false);
	}

	void CShopManager::UpdateShopsDuration()
	{
		SHOPMAP::iterator it = m_mapShops.begin();
		for (; it != m_mapShops.end(); it++)
		{
			CShop& shop = it->second;

			if(shop.GetDuration() > 0)
				shop.DecreaseDuration();
		}
	}

	void CShopManager::UpdateAuctionsDuration()
	{
		AUCTIONMAP::iterator it = m_mapAuctions.begin();
		for (; it != m_mapAuctions.end(); it++)
		{
			CAuction& auction = it->second;
			auction.DecreaseDuration();
		}
	}

	void CShopManager::ClearSearchTimeMap()
	{
		m_searchTimeMap.clear();
	}

	bool CShopManager::CheckSearchTime(DWORD dwPID) 
	{
		itertype(m_searchTimeMap) it = m_searchTimeMap.find(dwPID);
		if (it == m_searchTimeMap.end())
		{
			m_searchTimeMap.insert(std::make_pair(dwPID, get_dword_time()));
			return true;
		}

		if(it->second + OFFLINESHOP_SECONDS_PER_SEARCH*1000 > get_dword_time())
			return false;

		it->second = get_dword_time();
		return true;
	}

	bool CShopManager::CheckLastOfferTime(DWORD dwPID) 
	{
		itertype(m_offerTimeMap) it = m_offerTimeMap.find(dwPID);
		if (it == m_offerTimeMap.end())
		{
			m_offerTimeMap.insert(std::make_pair(dwPID, get_dword_time()));
			return true;
		}

		if (it->second + OFFLINESHOP_SECONDS_PER_OFFER * 1000 > get_dword_time())
			return false;

		it->second = get_dword_time();
		return true;
	}

	void CShopManager::ClearOfferTimeMap() 
	{
		m_offerTimeMap.clear();
	}
}
#endif