1* Search for:
import localeInfo

2* Add below:
if app.__ENABLE_NEW_OFFLINESHOP__:
	import uiOfflineshop

3* Search for:
				attachedInvenType = player.SlotTypeToInvenType(attachedSlotType)

4* Add below:
				if app.__ENABLE_NEW_OFFLINESHOP__:
					if uiOfflineshop.IsBuildingShop() and uiOfflineshop.IsSaleSlot(attachedInvenType, attachedSlotPos):
						chat.AppendChat(chat.CHAT_TYPE_INFO, localeInfo.OFFLINESHOP_CANT_SELECT_ITEM_DURING_BUILING)
						return