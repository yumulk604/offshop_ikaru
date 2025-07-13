1* Search for:
import localeInfo

2* Add below:
if app.__ENABLE_NEW_OFFLINESHOP__:
	import uiOfflineshop

3* Search for:
	def AddItemData(self,

4* Add above:
	if app.__ENABLE_NEW_OFFLINESHOP__:
		def AddRightClickForSale(self):
			self.AppendTextLine(localeInfo.OFFLINESHOP_TOOLTIP_RIGHT_CLICK_FOR_SALE, uiOfflineshop.COLOR_TEXT_SHORTCUT)