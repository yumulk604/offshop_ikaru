1* Search for:
import uiCommon

2* Add below:
if app.__ENABLE_NEW_OFFLINESHOP__:
	import offlineshop
	import uiOfflineshop

3* Search for:
	def OnPickItem(self, count):
		itemSlotIndex = self.dlgPickMoney.itemGlobalSlotIndex

4* Add below:
		if app.__ENABLE_NEW_OFFLINESHOP__:
			if uiOfflineshop.IsBuildingShop() and uiOfflineshop.IsSaleSlot(player.INVENTORY, itemSlotIndex):
				chat.AppendChat(chat.CHAT_TYPE_INFO, localeInfo.OFFLINESHOP_CANT_SELECT_ITEM_DURING_BUILING)
				return

5* Search for:
	def SellItem(self):

6* Add below:
		if app.__ENABLE_NEW_OFFLINESHOP__:
			if uiOfflineshop.IsBuildingShop() and uiOfflineshop.IsSaleSlot(player.INVENTORY, self.sellingSlotNumber):
				chat.AppendChat(chat.CHAT_TYPE_INFO, localeInfo.OFFLINESHOP_CANT_SELECT_ITEM_DURING_BUILING)
				return

7* Search for:
			attachedItemIndex = mouseModule.mouseController.GetAttachedItemIndex()

8* Add below:
			if app.__ENABLE_NEW_OFFLINESHOP__:
				if uiOfflineshop.IsBuildingShop() and uiOfflineshop.IsSaleSlot(player.SlotTypeToInvenType(attachedSlotType),attachedSlotPos):
					chat.AppendChat(chat.CHAT_TYPE_INFO, localeInfo.OFFLINESHOP_CANT_SELECT_ITEM_DURING_BUILING)
					return

9* Search for:
	def __DropSrcItemToDestItemInInventory(self, srcItemVID, srcItemSlotPos, dstItemSlotPos):
		if srcItemSlotPos == dstItemSlotPos:
			return

10* Add below:
		if app.__ENABLE_NEW_OFFLINESHOP__:
			if uiOfflineshop.IsBuildingShop() and (uiOfflineshop.IsSaleSlot(player.INVENTORY, srcItemSlotPos) or uiOfflineshop.IsSaleSlot(player.INVENTORY , dstItemSlotPos)):
				chat.AppendChat(chat.CHAT_TYPE_INFO, localeInfo.OFFLINESHOP_CANT_SELECT_ITEM_DURING_BUILING)
				return

11* Search for:
	def RefineItem(self, scrollSlotPos, targetSlotPos):

12* Add below:
		if app.__ENABLE_NEW_OFFLINESHOP__:
			if uiOfflineshop.IsBuildingShop():
				chat.AppendChat(chat.CHAT_TYPE_INFO, localeInfo.OFFLINESHOP_CANT_SELECT_ITEM_DURING_BUILING)
				return

13* Search for:
	def ShowToolTip(self, slotIndex):
		if None != self.tooltipItem:
			self.tooltipItem.SetInventoryItem(slotIndex)

14* Replace the function with:
	def ShowToolTip(self, slotIndex):
		if None != self.tooltipItem:
			self.tooltipItem.SetInventoryItem(slotIndex)
			if app.__ENABLE_NEW_OFFLINESHOP__:
				if uiOfflineshop.IsBuildingShop() or uiOfflineshop.IsBuildingAuction():
					self.__AddTooltipSaleMode(slotIndex)

	if app.__ENABLE_NEW_OFFLINESHOP__:
		def __AddTooltipSaleMode(self, slot):
			if player.IsEquipmentSlot(slot):
				return

			itemIndex = player.GetItemIndex(slot)
			if itemIndex !=0:
				item.SelectItem(itemIndex)
				if item.IsAntiFlag(item.ANTIFLAG_MYSHOP) or item.IsAntiFlag(item.ANTIFLAG_GIVE):
					return

				self.tooltipItem.AddRightClickForSale()

15* Search for:
	def UseItemSlot(self, slotIndex):

16* Add below:
		if app.__ENABLE_NEW_OFFLINESHOP__:
			if uiOfflineshop.IsBuildingShop():
				globalSlot 	= self.__InventoryLocalSlotPosToGlobalSlotPos(slotIndex)
				itemIndex 	= player.GetItemIndex(globalSlot)
				item.SelectItem(itemIndex)
				if not item.IsAntiFlag(item.ANTIFLAG_GIVE) and not item.IsAntiFlag(item.ANTIFLAG_MYSHOP):
					offlineshop.ShopBuilding_AddInventoryItem(globalSlot)
				else:
					chat.AppendChat(chat.CHAT_TYPE_INFO, localeInfo.OFFLINESHOP_CANT_SELECT_ITEM_DURING_BUILING)
				return

			elif uiOfflineshop.IsBuildingAuction():
				globalSlot = self.__InventoryLocalSlotPosToGlobalSlotPos(slotIndex)
				itemIndex = player.GetItemIndex(globalSlot)
				item.SelectItem(itemIndex)
				if not item.IsAntiFlag(item.ANTIFLAG_GIVE) and not item.IsAntiFlag(item.ANTIFLAG_MYSHOP):
					offlineshop.AuctionBuilding_AddInventoryItem(globalSlot)
				else:
					chat.AppendChat(chat.CHAT_TYPE_INFO, localeInfo.OFFLINESHOP_CANT_SELECT_ITEM_DURING_BUILING)
				return

		curCursorNum = app.GetCursor()
		if app.SELL == curCursorNum:
			return

17* Search for:
	def __UseItem(self, slotIndex):

18* Add below:
		if app.__ENABLE_NEW_OFFLINESHOP__:
			if uiOfflineshop.IsBuildingShop() and uiOfflineshop.IsSaleSlot(player.INVENTORY, slotIndex):
				chat.AppendChat(chat.CHAT_TYPE_INFO, localeInfo.OFFLINESHOP_CANT_SELECT_ITEM_DURING_BUILING)
				return