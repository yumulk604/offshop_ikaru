1* Search for:
void CPythonPlayer::__OnPressSmart(CInstanceBase& rkInstMain, bool isAuto)
{
	DWORD dwPickedItemID;
	DWORD dwPickedActorID;
	TPixelPosition kPPosPickedGround;

	bool isPickedItemID=__GetPickedItemID(&dwPickedItemID);
	bool isPickedActorID=__GetPickedActorID(&dwPickedActorID);
	bool isPickedGroundPos=__GetPickedGroundPos(&kPPosPickedGround);

	if (isPickedItemID)
	{
		__OnPressItem(rkInstMain, dwPickedItemID);
	}
	else if (isPickedActorID && dwPickedActorID != GetMainCharacterIndex())
	{
		__OnPressActor(rkInstMain, dwPickedActorID, isAuto);
	}
	else if (isPickedGroundPos)
	{
		__OnPressGround(rkInstMain, kPPosPickedGround);
	}
	else
	{
		__OnPressScreen(rkInstMain);
	}
}

void CPythonPlayer::__OnClickSmart(CInstanceBase& rkInstMain, bool isAuto)
{
	DWORD dwPickedItemID;
	DWORD dwPickedActorID;
	TPixelPosition kPPosPickedGround;
	if (__GetPickedItemID(&dwPickedItemID))
	{
		__OnClickItem(rkInstMain, dwPickedItemID);
	}
	else if (__GetPickedActorID(&dwPickedActorID))
	{
		__OnClickActor(rkInstMain, dwPickedActorID, isAuto);
	}
	else if (__GetPickedGroundPos(&kPPosPickedGround))
	{
		__OnClickGround(rkInstMain, kPPosPickedGround);
	}
	else
	{
		rkInstMain.NEW_Stop();
	}
}

2* Reaplce this two functions with:
void CPythonPlayer::__OnPressSmart(CInstanceBase &rkInstMain, bool isAuto)
{
	DWORD dwPickedItemID;
	DWORD dwPickedActorID;
	TPixelPosition kPPosPickedGround;
	bool isPickedItemID = __GetPickedItemID(&dwPickedItemID);
	bool isPickedActorID = __GetPickedActorID(&dwPickedActorID);
	bool isPickedGroundPos = __GetPickedGroundPos(&kPPosPickedGround);
#ifdef __ENABLE_NEW_OFFLINESHOP__
#	ifdef __ENABLE_NEW_SHOP_IN_CITIES__
	DWORD dwPickedShopVID = 0;
#	endif
#endif

	if (isPickedItemID)
	{
		__OnPressItem(rkInstMain, dwPickedItemID);
	}
	else if (isPickedActorID && dwPickedActorID != GetMainCharacterIndex())
	{
		__OnPressActor(rkInstMain, dwPickedActorID, isAuto);
	}
#ifdef __ENABLE_NEW_OFFLINESHOP__
#	ifdef __ENABLE_NEW_SHOP_IN_CITIES__
	else if (CPythonTextTail::instance().GetPickedNewShop(&dwPickedShopVID))
	{
		return;
	}
#	endif
#endif
	else if (isPickedGroundPos)
	{
		__OnPressGround(rkInstMain, kPPosPickedGround);
	}
	else
	{
		__OnPressScreen(rkInstMain);
	}
}

void CPythonPlayer::__OnClickSmart(CInstanceBase &rkInstMain, bool isAuto)
{
	DWORD dwPickedItemID;
	DWORD dwPickedActorID;
	TPixelPosition kPPosPickedGround;
#ifdef __ENABLE_NEW_OFFLINESHOP__
#	ifdef __ENABLE_NEW_SHOP_IN_CITIES__
	DWORD dwPickedShopVID = 0;
#	endif
#endif

	if (__GetPickedItemID(&dwPickedItemID))
	{
		__OnClickItem(rkInstMain, dwPickedItemID);
	}
	else if (__GetPickedActorID(&dwPickedActorID))
	{
		__OnClickActor(rkInstMain, dwPickedActorID, isAuto);
	}
#ifdef __ENABLE_NEW_OFFLINESHOP__
#	ifdef __ENABLE_NEW_SHOP_IN_CITIES__
	else if (CPythonTextTail::instance().GetPickedNewShop(&dwPickedShopVID))
	{
		CPythonNetworkStream::instance().SendOfflineshopOnClickShopEntity(dwPickedShopVID);
		CPythonOfflineshop::instance().EnableGuiRefreshSymbol();
		OFFSHOP_DEBUG("is picked %u ", dwPickedShopVID);
	}
#	endif
#endif
	else if (__GetPickedGroundPos(&kPPosPickedGround))
	{
		__OnClickGround(rkInstMain, kPPosPickedGround);
	}
	else
	{
		rkInstMain.NEW_Stop();
	}
}