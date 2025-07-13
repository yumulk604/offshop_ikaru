1* Search for:
import quest

2* Add below:
if app.__ENABLE_NEW_OFFLINESHOP__:
	import uiOfflineshop
	import offlineshop

3* Search for:
		self.pressNumber = None

3* Add below:
		if app.__ENABLE_NEW_OFFLINESHOP__:
			self.Offlineshop = None

4* Search for:
		event.SetLeftTimeString(localeInfo.UI_LEFT_TIME)

5* Add below:
		if app.__ENABLE_NEW_OFFLINESHOP__:
			offlineshop.HideShopNames()

6* Search for:
		self.currentCubeNPC = 0

7* Add below:
		if app.__ENABLE_NEW_OFFLINESHOP__:
			self.Offlineshop = uiOfflineshop.NewOfflineShopBoard()
			self.Offlineshop.Hide()		

8* Search for:
	def RecvWhisper(self, name):
		self.interface.RecvWhisper(name)

9* Add below:
	if app.__ENABLE_NEW_OFFLINESHOP__:
		def ShowMeOfflineShop(self):
			if self.Offlineshop:
				if not self.Offlineshop.IsShow():
					self.Offlineshop.Open()
				else:
					self.Offlineshop.Close()

10* Search for:
	def ShowName(self):

11* Add below:
		if app.__ENABLE_NEW_OFFLINESHOP__:
			offlineshop.ShowShopNames()
			
12* Search for:
	def HideName(self):

13* Add below:
		if app.__ENABLE_NEW_OFFLINESHOP__:
			offlineshop.HideShopNames()


14* Search for:
			attachedItemCount = mouseModule.mouseController.GetAttachedItemCount()

15* Add below:
			if app.__ENABLE_NEW_OFFLINESHOP__:
				if uiOfflineshop.IsBuildingShop() and uiOfflineshop.IsSaleSlot(player.SlotTypeToInvenType(attachedType), attachedItemSlotPos):
					chat.AppendChat(chat.CHAT_TYPE_INFO, localeInfo.OFFLINESHOP_CANT_SELECT_ITEM_DURING_BUILING)
					return

16* Search for:
		self.serverCommander=stringCommander.Analyzer()

17* Add above:
		if app.__ENABLE_NEW_OFFLINESHOP__:
			serverCommandList["ShowMeOfflineShop"] = self.ShowMeOfflineShop