1* Search for:
import uiCommon

2* Add below:
if app.__ENABLE_NEW_OFFLINESHOP__:
	import uiOfflineshop

3* Search for:
				SrcSlotNumber = mouseModule.mouseController.GetAttachedSlotNumber()

4* Add above:
				if app.__ENABLE_NEW_OFFLINESHOP__:
					if uiOfflineshop.IsBuildingShop() and uiOfflineshop.IsSaleSlot(attachedInvenType, mouseModule.mouseController.GetRealAttachedSlotNumber()):
						chat.AppendChat(chat.CHAT_TYPE_INFO, localeInfo.OFFLINESHOP_CANT_SELECT_ITEM_DURING_BUILING)
						return