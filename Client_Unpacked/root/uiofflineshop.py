import offlineshop
import ui
import app
import item
import player
import dbg
import uiCommon
import uiToolTip
import localeInfo
import ime
import snd
import uiPickMoney
import grp
import mouseModule

ENABLE_ITEM_WITHDRAW_QUESTION_SHOP_SAFEBOX = False

SUBTYPE_NOSET = 255
SEARCH_RESULT_LIMIT = 250
COLOR_TEXT_SHORTCUT = grp.GenerateColor(1.0,1.0,0.5,1.0)
YANG_PER_CHEQUE = 1000000000

ENABLE_CHEQUE_SYSTEM = 0
try:
	if app.ENABLE_CHEQUE_SYSTEM:
		ENABLE_CHEQUE_SYSTEM = 1
except:
	pass
	
def IsBuildingShop():
	interface = offlineshop.GetOfflineshopBoard()
	if not interface:
		return False
	return interface.IsBuildingShop()

def IsSaleSlot(win,slot):
	interface = offlineshop.GetOfflineshopBoard()
	if not interface:
		return False
	
	return interface.IsForSaleSlot(win,slot)

def IsForAuctionSlot(win,slot):
	interface = offlineshop.GetOfflineshopBoard()
	if not interface:
		return False

	return interface.IsForAuctionSlot(win, slot)

def IsBuildingAuction():
	interface = offlineshop.GetOfflineshopBoard()
	if not interface:
		return False

	return interface.IsBuildingAuction()

def PutsError(line):
	dbg.TraceError("offlineshop interface error : %s "%line)

def NumberToString(num):
	if num < 0:
		return "-" + NumberToString(-num)
	parts = []
	while num >= 1000:
		parts.insert(0,"%03d"%(num%1000))
		num = num//1000

	parts.insert(0,"%d"%num)
	return '.'.join(parts)

def GetDurationString(dur):
	days    = dur // (24 * 60)
	hours   = (dur//60)%24
	minutes = dur%60
	
	res = " "
	
	if days > 0:
		res += localeInfo.OFFLINESHOP_DAY_TEXT%days + " "
	
	if hours > 0:
		res += localeInfo.OFFLINESHOP_HOUR_TEXT%hours + " "
	
	if minutes > 0:
		res += localeInfo.OFFLINESHOP_MINUTE_TEXT%minutes
	
	if days == 0 and hours == 0 and minutes == 0:
		return localeInfo.OFFLINESHOP_MINUTE_TEXT%0
		
	return res

def MakeSlotInfo(window, slotIndex, yang, cheque=0):
	res = {}
	
	itemIndex = player.GetItemIndex(window, slotIndex)
	itemCount = player.GetItemCount(window, slotIndex)
	
	res["slot"]	 = slotIndex
	res["window"]= window
	res["vnum"]	 = itemIndex
	res["count"] = itemCount
	
	res["socket"]= {}
	res["attr"]  = {}

	try:
		if app.ENABLE_CHANGELOOK_SYSTEM:
			res['trans'] = player.GetItemTransmutation(window, slotIndex)
	except:
		pass

	if ENABLE_CHEQUE_SYSTEM:
		res['cheque'] = cheque
	
	for x in xrange(player.ATTRIBUTE_SLOT_MAX_NUM):
		attr = player.GetItemAttribute(window, slotIndex, x)
		
		res['attr'][x] = {}
		res['attr'][x]["type"]  = attr[0]
		res['attr'][x]["value"] = attr[1]
	
	for x in xrange(player.METIN_SOCKET_MAX_NUM):
		res['socket'][x] = player.GetItemMetinSocket(window, slotIndex, x)
	
	res["price"] = yang
	return res

def MakeOfferCancelButton():
	btn = ui.Button()
	btn.SetUpVisual("offlineshop/myoffers/deleteoffer_default.png")
	btn.SetDownVisual("offlineshop/myoffers/deleteoffer_down.png")
	btn.SetOverVisual("offlineshop/myoffers/deleteoffer_over.png")
	btn.Show()
	return btn

def MakeOfferViewImage(isView):
	flag = "0"
	if isView:
		flag="1"

	img = ui.ImageBox()
	img.LoadImage("offlineshop/myoffers/viewicon_%s.png"%flag)
	img.Show()
	return img

def MakeDefaultEmptySlot(event = None):
	slot = Slot(False)
	if event:
		slot.SetOnMouseLeftButtonUpEvent(event)
	return slot

def GetBestOfferPriceYang(lst):
	if not lst:
		return 0

	max =0
	for info in lst:
		if info['price_yang'] > max:
			max = info['price_yang']

	return max

def SortByDatetime(lst):
	def CustomSortByDatetime(a,b):
		def CmpFunc(a,b):
			if a > b:
				return -1
			if b > a:
				return 1
			return 0

		keys = ('year' , 'month' , 'day', 'hour', 'minute')
		for k in keys:
			if a[k] != b[k]:
				return CmpFunc(a[k] , b[k])
		return 0

	lst.sort(cmp=CustomSortByDatetime)

def SortOffersByPrice(lst):
	def CustomSortByOfferPrice(a,b):
		price_a = a.get('price_yang', 0)
		price_b = b.get('price_yang', 0)

		if price_a > price_b:
			return -1

		if price_b < price_b:
			return 1
		return 0

	lst.sort(cmp=CustomSortByOfferPrice)
	return lst

class TableWindow(ui.Window):
	
	def __init__(self, col, row, width, height, realw, realh):
		ui.Window.__init__(self)
		self.rows 			= row
		self.columns		= col

		self.width			= width
		self.height			= height

		ui.Window.SetSize(self, realw, realh)

	def __del__(self):
		ui.Window.__del__(self)
	

	def __GetCoordByPos(self, col,row, child):
		stepx = self.width/self.columns
		stepy = self.height/self.rows

		x = stepx * col
		y = stepy * row

		x += stepx/2 - child.GetWidth() /2
		y += stepy/2 - child.GetHeight()/2

		return x,y

	def SetTableElement(self, column , row, child= None):
		if self.rows <= row:
			PutsError("cannot SetTableElement on row %d"%row)
			return
		
		if self.columns <= column:
			PutsError("cannot SetTableElement on column %d (row %d )"%(column,row))
			return

		if child == None:
			return
		
		x,y = self.__GetCoordByPos(column, row, child)

		child.SetParent(self)
		child.SetPosition( x, y)
		child.Show()

class TableWindowWithScrollbar(TableWindow):

	SCROLLBAR_HORIZONTAL 	= 0
	SCROLLBAR_VERTICAL		= 1

	def __init__(self, w, h , columns, rows, scrollbar=0):
		htable = h if scrollbar == self.SCROLLBAR_VERTICAL else h-18
		wtable = w if scrollbar == self.SCROLLBAR_HORIZONTAL else w -18

		TableWindow.__init__(self , columns, rows , wtable, htable, w ,h)

		self.childrenDict 	= {}
		self.scrollbar 		= None
		self.rows_count 	= rows
		self.columns_count	= columns
		self.total_row		= 0
		self.elementCount	= 0

		self.defaultCreate  	= None
		self.defaultCreateArgs 	= None
		self.defaultChildren 	= []

		self.__loadScrollbar(scrollbar)
		self.ClearElement()
	
	def __del__(self):
		self.childrenDict = {}
		self.scrollbar = None
		self.defaultCreate = None
		self.defaultChildren = []
		TableWindow.__del__(self)
	
	def __loadScrollbar(self, scrollbar):

		if scrollbar == self.SCROLLBAR_VERTICAL:
			template = {
				'button1' 	: {
					'default' 	: 'offlineshop/scrollbar/vertical/button1_default.png',
					'over' 		: 'offlineshop/scrollbar/vertical/button1_over.png',
					'down' 		: 'offlineshop/scrollbar/vertical/button1_down.png',
				},
				'button2' 		: {
					'default'	: 'offlineshop/scrollbar/vertical/button2_default.png',
					'over'		: 'offlineshop/scrollbar/vertical/button2_over.png',
					'down'		: 'offlineshop/scrollbar/vertical/button2_down.png',
				},
				'middle'  		: {
					'default'	: 'offlineshop/scrollbar/vertical/middle_default.png',
					'over'		: 'offlineshop/scrollbar/vertical/middle_over.png',
					'down'		: 'offlineshop/scrollbar/vertical/middle_down.png',
				},
				'base'	  		: "offlineshop/scrollbar/vertical/base_image.png",
				'onscroll'		: self.__refreshViewList,
				'parent'		: self,
				'orientation'	: ui.CustomScrollBar.VERTICAL,
				'align'			: {'mode': ui.CustomScrollBar.RIGHT,},
			}

		else:
			template = {
				'button1' 	: {
					'default' 	: 'offlineshop/scrollbar/horizontal/button1_default.png',
					'over' 		: 'offlineshop/scrollbar/horizontal/button1_over.png',
					'down' 		: 'offlineshop/scrollbar/horizontal/button1_down.png',
				},
				'button2' 		: {
					'default'	: 'offlineshop/scrollbar/horizontal/button2_default.png',
					'over'		: 'offlineshop/scrollbar/horizontal/button2_over.png',
					'down'		: 'offlineshop/scrollbar/horizontal/button2_down.png',
				},
				'middle'  		: {
					'default'	: 'offlineshop/scrollbar/horizontal/middle_default.png',
					'over'		: 'offlineshop/scrollbar/horizontal/middle_over.png',
					'down'		: 'offlineshop/scrollbar/horizontal/middle_down.png',
				},
				'base'	  		: "offlineshop/scrollbar/horizontal/base_image.png",
				'onscroll'		: self.__refreshViewList,
				'parent'		: self,

				'orientation'	: ui.CustomScrollBar.HORIZONTAL,
				'align'			: {'mode':ui.CustomScrollBar.BOTTOM,},
			}
		
		self.scrollbar = ui.CustomScrollBar(template)
	
	def SetElement(self, column, row, child):
		if not row  in self.childrenDict:
			self.childrenDict[row] = {}
		
		self.childrenDict[row][column] = child
		self.__refreshViewList()
	

	def SetDefaultCreateChild(self, func, *args):
		self.defaultCreate = func
		self.defaultCreateArgs = args

	def __refreshViewList(self):
		if self.scrollbar.orientation == ui.CustomScrollBar.VERTICAL:
			if len(self.childrenDict.keys()) <= self.rows_count:
				self.scrollbar.Hide()
			else:
				self.scrollbar.Show()

			init_row = self.__getInitRow()

			for row, dct in self.childrenDict.items():
				for column, child in dct.items():
					if row < init_row or row >= init_row + self.rows_count:
						child.Hide()
						continue
					self.SetTableElement(column, row-init_row , child)
			
			
			#updated 25-01-2020 #topatch
			for s in self.defaultChildren:
				s.Destroy()
			
			self.defaultChildren = []
			if self.defaultCreate:
				for row in xrange(init_row, init_row + self.rows_count):
					for col in xrange(self.columns_count):
						if not row in self.childrenDict or not col in self.childrenDict[row]:

							if self.defaultCreateArgs:
								defaultChild = self.defaultCreate(*self.defaultCreateArgs)


							else:
								defaultChild = self.defaultCreate()

							defaultChild.Show()

							self.SetTableElement(col, row - init_row, defaultChild)
							self.defaultChildren.append(defaultChild)

		elif self.scrollbar.orientation == ui.CustomScrollBar.HORIZONTAL:
			if len(self.childrenDict.get(0, [])) <= self.columns_count:
				self.scrollbar.Hide()
			else:
				self.scrollbar.Show()

			init_column = self.__getInitColumn()
			end_column  = init_column+ self.columns_count

			for dct in self.childrenDict.values():
				for elm in dct.values():
					elm.Hide()

			for col in xrange(init_column, end_column):
				for row, dct in self.childrenDict.items():
					if dct.get(col, False):
						self.SetTableElement(col-init_column, row , dct[col])
						dct[col].Show()
			
			
			#updated 25-01-2020 #topatch
			for s in self.defaultChildren:
				s.Destroy()
			
			self.defaultChildren = []
			if self.defaultCreate:
				for row in xrange(0, self.rows_count):
					for col in xrange(init_column, end_column):
						if not row in self.childrenDict or not col in self.childrenDict[row]:
							defaultChild = self.defaultCreate(*self.defaultCreateArgs) if self.defaultCreateArgs else self.defaultCreate()
							defaultChild.Show()

							self.SetTableElement(col-init_column, row, defaultChild)
							self.defaultChildren.append(defaultChild)

	def __getInitRow(self):
		if not self.scrollbar.IsShow():
			return 0
		
		return int((len(self.childrenDict.keys()) - self.rows_count ) * self.scrollbar.GetPos())

	def __getInitColumn(self):
		if not self.scrollbar.IsShow():
			return 0

		return int((len(self.childrenDict.get(0,[])) - self.columns_count ) * self.scrollbar.GetPos())

	def __AdjustScrollbarStep(self):
		total_slots		= self.rows_count*self.columns_count
		extra_element 	= self.elementCount - (total_slots)
		
		if extra_element <= 0:
			return

		if self.scrollbar.orientation == ui.CustomScrollBar.HORIZONTAL:
			extra_col = int(extra_element // self.rows_count)
			if extra_element % self.rows_count != 0:
				extra_col += 1
			self.scrollbar.SetScrollStep(1.0 / float(extra_col+1))

		else:
			extra_row_count = int(extra_element/self.columns_count)
			if extra_element % self.columns_count != 0:
				extra_row_count += 1

			self.scrollbar.SetScrollStep(1.0/float(extra_row_count+1))

	def ClearElement(self):
		for a in self.childrenDict.values():
			for child in a.values():
				child.Hide()
		
		self.childrenDict = {}
		self.elementCount = 0
		self.__refreshViewList()

	def AddElement(self, child):
		normElement =  self.columns_count*self.rows_count

		if self.elementCount < normElement or self.scrollbar.orientation == ui.CustomScrollBar.VERTICAL:
			column 	= self.elementCount%self.columns_count
			row		= self.elementCount//self.columns_count

		else:
			index = self.elementCount - normElement
			row   = index % self.rows_count
			column= index //self.rows_count

			column += self.columns_count

		self.SetElement(column, row, child)
		self.elementCount += 1
		
		self.__AdjustScrollbarStep()
	
	def GetElementDict(self):
		return self.childrenDict

	if app.ENABLE_WHEEL_MOUSE:
		def OnRunMouseWheel(self, nLen):
			if self.scrollbar:
				self.scrollbar.OnRunMouseWheel(nLen)

class Slot(ui.Window):
	
	def __init__(self, isSold=False):
		ui.Window.__init__(self)

		self.background = None
		self.background_sold = None
		self.iconImage = None
		self.countText = None
		self.upgradeImage = None

		self.slotInfo = {}
		self.index = 0
		self.eventClick = None
		self.childrens = []

		self.__loadBackground(isSold)
		
		print("initializing Slot")
	
	#updated 25-01-2020 #topatch
	def __del__(self):
		self.__clear()
		ui.Window.__del__(self)
	
	#updated 25-01-2020 #topatch
	def __clear(self):
		self.background 		= None
		self.iconImage  		= None
		self.countText			= None
		self.upgradeImage		= None
		self.background_sold	= None
		
		ui.Window.SetOnMouseLeftButtonUpEvent(self, None)
		
		if self.background:
			self.background.SetOnMouseLeftButtonUpEvent(None)
		
		if self.iconImage:
			self.iconImage.SetOnMouseLeftButtonUpEvent(None)
		
		if self.countText:
			self.countText.SetOnMouseLeftButtonUpEvent(None)
		
		if self.upgradeImage:
			self.upgradeImage.SetOnMouseLeftButtonUpEvent(None)

		if self.background_sold:
			self.background_sold.SetOnMouseLeftButtonUpEvent(None)
	
		self.slotInfo 		= {}
		self.index			= 0
		self.eventClick		= None
		self.childrens		= []
	
	#updated 25-01-2020 #topatch
	def Destroy(self):
		self.__clear()
	
	
	def __loadBackground(self, isSold):
		bg = ui.ImageBox()
		bg.LoadImage("offlineshop/slot/base_image.png")
		bg.SetParent(self)
		bg.SetPosition(0,0)
		bg.Show()

		self.SetSize(bg.GetWidth(), bg.GetHeight())
		self.background = bg

		sold = ui.ImageBox()
		sold.LoadImage("offlineshop/slot/base_image_sold.png")
		sold.SetParent(self.background)
		sold.SetPosition(0, 0)

		if isSold:
			sold.Show()
		else:
			sold.Hide()

		self.background_sold = sold



	
	def SetInfo(self, info):
		self.slotInfo = info
		self.SetSlot(info["vnum"] , info["count"])
	
	def GetInfo(self):
		return self.slotInfo
	
	def SetSlot(self, itemvnum, itemcount):
		item.SelectItem(itemvnum)
		
		image 	= item.GetIconImageFileName()
		name	= item.GetItemName()
		
		self.__SetItemIconImage(image)
		self.__SetItemCount(itemcount)
		self.__SetUpgradeByName(name)
	
	
	def SetIndex(self, index):
		self.index = index
	
	
	def GetIndex(self):
		return self.index
	

	def SetSold(self, flag):
		if flag:
			self.background_sold.Show()
		else:
			self.background_sold.Hide()


	def __GetIconPosition(self, image):
		w = image.GetWidth()
		h = image.GetHeight()
		
		return (self.background.GetWidth()/2 - w/2,  self.background.GetHeight()/2 - h/2)
	
	def __SetItemIconImage(self, icon):
		image = ui.ImageBox()
		image.LoadImage(icon)
		image.SetParent(self.background)
		
		x,y = self.__GetIconPosition(image)
		
		image.SetPosition(x,y)
		image.Show()
		
		self.iconImage = image
	
	
	def __GetCountPosition(self):
		x,y = self.iconImage.GetLocalPosition()
		x += self.iconImage.GetWidth()/2
		y += self.iconImage.GetHeight()
		
		return (x,y)
	
	
	def __SetItemCount(self, count):
		if count <= 1:
			self.countText = None
			return
		
		countText = ui.TextLine()
		countText.SetParent(self.background)
		
		x,y = self.__GetCountPosition()
		countText.SetPosition(x,y)
		countText.SetHorizontalAlignCenter()
		countText.SetText("x"+str(count))
		countText.Show()
		
		self.countText = countText
	
	
	
	def __GetUpgradeImagePosition(self,img):
		w = img.GetWidth()
		h = img.GetHeight()
		
		bw = self.background.GetWidth()
		bh = self.background.GetHeight()
		
		return (bw - (w+5), bh-(h+5))
	
	
	def __SetUpgradeByName(self, name):
		name = name.strip()
		if len(name) > 2:
			if name[-2] == '+':
				if name[-1].isdigit():
					value = int(name[-1])
					
					upgrade = ui.ImageBox()
					upgrade.LoadImage("offlineshop/slot/upgrade/%d.png"%value)
					upgrade.SetParent(self.background)
					
					x,y = self.__GetUpgradeImagePosition(upgrade)
					upgrade.SetPosition(x,y)
					upgrade.Show()
					
					self.upgradeImage = upgrade
	
	
	def SetOnMouseLeftButtonUpEvent(self, event):
		self.eventClick = event
		ui.Window.SetOnMouseLeftButtonUpEvent(self, self.__OnClick)
		
		if self.background:
			self.background.SetOnMouseLeftButtonUpEvent(self.__OnClick)
		
		if self.iconImage:
			self.iconImage.SetOnMouseLeftButtonUpEvent(self.__OnClick)
		
		if self.countText:
			self.countText.SetOnMouseLeftButtonUpEvent(self.__OnClick)
		
		if self.upgradeImage:
			self.upgradeImage.SetOnMouseLeftButtonUpEvent(self.__OnClick)

		if self.background_sold:
			self.background_sold.SetOnMouseLeftButtonUpEvent(self.__OnClick)


	def __OnClick(self):
		if self.eventClick!=None:
			self.eventClick(self)
	
	
	def IsInSlot(self):
		if not self.IsShow():
			return False
		
		if self.IsIn():
			return True
		
		if self.background:
			if self.background.IsIn():
				return True
		
		if self.iconImage:
			if self.iconImage.IsIn():
				return True
		
		if self.countText:
			if self.countText.IsIn():
				return True
		
		if self.upgradeImage:
			if self.upgradeImage.IsIn():
				return True

		if self.background_sold:
			if self.background_sold.IsShow() and self.background_sold.IsIn():
				return True

		if self.childrens:
			for child in self.childrens:
				if child.IsIn():
					return True


		return False


	def AppendChild(self, child):
		child.SetParent(self.background)
		self.childrens.append(child)

class Offer(ui.Window):
	def __init__(self, info):
		ui.Window.__init__(self)

		self.bgImage = None
		self.guestName = None
		self.priceText = None
		self.acceptButton = None
		self.deleteButton = None
		self.itemText = None
		self.info = {}
		self.index = 0
		self.is_accept = 0
		self.acceptEvent = None
		self.deniedEvent = None

		self.info  		= info
		self.index 		= info["id"]
		self.is_accept	= info["is_accept"]

		self.__loadBackground()
		self.__loadButtons()
		self.SetGuestName(info['buyer_name'])
		self.SetPriceYang(info["price"])

	def __del__(self):

		self.bgImage = None
		self.guestName = None
		self.priceText = None
		self.acceptButton = None
		self.deleteButton = None
		self.itemText = None
		self.info = {}
		self.index = 0
		self.is_accept = 0
		self.acceptEvent = None
		self.deniedEvent = None

		ui.Window.__del__(self)
	
	def __loadBackground(self):
		bg = ui.ImageBox()
		bg.SetParent(self)
		bg.SetPosition(0,0)
		bg.LoadImage("offlineshop/offer/base_image.png")
		bg.Show()

		self.SetSize(bg.GetWidth(), bg.GetHeight())
		self.bgImage = bg

	def SetGuestName(self, name):
		text = ui.TextLine()
		text.SetParent(self.bgImage)
		text.SetPosition(77 , 0)
		text.SetHorizontalAlignCenter()
		text.Show()
		text.SetText(name)
		self.guestName = text

	def SetPriceYang(self, yang):
		text = ui.TextLine()
		text.SetParent(self.bgImage)
		text.SetPosition(355 , 0)
		# text.SetHorizontalAlignCenter()
		text.Show()
		text.SetText(localeInfo.NumberToMoneyString(yang))
		self.priceText = text

	def SetItemName(self, name):
		text = ui.TextLine()
		text.SetParent(self.bgImage)
		text.SetPosition(239 , 0)
		text.SetHorizontalAlignCenter()
		text.Show()
		text.SetText(name)
		self.itemText = text
	
	def __loadButtons(self):
		if self.info["is_accept"]:
			return
		#accept
		button = ui.Button()
		button.SetParent(self.bgImage)
		button.SetPosition(self.bgImage.GetWidth() - 50, 0)
		
		path = "offlineshop/offer/accept_%s.png"
		button.SetUpVisual(path%"default")
		button.SetOverVisual(path%"over")
		button.SetDownVisual(path%"down")
		button.Show()
		
		self.acceptButton = button

		button = ui.Button()
		button.SetParent(self.bgImage)
		button.SetPosition(self.bgImage.GetWidth() - 30, 0)
		
		path = "offlineshop/offer/cancel_%s.png"
		button.SetUpVisual(path%"default")
		button.SetOverVisual(path%"over")
		button.SetDownVisual(path%"down")
		button.Show()

		self.deleteButton = button

	def SetAcceptButtonEvent(self, event):
		if self.acceptButton:
			self.acceptButton.SAFE_SetEvent(self.__OnAccept)
		self.acceptEvent = event
		
	def SetDeleteButtonEvent(self, event):
		if self.deleteButton:
			self.deleteButton.SAFE_SetEvent(self.__OnCancel)
		self.deniedEvent = event

	def GetInfo(self):
		return self.info

	def __OnAccept(self):
		if self.acceptEvent:
			self.acceptEvent(self)

	def __OnCancel(self):
		if self.deniedEvent:
			self.deniedEvent(self)

class AuctionOffer(ui.Window):
	def __init__(self, info):
		ui.Window.__init__(self)

		self.background = None
		self.ownerName = None
		self.offerText = None

		self.__LoadWindow(info)

	def __del__(self):
		self.background = None
		self.ownerName = None
		self.offerText = None
		ui.Window.__del__(self)

	def __LoadWindow(self, info):
		name = info["buyer_name"]
		best_yang = info['price_yang']

		self.__LoadBackground()
		self.__LoadOwnerName(name)
		self.__LoadOfferText(best_yang)

	def GetInfo(self):
		return self.info

	def __LoadBackground(self):
		bg = ui.ImageBox()
		bg.LoadImage("offlineshop/shoplist/shoplist_element_default.png")
		bg.SetParent(self)
		bg.SetPosition(0, 0)
		bg.Show()

		self.SetSize(bg.GetWidth(), bg.GetHeight())
		self.background = bg

	def __LoadOwnerName(self, ownerName):
		if not ownerName:
			return

		text = ui.TextLine()
		text.SetParent(self.background)
		text.SetPosition(186, 1)
		text.SetHorizontalAlignCenter()
		text.SetText(ownerName)
		text.Show()

		self.ownerName = text

	def __LoadOfferText(self, yang):
		text = ui.TextLine()
		text.SetParent(self.background)
		text.SetPosition(403, 1)
		text.SetHorizontalAlignCenter()
		text.SetText(localeInfo.NumberToMoneyString(yang))
		text.Show()

		self.offerText = text

class MyOffer(ui.Window):
	def __init__(self, info):
		ui.Window.__init__(self)
		self.info = info
		self.clear()

		self.__LoadBackground()
		self.__LoadOwnerName()
		self.__LoadOfferText()
		self.__LoadSlot()
		self.__LoadCancelButton()

	def clear(self):
		self.background   		= None
		self.ownerName    		= None
		self.offerText    		= None
		self.slot		  		= None
		self.cancelButton 		= None
		self.cancelButtonEvent	= None

	def __del__(self):
		self.clear()
		ui.Window.__del__(self)

	def __LoadBackground(self):
		bg = ui.ImageBox()
		bg.SetParent(self)
		bg.SetPosition(0,0)
		bg.LoadImage("offlineshop/myoffers/offer_base_image.png")
		bg.Show()

		self.SetSize(bg.GetWidth(), bg.GetHeight())
		self.background = bg

	def __LoadOwnerName(self):
		text = ui.TextLine()
		text.SetParent(self.background)
		text.SetPosition(175, 38)
		text.SetHorizontalAlignCenter()
		text.SetText(self.info.get('shop_name', ""))
		text.Show()

		self.ownerName = text

	def __LoadOfferText(self):
		text = ui.TextLine()
		text.SetParent(self.background)
		text.SetPosition(175, 60)
		text.SetHorizontalAlignCenter()
		text.SetText(NumberToString(self.info.get('price', "")))
		text.Show()

		self.offerText = text

	def __LoadSlot(self):
		slot = Slot(self.info['is_accept'])
		slot.SetInfo(self.info['item'])
		slot.SetParent(self.background)
		slot.SetPosition(9,4)
		slot.Show()

		self.slot = slot

	def __LoadCancelButton(self):
		button = ui.Button()
		button.SetParent(self.background)
		button.SetPosition(self.GetWidth()/2, 46 + 40)

		button.SetUpVisual("d:/ymir work/ui/public/small_button_01.sub")
		button.SetDownVisual("d:/ymir work/ui/public/small_button_03.sub")
		button.SetOverVisual("d:/ymir work/ui/public/small_button_02.sub")

		button.SetText(localeInfo.OFFLINESHOP_MYOFFERS_CANCEL_BUTTON_TEXT)

		button.Show()
		button.SAFE_SetEvent(self.__OnClickCancelButton)
		self.cancelButton = button

	def SetCancelButtonEvent(self, event):
		self.cancelButtonEvent = event

	def __OnClickCancelButton(self):
		if self.cancelButtonEvent:
			self.cancelButtonEvent(self.info['offer_id'])

	def IsInSlot(self):
		return self.slot.IsInSlot()


	def GetIndex(self):
		return self.info['offer_id']

	#updated 25-01-2020 #topatch
	def DisableCancelButton(self):
		if self.cancelButton:
			self.cancelButton.Hide()

class AuctionListElement(ui.Window):
	def __init__(self, info):
		ui.Window.__init__(self)

		self.ownerName = None
		self.durationText = None
		self.offerCountText = None
		self.owner_id = -1
		self.info = info
		self.button = None
		self.buttonEvent = None
		self.bestYangText = None

		self.__LoadWindow(info)

	def __del__(self):
		self.ownerName = None
		self.durationText = None
		self.offerCountText = None
		self.owner_id = -1
		self.info = {}
		self.button = None
		self.buttonEvent = None
		self.bestYangText = None
		ui.Window.__del__(self)

	def __LoadWindow(self, info):
		owner_id 	= info["owner_id"]
		duration 	= info["duration"]
		count 		= info["offer_count"]
		name 		= info["owner_name"]
		best_yang	= info['best_yang']

		self.owner_id = owner_id

		self.__LoadOpenAuctionButton()
		self.__LoadOwnerName(name)
		self.__LoadDuration(duration)
		self.__LoadCount(count)
		self.__LoadBestYang(best_yang)

	def SetIndex(self,index):
		self.owner_id = index

	def GetIndex(self):
		return self.owner_id

	def GetInfo(self):
		return self.info

	def SetOnClickOpenShopButton(self, event):
		self.openShopButton.SAFE_SetEvent(self.__OnClickOpenShop)
		self.openShopButtonEvent = event

	def __LoadOwnerName(self, ownerName):
		if not ownerName:
			return

		text = ui.TextLine()
		text.SetParent(self.button)
		text.SetPosition(85, 0)
		text.SetHorizontalAlignCenter()
		text.SetText(ownerName)
		text.Show()

		self.ownerName = text

	def __LoadDuration(self, duration):
		text = ui.TextLine()
		text.SetParent(self.button)
		text.SetPosition(344, 0)
		text.SetHorizontalAlignCenter()
		text.SetText(GetDurationString(duration))
		text.Show()

		self.durationText = text

	def __LoadCount(self, count):
		text = ui.TextLine()
		text.SetParent(self.button)
		text.SetPosition(480, 0)
		text.SetHorizontalAlignCenter()
		text.SetText(str(count))
		text.Show()

		self.offerCountText = text

	def __LoadBestYang(self, yang):
		text = ui.TextLine()
		text.SetParent(self.button)
		text.SetPosition(217, 0)
		text.SetHorizontalAlignCenter()
		text.SetText(localeInfo.NumberToMoneyString(yang))
		text.Show()

		self.bestYangText = text

	def SetOnClickOpenAuctionButton(self, event):
		self.button.SAFE_SetEvent(self.__OnClickMe)
		self.buttonEvent = event

	def __OnClickMe(self):
		if self.buttonEvent:
			self.buttonEvent(self.owner_id)

	def __LoadOpenAuctionButton(self):
		button = ui.Button()
		button.SetParent(self)
		button.SetUpVisual("offlineshop/shoplist/shoplist_element_default.png")
		button.SetDownVisual("offlineshop/shoplist/shoplist_element_down.png")
		button.SetOverVisual("offlineshop/shoplist/shoplist_element_over.png")

		button.SetPosition(0, 0)
		button.Show()
		self.SetSize(button.GetWidth(), button.GetHeight())
		self.button = button

	def IsInSlot(self):
		if self.IsIn():
			return True

		if self.ownerName:
			if self.ownerName.IsIn():
				return True

		if self.durationText:
			if self.durationText.IsIn():
				return True

		if self.offerCountText:
			if self.offerCountText.IsIn():
				return True

		if self.button:
			if self.button.IsIn():
				return True

		return False

class ShopListElement(ui.Window):
	def __init__(self , info):
		ui.Window.__init__(self)

		self.ownerName = None
		self.shopName = None
		self.durationText = None
		self.countText = None
		self.openShopButton = None
		self.openShopButtonEvent = None
		self.owner_id = -1

		self.__LoadWindow(info)

	def __del__(self):
		self.ownerName = None
		self.shopName = None
		self.durationText = None
		self.countText = None
		self.openShopButton = None
		self.openShopButtonEvent = None

		self.owner_id = -1
		ui.Window.__del__(self)

	def __LoadWindow(self, info):
		owner_id	= info["owner_id"]
		duration	= info["duration"]
		count		= info["count"]
		name		= info["name"]
		
		ownerName	= name[:name.find('@')] if '@' in name else "NONAME"
		name		= name[name.find('@')+1:] if '@' in name else name
		
		self.owner_id  = owner_id
		
		self.__LoadOpenShopButton()
		self.__LoadOwnerName(ownerName)
		self.__LoadShopName(name)
		self.__LoadDuration(duration)
		self.__LoadCount(count)

	def SetOnClickOpenShopButton(self, event):
		self.openShopButton.SAFE_SetEvent(self.__OnClickOpenShop)
		self.openShopButtonEvent = event

	def __OnClickOpenShop(self):
		if self.openShopButtonEvent:
			self.openShopButtonEvent(self.owner_id)

	def __LoadOpenShopButton(self):
		button = ui.Button()
		button.SetParent(self)
		button.SetUpVisual("offlineshop/shoplist/shoplist_element_default.png")
		button.SetDownVisual("offlineshop/shoplist/shoplist_element_down.png")
		button.SetOverVisual("offlineshop/shoplist/shoplist_element_over.png")

		button.SetPosition(0, 0)
		button.Show()

		self.SetSize(button.GetWidth(), button.GetHeight())
		self.openShopButton = button

	def __LoadOwnerName(self ,ownerName):
		if not ownerName:
			return
		
		text = ui.TextLine()
		text.SetParent(self.openShopButton)
		text.SetPosition(48 , 2)
		text.SetHorizontalAlignCenter()
		text.SetText(ownerName)
		text.Show()
		
		self.ownerName = text

	def __LoadShopName(self ,name):
		text = ui.TextLine()
		text.SetParent(self.openShopButton)
		text.SetPosition(198 , 2)
		text.SetHorizontalAlignCenter()
		text.SetText(name)
		text.Show()
		
		self.shopName = text

	def __LoadDuration(self ,duration):
		text = ui.TextLine()
		text.SetParent(self.openShopButton)
		text.SetPosition(358 , 2)
		text.SetHorizontalAlignCenter()
		text.SetText(GetDurationString(duration))
		text.Show()
		
		self.durationText = text
	
	def __LoadCount(self ,count):
		text = ui.TextLine()
		text.SetParent(self.openShopButton)
		text.SetPosition(480 , 2)
		text.SetHorizontalAlignCenter()
		text.SetText(str(count))
		text.Show()
		
		self.countText = text

class Suggestions():
	def __init__(self):
		self.inputBox = None
		self.comboBox = None

		self.mainDict = {}
		self.tempDict = {}
		self.isRefreshing = False
		self.isSelecting = False
	
	def SetInputBox(self, box):
		box.OnIMEUpdate = self.__OnUpdateInputBox
		self.inputBox = box
	
	def SetComboBox(self, box):
		box.SetEvent(ui.__mem_func__(self.__OnSelectItem))
		box.ClearItem()
		box.SetCurrentItem(localeInfo.OFFLINESHOP_NAME_SUGGESTION_UNSELECT)
		
		self.comboBox = box
	
	def SetMainDict(self, inDict):
		def getUnrefined(st):
			pos = st.find('+')
			if pos == -1:
				return st
			
			if pos > len(st) -4 and pos > 4:
				return st[:pos]
			return st
		
		cleanDict = {}
		inserted  = []
		
		for name , vnum in inDict.items():
			unrefined = getUnrefined(name).strip()
			if unrefined in inserted:
				continue
			
			cleanDict[unrefined] = vnum
			inserted.append(unrefined)
		
		self.mainDict = cleanDict
	
	def __OnUpdateInputBox(self):
		snd.PlaySound("sound/ui/type.wav")
		ui.TextLine.SetText(self.inputBox, ime.GetText(self.inputBox.bCodePage))
		
		self.__RefreshComboBox()
	
	def __RefreshComboBox(self):
		if self.isSelecting:
			return
		
		self.isRefreshing = True
		
		self.tempDict 	= {}
		inputText		= self.inputBox.GetText().strip()
		
		if len(inputText) < 3:
			return
		
		self.comboBox.ClearItem()
		self.comboBox.SetCurrentItem(localeInfo.OFFLINESHOP_NAME_SUGGESTION_UNSELECT)
		
		
		idx = 0
		for k,v in self.mainDict.items():
			if inputText in k:
				self.tempDict[idx] = k
				self.comboBox.InsertItem(idx, k)
				
				idx += 1
				
				if idx == 20:
					break
		
		self.isRefreshing = False
	
	def __OnSelectItem(self, index):
		if self.isRefreshing:
			return
		
		self.isSelecting = True
		self.inputBox.SetText(self.tempDict[index])
		self.isSelecting = False
	
	def __del__(self):
		self.inputBox		= None
		self.comboBox	= None
		
		self.mainDict	= {}
		self.tempDict	= {}
	
	def Clear(self):
		self.comboBox.ClearItem()
		self.comboBox.SetCurrentItem(localeInfo.OFFLINESHOP_NAME_SUGGESTION_UNSELECT)

class SuggestionElement(ui.Button):
	def __init__(self):
		self.clickEvent = None
		self.index = 0

		ui.Button.__init__(self)
	
	def __del__(self):
		self.clickEvent = None
		ui.Button.__del__(self)
	
	def SetClickEvent(self, event):
		self.clickEvent = event
		self.SAFE_SetEvent(self.__OnClickMe)
	
	def __OnClickMe(self):
		if self.clickEvent:
			self.clickEvent(self.index)
	
	
	def SetElement(self, index, text):
		self.index = index
		self.SetText(text)

class SuggestionSelector(ui.Window):
	def __init__(self):
		ui.Window.__init__(self)

		self.scrollbar = None
		self.background = None
		self.attributeDict = {}
		self.onSelectEvent = None

		self.elements = []

		self.__loadBackground()
		self.__loadElements()
		self.__loadScrollbar()
	
	def __loadBackground(self):
		bg = ui.ImageBox()
		bg.LoadImage("offlineshop/searchfilter/attribute_selector_base_image.png")
		bg.SetParent(self)
		bg.SetPosition(0,0)
		
		bg.Show()
		self.background = bg
		self.SetSize(bg.GetWidth() , bg.GetHeight())
	
	def __loadElements(self):
		for x in xrange(8):
			element = SuggestionElement()
			element.SetParent(self.background)
			element.SetPosition(0, x * 16)
			
			element.SetUpVisual("offlineshop/searchfilter/attribute_default.png")
			element.SetDownVisual("offlineshop/searchfilter/attribute_down.png")
			element.SetOverVisual("offlineshop/searchfilter/attribute_over.png")
			
			element.Show()
			
			element.SetClickEvent(self.__OnSelectAttribute)
			self.elements.append(element)
	
	def __loadScrollbar(self):
		scroll = ui.ScrollBar()
		scroll.SetParent(self.background)
		scroll.SetPosition(self.GetWidth()-10, 0)
		scroll.SetScrollBarSize(self.GetHeight())
		scroll.SetScrollEvent(self.__OnScroll)
		scroll.Show()
		
		self.scrollbar = scroll
	
	def __OnSelectAttribute(self, index):
		if self.onSelectEvent:
			self.onSelectEvent(index)

	def __OnScroll(self):
		self.__refreshViewList()
	
	
	def __refreshViewList(self):
		pos 		= self.scrollbar.GetPos()
		initIndex	= int(pos * (len(self.attributeDict) - len(self.elements) ))
		
		for x in xrange(initIndex , initIndex + len(self.elements)):
			index 	= self.attributeDict.keys()[x]
			text	= self.attributeDict[index]
			
			self.elements[x-initIndex].SetElement(index, text)

	def SetAttributeDict(self, dct):
		self.attributeDict = dct
		self.__refreshViewList()

	def SetSelectEvent(self, event):
		self.onSelectEvent = event

	def __del__(self):
		self.background 	= None
		self.elements		= []
		self.scrollbar		= None
		self.onSelectEvent	= None
		
		ui.Window.__del__(self)

	if app.ENABLE_WHEEL_MOUSE:
		def OnRunMouseWheel(self, nLen):
			if nLen > 0:
				self.scrollbar.OnUp()
			else:
				self.scrollbar.OnDown()	

class FilterHistoryElement(ui.Window):
	def __init__(self, info):
		ui.Window.__init__(self)

		self.button = None
		self.datetext = None
		self.timetext = None
		self.counttext = None
		self.buttonEvent = None
		self.info = {}

		self.__loadButton()
		self.__loadDateText(info)
		self.__loadTimeText(info)
		self.__loadCountText(info)
	
		self.info = info

	def GetInfo(self):
		return self.info

	def SetButtonEvent(self , event):
		self.buttonEvent = event
		self.button.SAFE_SetEvent(self.__OnClickMe)

	def __OnClickMe(self):
		if self.buttonEvent:
			self.buttonEvent(self)

	def __loadButton(self):
		button = ui.Button()
		button.SetParent(self)
		button.SetPosition(0,0)
		
		path = "offlineshop/searchhistory/element_%s.png"
		
		button.SetUpVisual(path%"default")
		button.SetDownVisual(path%"down")
		button.SetOverVisual(path%"over")
		button.Show()

		self.SetSize(button.GetWidth(), button.GetHeight())
		self.button = button
	
	def __loadDateText(self,info):
		text = ui.TextLine()
		text.SetParent(self.button)
		text.SetPosition(60, 3)
		
		day		= info["day"]
		month	= info["month"]
		year	= info["year"]
		
		text.SetText("%02d - %02d - %d "%(day, month, year))
		text.Show()

		self.datetext = text

	def __loadTimeText(self, info):
		text = ui.TextLine()
		text.SetParent(self.button)
		text.SetPosition(190+75, 3)

		hour 	= info["hour"]
		minute 	= info["minute"]

		text.SetText("%02d : %02d " % (hour, minute))
		text.Show()

		self.timetext = text
	
	def __loadCountText(self, info):
		text = ui.TextLine()
		text.SetParent(self.button)
		text.SetPosition(485, 3)

		count = info["count"]

		text.SetText(" %d " % (count))
		text.SetHorizontalAlignCenter()
		text.Show()

		self.counttext = text

	def __del__(self):
		self.button 	= None
		self.datetext	= None
		self.timetext	= None
		self.counttext	= None
		self.buttonEvent= None
		ui.Window.__del__(self)
	
	def IsInSlot(self):
		if not self.IsShow():
			return False

		if self.IsIn():
			return True
		
		if self.button:
			if self.button.IsIn():
				return True
		
		if self.datetext:
			if self.datetext.IsIn():
				return True
		
		if self.timetext:
			if self.timetext.IsIn():
				return True
		
		if self.counttext:
			if self.counttext.IsIn():
				return True
		
		return False

	def GetIndex(self):
		return self.info['id']

class FilterPatternElement(ui.Window):
	def __init__(self, info):
		ui.Window.__init__(self)

		self.button = None
		self.datetext = None
		self.nametext = None
		self.buttonEvent = None
		self.info = {}

		self.__loadButton()
		self.__loadDateText(info)
		self.__loadNameText(info)

		self.info = info

	def GetInfo(self):
		return self.info

	def SetButtonEvent(self , event):
		self.buttonEvent = event
		self.button.SAFE_SetEvent(self.__OnClickMe)

	def __OnClickMe(self):
		if self.buttonEvent:
			self.buttonEvent(self)

	def __loadButton(self):
		button = ui.Button()
		button.SetParent(self)
		button.SetPosition(0,0)

		path = "offlineshop/searchhistory/element_%s.png"

		button.SetUpVisual(path%"default")
		button.SetDownVisual(path%"down")
		button.SetOverVisual(path%"over")
		button.Show()

		self.SetSize(button.GetWidth(), button.GetHeight())
		self.button = button

	def __loadDateText(self,info):
		text = ui.TextLine()
		text.SetParent(self.button)
		text.SetPosition(352, 3)

		day		= info["day"]
		month	= info["month"]
		year	= info["year"]
		hour	= info["hour"]
		minute	= info["minute"]

		text.SetText("%02d - %02d - %d  %02d : %02d"%(day, month, year, hour, minute))
		text.Show()

		self.datetext = text


	def __loadNameText(self, info):
		text = ui.TextLine()
		text.SetParent(self.button)
		text.SetPosition(20, 3)

		text.SetText(info["name"])
		text.Show()

		self.nametext = text

	def __del__(self):
		self.button = None
		self.datetext = None
		self.nametext = None
		self.buttonEvent = None
		ui.Window.__del__(self)

	def IsInSlot(self):
		if not self.IsShow():
			return False

		if self.IsIn():
			return True

		if self.button:
			if self.button.IsIn():
				return True

		if self.datetext:
			if self.datetext.IsIn():
				return True

		if self.nametext:
			if self.nametext.IsIn():
				return True
		return False

	def GetIndex(self):
		return self.info['id']

class NewOfflineShopBoard(ui.ScriptWindow):
	BOARD_KEYS = ("create_shop" , "my_shop", "open_shop", "shop_list", "search_history", )
	
	ITEM_TYPES = {
		item.ITEM_TYPE_NONE		: {
			"name" 		: localeInfo.OFFLINESHOP_TYPE_COMBOBOX_NOSET,
		},
		
		item.ITEM_TYPE_WEAPON		: {
			"name" : localeInfo.OFFLINESHOP_TYPE_COMBOBOX_WEAPON,
			"subtypes"	: {
				item.WEAPON_SWORD			:	localeInfo.OFFLINESHOP_COMBOBOX_WEAPON_SWORD,
				item.WEAPON_DAGGER			:	localeInfo.OFFLINESHOP_COMBOBOX_WEAPON_DAGGER,
				item.WEAPON_BOW				:	localeInfo.OFFLINESHOP_COMBOBOX_WEAPON_BOW,
				item.WEAPON_TWO_HANDED		:	localeInfo.OFFLINESHOP_COMBOBOX_WEAPON_TWO_HANDED,
				item.WEAPON_BELL			:	localeInfo.OFFLINESHOP_COMBOBOX_WEAPON_BELL,
				item.WEAPON_FAN				:	localeInfo.OFFLINESHOP_COMBOBOX_WEAPON_FAN,
			},
		},
		
		item.ITEM_TYPE_ARMOR		: {
			"name" : localeInfo.OFFLINESHOP_TYPE_COMBOBOX_ARMOR,
			"subtypes"	: {
				item.ARMOR_BODY			:	localeInfo.OFFLINESHOP_COMBOBOX_ARMOR_BODY,
				item.ARMOR_HEAD			:	localeInfo.OFFLINESHOP_COMBOBOX_ARMOR_HEAD,
				item.ARMOR_SHIELD		:	localeInfo.OFFLINESHOP_COMBOBOX_ARMOR_SHIELD,
				item.ARMOR_WRIST		:	localeInfo.OFFLINESHOP_COMBOBOX_ARMOR_WRIST,
				item.ARMOR_FOOTS		:	localeInfo.OFFLINESHOP_COMBOBOX_ARMOR_FOOTS,
				item.ARMOR_NECK			:	localeInfo.OFFLINESHOP_COMBOBOX_ARMOR_NECK,
				item.ARMOR_EAR			:	localeInfo.OFFLINESHOP_COMBOBOX_ARMOR_EAR,
			},
		},
		
		item.ITEM_TYPE_METIN		: {
			"name" : localeInfo.OFFLINESHOP_TYPE_COMBOBOX_METIN,
		},
		
		item.ITEM_TYPE_FISH		: {
			"name" : localeInfo.OFFLINESHOP_TYPE_COMBOBOX_FISH,
		},
		
		item.ITEM_TYPE_SKILLBOOK		: {
			"name" : localeInfo.OFFLINESHOP_TYPE_COMBOBOX_SKILLBOOK,
		},
		
		item.ITEM_TYPE_BLEND		: {
			"name" : localeInfo.OFFLINESHOP_TYPE_COMBOBOX_BLEND,
		},
		
		item.ITEM_TYPE_DS		: {
			"name" : localeInfo.OFFLINESHOP_TYPE_COMBOBOX_DS,
		},
		
		item.ITEM_TYPE_RING		: {
			"name" : localeInfo.OFFLINESHOP_TYPE_COMBOBOX_RING,
		},
		
		item.ITEM_TYPE_BELT		: {
			"name" : localeInfo.OFFLINESHOP_TYPE_COMBOBOX_BELT,
		},


		item.ITEM_TYPE_METIN		: {
			"name" : localeInfo.OFFLINESHOP_TYPE_COMBOBOX_METIN,
		},

		item.ITEM_TYPE_GIFTBOX		: {
			"name" : localeInfo.OFFLINESHOP_TYPE_COMBOBOX_GIFTBOX,
		},

		item.ITEM_TYPE_COSTUME		: {
			"name" : localeInfo.OFFLINESHOP_TYPE_COMBOBOX_COSTUME,
			"subtypes"	: {
				item.COSTUME_TYPE_BODY			:	localeInfo.OFFLINESHOP_COMBOBOX_COSTUME_BODY,
				item.COSTUME_TYPE_HAIR			:	localeInfo.OFFLINESHOP_COMBOBOX_COSTUME_HAIR,
				item.COSTUME_TYPE_MOUNT			:	localeInfo.OFFLINESHOP_COMBOBOX_COSTUME_MOUNT,
				item.COSTUME_TYPE_SASH			:	localeInfo.OFFLINESHOP_COMBOBOX_COSTUME_ACCE,
				item.COSTUME_TYPE_WEAPON		:	localeInfo.OFFLINESHOP_COMBOBOX_COSTUME_WEAPON,
			},
		},

		item.ITEM_TYPE_MATERIAL		:{
			"name": localeInfo.OFFLINESHOP_TYPE_COMBOBOX_REFINE,
		},
	}
	
	ATTRIBUTES = {
		0	: localeInfo.OFFLINESHOP_ATTR_UNSET,
		
		item.APPLY_MAX_HP 				: localeInfo.OFFLINESHOP_ATTR_MAX_HP,
		item.APPLY_MAX_SP 				: localeInfo.OFFLINESHOP_ATTR_MAX_SP,
		item.APPLY_CON 					: localeInfo.OFFLINESHOP_ATTR_CON,
		item.APPLY_INT 					: localeInfo.OFFLINESHOP_ATTR_INT,
		item.APPLY_STR 					: localeInfo.OFFLINESHOP_ATTR_STR,
		item.APPLY_DEX 					: localeInfo.OFFLINESHOP_ATTR_DEX,
		item.APPLY_ATT_SPEED 			: localeInfo.OFFLINESHOP_ATTR_ATT_SPEED,
		item.APPLY_MOV_SPEED 			: localeInfo.OFFLINESHOP_ATTR_MOV_SPEED,
		item.APPLY_CAST_SPEED 			: localeInfo.OFFLINESHOP_ATTR_CAST_SPEED,
		item.APPLY_HP_REGEN 			: localeInfo.OFFLINESHOP_ATTR_HP_REGEN,
		item.APPLY_SP_REGEN 			: localeInfo.OFFLINESHOP_ATTR_SP_REGEN,
		item.APPLY_POISON_PCT 			: localeInfo.OFFLINESHOP_ATTR_APPLY_POISON_PCT,
		item.APPLY_STUN_PCT 			: localeInfo.OFFLINESHOP_ATTR_APPLY_STUN_PCT,
		item.APPLY_SLOW_PCT 			: localeInfo.OFFLINESHOP_ATTR_APPLY_SLOW_PCT,
		item.APPLY_CRITICAL_PCT 		: localeInfo.OFFLINESHOP_ATTR_APPLY_CRITICAL_PCT,
		item.APPLY_PENETRATE_PCT 		: localeInfo.OFFLINESHOP_ATTR_APPLY_PENETRATE_PCT,

		item.APPLY_ATTBONUS_WARRIOR 	: localeInfo.OFFLINESHOP_ATTR_APPLY_ATTBONUS_WARRIOR,
		item.APPLY_ATTBONUS_ASSASSIN 	: localeInfo.OFFLINESHOP_ATTR_APPLY_ATTBONUS_ASSASSIN,
		item.APPLY_ATTBONUS_SURA 		: localeInfo.OFFLINESHOP_ATTR_APPLY_ATTBONUS_SURA,
		item.APPLY_ATTBONUS_SHAMAN 		: localeInfo.OFFLINESHOP_ATTR_APPLY_ATTBONUS_SHAMAN,
		item.APPLY_ATTBONUS_MONSTER 	: localeInfo.OFFLINESHOP_ATTR_APPLY_ATTBONUS_MONSTER,

		item.APPLY_ATTBONUS_HUMAN 		: localeInfo.OFFLINESHOP_ATTR_APPLY_ATTBONUS_HUMAN,
		item.APPLY_ATTBONUS_ANIMAL 		: localeInfo.OFFLINESHOP_ATTR_APPLY_ATTBONUS_ANIMAL,
		item.APPLY_ATTBONUS_ORC 		: localeInfo.OFFLINESHOP_ATTR_APPLY_ATTBONUS_ORC,
		item.APPLY_ATTBONUS_MILGYO 		: localeInfo.OFFLINESHOP_ATTR_APPLY_ATTBONUS_MILGYO,
		item.APPLY_ATTBONUS_UNDEAD 		: localeInfo.OFFLINESHOP_ATTR_APPLY_ATTBONUS_UNDEAD,
		item.APPLY_ATTBONUS_DEVIL 		: localeInfo.OFFLINESHOP_ATTR_APPLY_ATTBONUS_DEVIL,
		item.APPLY_STEAL_HP 			: localeInfo.OFFLINESHOP_ATTR_APPLY_STEAL_HP,
		item.APPLY_STEAL_SP 			: localeInfo.OFFLINESHOP_ATTR_APPLY_STEAL_SP,
		item.APPLY_MANA_BURN_PCT 		: localeInfo.OFFLINESHOP_ATTR_APPLY_MANA_BURN_PCT,
		item.APPLY_DAMAGE_SP_RECOVER 	: localeInfo.OFFLINESHOP_ATTR_APPLY_DAMAGE_SP_RECOVER,
		item.APPLY_BLOCK 				: localeInfo.OFFLINESHOP_ATTR_APPLY_BLOCK,
		item.APPLY_DODGE 				: localeInfo.OFFLINESHOP_ATTR_APPLY_DODGE,
		item.APPLY_RESIST_SWORD 		: localeInfo.OFFLINESHOP_ATTR_APPLY_RESIST_SWORD,
		item.APPLY_RESIST_TWOHAND 		: localeInfo.OFFLINESHOP_ATTR_APPLY_RESIST_TWOHAND,
		item.APPLY_RESIST_DAGGER 		: localeInfo.OFFLINESHOP_ATTR_APPLY_RESIST_DAGGER,
		item.APPLY_RESIST_BELL 			: localeInfo.OFFLINESHOP_ATTR_APPLY_RESIST_BELL,
		item.APPLY_RESIST_FAN 			: localeInfo.OFFLINESHOP_ATTR_APPLY_RESIST_FAN,
		item.APPLY_RESIST_BOW 			: localeInfo.OFFLINESHOP_ATTR_RESIST_BOW,
		item.APPLY_RESIST_FIRE 			: localeInfo.OFFLINESHOP_ATTR_RESIST_FIRE,
		item.APPLY_RESIST_ELEC 			: localeInfo.OFFLINESHOP_ATTR_RESIST_ELEC,
		item.APPLY_RESIST_MAGIC 		: localeInfo.OFFLINESHOP_ATTR_RESIST_MAGIC,
		item.APPLY_RESIST_WIND 			: localeInfo.OFFLINESHOP_ATTR_APPLY_RESIST_WIND,
		item.APPLY_REFLECT_MELEE 		: localeInfo.OFFLINESHOP_ATTR_APPLY_REFLECT_MELEE,
		item.APPLY_POISON_REDUCE 		: localeInfo.OFFLINESHOP_ATTR_APPLY_POISON_REDUCE,
		item.APPLY_EXP_DOUBLE_BONUS 	: localeInfo.OFFLINESHOP_ATTR_APPLY_EXP_DOUBLE_BONUS,
		item.APPLY_GOLD_DOUBLE_BONUS 	: localeInfo.OFFLINESHOP_ATTR_APPLY_GOLD_DOUBLE_BONUS,
		item.APPLY_ITEM_DROP_BONUS 		: localeInfo.OFFLINESHOP_ATTR_APPLY_ITEM_DROP_BONUS,
		item.APPLY_IMMUNE_STUN 			: localeInfo.OFFLINESHOP_ATTR_APPLY_IMMUNE_STUN,
		item.APPLY_IMMUNE_SLOW 			: localeInfo.OFFLINESHOP_ATTR_APPLY_IMMUNE_SLOW,
		item.APPLY_IMMUNE_FALL 			: localeInfo.OFFLINESHOP_ATTR_APPLY_IMMUNE_FALL,
		item.APPLY_DEF_GRADE_BONUS 		: localeInfo.OFFLINESHOP_ATTR_DEF_GRADE,
		item.APPLY_ATT_GRADE_BONUS 		: localeInfo.OFFLINESHOP_ATTR_ATT_GRADE,
		item.APPLY_MAGIC_ATT_GRADE 		: localeInfo.OFFLINESHOP_ATTR_MAGIC_ATT_GRADE,
		item.APPLY_MAGIC_DEF_GRADE 		: localeInfo.OFFLINESHOP_ATTR_MAGIC_DEF_GRADE,
		item.APPLY_SKILL_DAMAGE_BONUS 	: localeInfo.OFFLINESHOP_ATTR_SKILL_DAMAGE_BONUS,

		item.APPLY_NORMAL_HIT_DAMAGE_BONUS 	: localeInfo.OFFLINESHOP_ATTR_NORMAL_HIT_DAMAGE_BONUS,
		item.APPLY_SKILL_DEFEND_BONUS 		: localeInfo.OFFLINESHOP_ATTR_SKILL_DEFEND_BONUS,
		item.APPLY_NORMAL_HIT_DEFEND_BONUS 	: localeInfo.OFFLINESHOP_ATTR_NORMAL_HIT_DEFEND_BONUS,

		item.APPLY_MAGIC_ATTBONUS_PER 		: localeInfo.OFFLINESHOP_ATTR_MAGIC_ATTBONUS_PER,
		item.APPLY_MELEE_MAGIC_ATTBONUS_PER : localeInfo.OFFLINESHOP_ATTR_MELEE_MAGIC_ATTBONUS_PER,
		item.APPLY_RESIST_ICE 				: localeInfo.OFFLINESHOP_ATTR_RESIST_ICE,
		item.APPLY_RESIST_EARTH 			: localeInfo.OFFLINESHOP_ATTR_RESIST_EARTH,
		item.APPLY_RESIST_DARK 				: localeInfo.OFFLINESHOP_ATTR_RESIST_DARK,
		item.APPLY_ANTI_CRITICAL_PCT 		: localeInfo.OFFLINESHOP_ATTR_ANTI_CRITICAL_PCT,
		item.APPLY_ANTI_PENETRATE_PCT 		: localeInfo.OFFLINESHOP_ATTR_ANTI_PENETRATE_PCT,
# NEW BONUS
		item.APPLY_ATTBONUS_SEFI			: localeInfo.SWITCHBOOT_TOOLTIP_APPLY_ATTBONUS_SEFI,
		item.APPLY_ATTBONUS_METINE			: localeInfo.SWITCHBOOT_TOOLTIP_APPLY_ATTBONUS_METINE,
# END
	}

	if app.ENABLE_ELEMENT_ADD:
		ATTRIBUTES.update({
			item.APPLY_ENCHANT_ELECT 		: localeInfo.SWITCHBOOT_TOOLTIP_APPLY_ENCHANT_ELECT,
			item.APPLY_ENCHANT_FIRE 		: localeInfo.SWITCHBOOT_TOOLTIP_APPLY_ENCHANT_FIRE,
			item.APPLY_ENCHANT_ICE 			: localeInfo.SWITCHBOOT_TOOLTIP_APPLY_ENCHANT_ICE,
			item.APPLY_ENCHANT_WIND 		: localeInfo.SWITCHBOOT_TOOLTIP_APPLY_ENCHANT_WIND,
			item.APPLY_ENCHANT_EARTH 		: localeInfo.SWITCHBOOT_TOOLTIP_APPLY_ENCHANT_EARTH,
			item.APPLY_ENCHANT_DARK 		: localeInfo.SWITCHBOOT_TOOLTIP_APPLY_ENCHANT_DARK,
			item.APPLY_ATTBONUS_CZ	 		: localeInfo.SWITCHBOOT_TOOLTIP_APPLY_ATTBONUS_CZ,
			item.APPLY_ATTBONUS_INSECT 		: localeInfo.SWITCHBOOT_TOOLTIP_APPLY_ATTBONUS_INSECT,
			item.APPLY_ATTBONUS_DESERT 		: localeInfo.SWITCHBOOT_TOOLTIP_APPLY_ATTBONUS_DESERT,
		})

	if app.ENABLE_PENDANT:
		ATTRIBUTES.update({
			item.APPLY_ATTBONUS_SWORD 		: localeInfo.SWITCHBOOT_TOOLTIP_APPLY_ATTBONUS_SWORD,
			item.APPLY_ATTBONUS_TWOHAND 	: localeInfo.SWITCHBOOT_TOOLTIP_APPLY_ATTBONUS_TWOHAND,
			item.APPLY_ATTBONUS_DAGGER 		: localeInfo.SWITCHBOOT_TOOLTIP_APPLY_ATTBONUS_DAGGER,
			item.APPLY_ATTBONUS_BELL 		: localeInfo.SWITCHBOOT_TOOLTIP_APPLY_ATTBONUS_BELL,
			item.APPLY_ATTBONUS_FAN 		: localeInfo.SWITCHBOOT_TOOLTIP_APPLY_ATTBONUS_FAN,
			item.APPLY_ATTBONUS_BOW 		: localeInfo.SWITCHBOOT_TOOLTIP_APPLY_ATTBONUS_BOW,
			item.APPLY_RESIST_HUMAN 		: localeInfo.SWITCHBOOT_TOOLTIP_APPLY_RESIST_HUMAN,
			item.APPLY_RESIST_MOUNT_FALL 	: localeInfo.SWITCHBOOT_TOOLTIP_APPLY_RESIST_MOUNT_FALL,
		})

	if app.ENABLE_HYDRA_DUNGEON:
		ATTRIBUTES.update({
			item.APPLY_ATTBONUS_HYDRA		: localeInfo.SWITCHBOOT_TOOLTIP_APPLY_ATTBONUS_HYDRA,
		})
## END

	if app.ENABLE_WOLFMAN:
		ATTRIBUTES.update({
			item.APPLY_BLEEDING_PCT 		: localeInfo.OFFLINESHOP_ATTR_APPLY_BLEEDING_PCT,
			item.APPLY_BLEEDING_REDUCE 		: localeInfo.OFFLINESHOP_ATTR_APPLY_BLEEDING_REDUCE,
		})

	if app.ENABLE_MAGIC_REDUCTION_SYSTEM:
		ATTRIBUTES.update({
			item.APPLY_RESIST_MAGIC_REDUCTION : localeInfo.OFFLINESHOP_ATTR_RESIST_MAGIC_REDUCTION,
		})

	def __init__(self):
		ui.ScriptWindow.__init__(self)
		self.clear()
		offlineshop.SetOfflineshopBoard(self)
		self.__loadWindow()

	def clear(self):
		self.MyShopButton = None
		self.ListOfShopButton = None
		self.SearchFilterButton = None
		self.SearchHistoryButton = None
		self.MyPatternsButton = None

		self.MyAuctionButton = None
		self.ListOfAuctionsButton = None

		self.pageBoards = {}
		self.pageCategory = "my_shop"
		self.updateEvents = {}
		self.itemTooltip = None
		self.popupMessage = None
		self.ShopItemForSale = []
		self.ShopItemSold = []
		self.EditPriceSlot = None
		self.AddItemSlotIndex = -1
		self.CommonInputPriceDlg = None
		self.CommonQuestionDlg = None
		self.CommonPickValuteDlg = None
		self.TitleBar = None

		self.SearchFilterShopItemResult = []
		self.FilterHistory = []
		self.FilterPatterns = {}

		self.ShopList = []
		self.ShopOpenInfo = {}
		self.ShopListTable = None

		self.InsertedItems = []
		self.CreateShopNameEdit = None
		self.CreateShopDaysCountText = None
		self.CreateShopHoursCountText = None
		self.CreateShopItemsTable = None
		self.CreateShopItemsInfos = {}

		self.MyShopItemsTable = None
		self.MyShopShopTitle = None
		self.MyShopShopDuration = None
		self.MyShopCloseButton = None
		self.MyShopEditNameDlg = None
		self.MyShopOffers = None
		self.MyShopOffersTable = None

		self.OpenShopItemsTable = None
		self.OpenShopBackToListButton = None
		self.OpenShopShopTitle = None
		self.OpenShopShopDuration = None
		self.OpenShopBuyItemID = -1

		self.SearchFilterItemsNameDict = {}
		self.SearchFilterCheckBoxesRace = {}
		self.SearchFilterCheckBoxes = {}
		self.SearchFilterAttributeButtons = []

		self.SearchFilterComboBoxSuggestion = None
		self.SearchFilterSuggestionObj = None
		self.SearchFilterItemNameInput = None
		self.SearchFilterItemLevelStart = None
		self.SearchFilterItemLevelEnd = None
		self.SearchFilterItemYangMin = None
		self.SearchFilterItemYangMax = None
		self.SearchFilterResetFilterButton = None
		self.SearchFilterSavePatternButton = None
		self.SearchFilterStartSearch = None

		self.SearchFilterResultItemsTable = None

		self.SearchHistoryTable = None

		self.SearchPatternsTable = None
		self.SearchPatternsInputNameDlg = None

		self.ShopSafeboxItems = []
		self.ShopSafeboxItemsTable = None
		self.ShopSafeboxValuteAmount=0
		self.ShopSafeboxValuteText = None
		self.ShopSafeboxWithdrawYangButton = None
		if ENABLE_CHEQUE_SYSTEM:
			self.ShopSafeboxValuteTextCheque = None

		self.MyOffersList = None
		self.MyOffersTable = None

		self.MyAuctionInfo = {}
		self.MyAuctionOffers = []
		self.MyAuctionOfferTable = None

		self.MyAuctionOwnerName = None
		self.MyAuctionDuration = None
		self.MyAuctionBestOffer = None
		self.MyAuctionMinRaise = None
		self.MyAuctionSlot = None

		self.OpenAuctionInfo = {}
		self.OpenAuctionOffers = []
		self.OpenAuctionOfferTable = None

		self.OpenAuctionOwnerName = None
		self.OpenAuctionDuration = None
		self.OpenAuctionBestOffer = None
		self.OpenAuctionMinRaise = None
		self.OpenAuctionSlot = None

		self.AuctionListInfo = {}
		self.AuctionListTable = None

		self.CreateAuctionCreateAuctionButton = None
		self.CreateAuctionDaysInput = None
		self.CreateAuctionStartingPriceInput = None
		self.CreateAuctionSlot = None
		self.CreateAuctionDaysIncreaseButton = None
		self.CreateAuctionDaysDecreaseButton = None

		self.RefreshSymbol = None

	def Destroy(self):
		self.Hide()
		self.clear()
		self.ClearDictionary()

	def OnPressEscapeKey(self):
		self.Close()
		return True

	def __del__(self):
		self.Destroy()
		print("------------ DESTROYED OFFLINESHOP INTERFACE ------------")
		ui.ScriptWindow.__del__(self)

	def __loadWindow(self):
		pyLoader = ui.PythonScriptLoader()
		pyLoader.LoadScriptFile(self , "uiscript/offlineshopwindow.py")
		
		childDict = {
			"create_shop"		: "MyShopBoardNoShop",
			"my_shop"			: "MyShopBoard",
			"open_shop"			: "ListOfShop_OpenShop",
			"shop_list"			: "ListOfShop_List",
			"shop_safebox"		: "ShopSafeboxPage",
			"my_offers"			: "MyOffersPage",

			"search_history"	: "SearchHistoryBoard",
			"my_patterns"		: "MyPatternsBoard",
			"search_filter"		: "SearchFilterBoard",

			"my_auction"		: "MyAuction",
			"auction_list"		: "AuctionList",
			"open_auction"		: "OpenAuction",
			"create_auction"	: "CreateAuction",

		}

		self.pageBoards = {}
		
		for k, v in childDict.items():
			self.pageBoards[k] = self.GetChild(v)
			self.pageBoards[k].Hide()

		self.RefreshSymbol = self.GetChild("RefreshSymbol")

		self.CreateShopNameEdit				= self.GetChild("ShopNameInput")
		self.CreateShopDaysCountText		= self.GetChild("DaysCountText")
		self.CreateShopHoursCountText		= self.GetChild("HoursCountText")
		
		self.CreateShopIncreaseDaysButton	= self.GetChild("IncreaseDaysButton")
		self.CreateShopDecreaseDaysButton	= self.GetChild("DecreaseDaysButton")
		
		self.CreateShopIncreaseHoursButton	= self.GetChild("IncreaseHoursButton")
		self.CreateShopDecreaseHoursButton	= self.GetChild("DecreaseHoursButton")
		
		self.CreateShopButton				= self.GetChild("CreateShopButton")
		
		self.CreateShopIncreaseDaysButton.SAFE_SetEvent(self.__OnClickCreateShopIncreaseDaysButton)
		self.CreateShopDecreaseDaysButton.SAFE_SetEvent(self.__OnClickCreateShopDecreaseDaysButton)
		
		self.CreateShopIncreaseHoursButton.SAFE_SetEvent(self.__OnClickCreateShopIncreaseHoursButton)
		self.CreateShopDecreaseHoursButton.SAFE_SetEvent(self.__OnClickCreateShopDecreaseHoursButton)

		self.CreateShopButton.SAFE_SetEvent(self.__OnClickCreateShopButton)
		self.__MakeCreateShopItemsTable()

		self.MyShopShopDuration		= self.GetChild("MyShopShopDuration")
		self.MyShopShopTitle		= self.GetChild("MyShopShopTitle")
		self.MyShopCloseButton		= self.GetChild("MyShopCloseButton")
		self.MyShopEditTitleButton	= self.GetChild("MyShopEditTitleButton")
		self.__MakeMyShopItemsTable()
		self.__MakeMyShopOffersTable()
		
		self.MyShopCloseButton.SAFE_SetEvent(self.__OnClickCloseButton)
		self.MyShopEditTitleButton.SAFE_SetEvent(self.__OnClickMyShopEditNameButton)
		
		self.MyShopEditNameDlg	= uiCommon.InputDialogWithDescription()
		self.MyShopEditNameDlg.SetMaxLength(35)
		self.MyShopEditNameDlg.SetDescription(localeInfo.OFFLINESHOP_EDIT_SHOPNAME_DESCRIPTION)
		self.MyShopEditNameDlg.SetAcceptEvent(self.__OnAcceptChangeShopNameDlg)
		self.MyShopEditNameDlg.SetCancelEvent(self.__OnCancelChangeShopNameDlg)
		self.MyShopEditNameDlg.SetTitle(localeInfo.OFFLINESHOP_EDIT_SHOPNAME_TITLE)
		self.MyShopEditNameDlg.Hide()

		self.__MakeShopListTable()

		self.OpenShopBackToListButton	= self.GetChild("OpenShopBackToListButton")
		self.OpenShopShopTitle			= self.GetChild("OpenShopShopTitle")
		self.OpenShopShopDuration		= self.GetChild("OpenShopShopDuration")
		
		self.CommonQuestionDlg 	= uiCommon.QuestionDialog()

		self.CommonPickValuteDlg = uiPickMoney.PickMoneyDialog()
		self.CommonPickValuteDlg.LoadDialog()

		self.OpenShopBackToListButton.SAFE_SetEvent(self.__OnClickShopListPage)
		self.__MakeOpenShopItemsTable()


		self.TitleBar = self.GetChild("TitleBar")
		self.TitleBar.SetCloseEvent(self.Close)

		self.MyShopButton			= self.GetChild("MyShopButton")
		self.ListOfShopButton		= self.GetChild("ListOfShopButton")
		self.ShopSafeboxButton		= self.GetChild("ShopSafeboxButton")
		self.MyOffersPageButton		= self.GetChild("MyOffersPageButton")

		self.SearchFilterButton		= self.GetChild("SearchFilterButton")
		self.SearchHistoryButton	= self.GetChild("SearchHistoryButton")
		self.MyPatternsButton		= self.GetChild("MyPatternsButton")
		
		self.MyAuctionButton		= self.GetChild("MyAuctionButton")
		self.ListOfAuctionsButton	= self.GetChild("ListOfAuctionsButton")

		self.MyShopButton.SAFE_SetEvent(self.__OnClickMyShopPage)
		self.ListOfShopButton.SAFE_SetEvent(self.__OnClickShopListPage)
		self.ShopSafeboxButton.SAFE_SetEvent(self.__OnClickShopSafeboxPage)
		self.MyOffersPageButton.SAFE_SetEvent(self.__OnClickMyOffersPage)

		self.SearchFilterButton.SAFE_SetEvent(self.__OnClickSearchFilterPage)
		self.SearchHistoryButton.SAFE_SetEvent(self.__OnClickSearchHistoryPage)
		self.MyPatternsButton.SAFE_SetEvent(self.__OnClickMyPatternsPage)
		
		self.MyAuctionButton.SAFE_SetEvent(self.__OnClickMyAuctionPage)
		self.ListOfAuctionsButton.SAFE_SetEvent(self.__OnClickAuctionListPage)

		self.updateEvents = {
			"my_shop"		: self.__OnUpdateMyShopPage,
			"create_shop"	: self.__OnUpdateCreateShopPage,
			"open_shop"		: self.__OnUpdateOpenShopPage,
			"search_filter"	: self.__OnUpdateSearchFilterPage,
			"shop_safebox"	: self.__OnUpdateShopSafeboxPage,
			"my_offers"		: self.__OnUpdateMyOffersPage,
			"search_history": self.__OnUpdateSearchHistoryPage,
			"my_patterns"	: self.__OnUpdateMyPatternPage,
			"create_auction": self.__OnUpdateCreateAuctionPage,
			"my_auction"	: self.__OnUpdateMyAuctionPage,
			"open_auction"	: self.__OnUpdateOpenAuctionPage,
			"auction_list"  : self.__OnUpdateAuctionListPage,
		}

		tooltip = uiToolTip.ItemToolTip(width=300)
		tooltip.ClearToolTip()
		tooltip.SetFollow(True)
		tooltip.Hide()
		self.itemTooltip = tooltip

		popup = uiCommon.PopupDialog()
		popup.SetWidth(250)
		popup.Hide()
		self.popupMessage = popup

		self.CommonInputPriceDlg =  uiCommon.MoneyInputDialogCheque() if ENABLE_CHEQUE_SYSTEM  else uiCommon.MoneyInputDialog()
		self.CommonInputPriceDlg.SetAcceptEvent(self.__OnAcceptInputPrice)
		self.CommonInputPriceDlg.SetCancelEvent(self.__OnCancelInputPrice)

		offlineshop.RefreshItemNameMap()
		self.__MakeSearchFilterResultItemsTable()

		self.SearchFilterItemNameInput 		= self.GetChild("SearchFilterItemNameInput")
		self.SearchFilterItemLevelStart		= self.GetChild("SearchFilterItemLevelStart")
		self.SearchFilterItemLevelEnd		= self.GetChild("SearchFilterItemLevelEnd")
		self.SearchFilterItemYangMin		= self.GetChild("SearchFilterItemYangMin")
		self.SearchFilterItemYangMax		= self.GetChild("SearchFilterItemYangMax")
		self.SearchFilterResetFilterButton	= self.GetChild("SearchFilterResetFilterButton")
		self.SearchFilterSavePatternButton	= self.GetChild("SearchFilterSavePatternButton")
		self.SearchFilterStartSearch		= self.GetChild("SearchFilterStartSearch")
		
		self.__MakeSearchFilterCheckBoxes()
		self.SearchFilterComboBoxSuggestion	= ui.ComboBox()
		self.SearchFilterComboBoxSuggestion.SetParent(self.pageBoards["search_filter"])
		self.SearchFilterComboBoxSuggestion.SetPosition(75,50)
		self.SearchFilterComboBoxSuggestion.SetSize(130,17)
		self.SearchFilterComboBoxSuggestion.Show()

		self.SearchFilterSuggestionObj = Suggestions()
		self.SearchFilterSuggestionObj.SetComboBox(self.SearchFilterComboBoxSuggestion)
		self.SearchFilterSuggestionObj.SetInputBox(self.SearchFilterItemNameInput)
		self.SearchFilterSuggestionObj.SetMainDict(self.SearchFilterItemsNameDict)
		
		self.SearchFilterResetFilterButton.SAFE_SetEvent(self.__OnClickSearchFilterResetFilterButton)
		self.SearchFilterSavePatternButton.SAFE_SetEvent(self.__OnClickSearchFilterSavePatternButton)
		self.SearchFilterStartSearch.SAFE_SetEvent(self.__OnClickSearchFilterStartSearch)

		self.SearchFilterSubTypeComboBox = ui.ComboBox()
		self.SearchFilterSubTypeComboBox.SetParent(self.pageBoards["search_filter"])
		self.SearchFilterSubTypeComboBox.SetPosition(220,50)
		self.SearchFilterSubTypeComboBox.SetSize(100,17)
		self.SearchFilterSubTypeComboBox.Show()
		self.SearchFilterSubTypeComboBox.InsertItem(SUBTYPE_NOSET , localeInfo.OFFLINESHOP_TYPE_COMBOBOX_NOSET)
		self.SearchFilterSubTypeComboBox.SetCurrentItem(localeInfo.OFFLINESHOP_TYPE_COMBOBOX_NOSET)

		self.SearchFilterTypeComboBox = ui.ComboBox()
		self.SearchFilterTypeComboBox.SetParent(self.pageBoards["search_filter"])
		self.SearchFilterTypeComboBox.SetPosition(220,29)
		self.SearchFilterTypeComboBox.SetSize(100,17)
		self.SearchFilterTypeComboBox.Show()

		for k,v in self.ITEM_TYPES.items():
			self.SearchFilterTypeComboBox.InsertItem(k, v["name"])
		
		self.SearchFilterTypeComboBox.SetCurrentItem(localeInfo.OFFLINESHOP_TYPE_COMBOBOX_NOSET)

		self.SearchFilterTypeComboBox.SetEvent(self.__OnSelectSearchFilterTypeComboBox)
		self.SearchFilterSubTypeComboBox.SetEvent(self.__OnSelectSearchFilterSubTypeComboBox)
		
		self.SearchFilterTypeComboBoxIndex = 0
		self.SearchFilterSubTypeComboBoxIndex = SUBTYPE_NOSET

		for i in xrange(player.ATTRIBUTE_SLOT_NORM_NUM):
			self.SearchFilterAttributeButtons.append(self.GetChild("SearchFilterAttributeButton%d"%(i+1)))
			self.SearchFilterAttributeButtons[i].SAFE_SetEvent(self.__OnClickSearchFilterAttributeButton, i)
			self.SearchFilterAttributeButtons[i].SetText(self.ATTRIBUTES[0])
		
		selector = SuggestionSelector()
		selector.SetParent(self.pageBoards["search_filter"])
		selector.SetSelectEvent(self.__OnSelectSearchFilterSuggestionSelector)
		selector.SetAttributeDict(self.ATTRIBUTES)
		selector.Hide()
		
		
		self.SearchFilterSuggestionSelector = selector
		self.SearchFilterAttributeSetting = [0 for x in xrange(player.ATTRIBUTE_SLOT_NORM_NUM)]

		self.__MakeSearchHistoryTable()

		self.__MakeSearchPatternsTable()
		self.SearchPatternsInputNameDlg = uiCommon.InputDialogWithDescription()
		self.SearchPatternsInputNameDlg.SetMaxLength(25)
		self.SearchPatternsInputNameDlg.SetDescription(localeInfo.OFFLINESHOP_MY_PATTERN_INSERT_NAME_DESC)
		self.SearchPatternsInputNameDlg.SetAcceptEvent(self.__OnAcceptMyPatternInputName)
		self.SearchPatternsInputNameDlg.SetCancelEvent(self.__OnCancelMyPatternInputName)
		self.SearchPatternsInputNameDlg.SetTitle(localeInfo.OFFLINESHOP_MY_PATTERN_INSERT_NAME_TITLE)
		self.SearchPatternsInputNameDlg.Hide()

		self.__MakeShopSafeboxItemsTable()
		self.ShopSafeboxValuteText 			= self.GetChild("ShopSafeboxValuteText")
		self.ShopSafeboxWithdrawYangButton	= self.GetChild("ShopSafeboxWithdrawYangButton")

		self.ShopSafeboxWithdrawYangButton.SAFE_SetEvent(self.__OnClickShopSafeboxWithdrawYang)
		if ENABLE_CHEQUE_SYSTEM:
			self.ShopSafeboxValuteTextCheque= self.GetChild("ShopSafeboxValuteTextCheque")

		self.__MakeMyOffersTable()

		self.MyAuctionOwnerName 	= self.GetChild("MyAuction_OwnerName")
		self.MyAuctionDuration 		= self.GetChild("MyAuction_Duration")
		self.MyAuctionBestOffer 	= self.GetChild("MyAuction_BestOffer")
		self.MyAuctionMinRaise 		= self.GetChild("MyAuction_MinRaise")
		
		self.__MakeMyAuctionOffersTable()

		self.OpenAuctionOwnerName	= self.GetChild("OpenAuction_OwnerName")
		self.OpenAuctionDuration 	= self.GetChild("OpenAuction_Duration")
		self.OpenAuctionBestOffer 	= self.GetChild("OpenAuction_BestOffer")
		self.OpenAuctionMinRaise 	= self.GetChild("OpenAuction_MinRaise")

		if ENABLE_CHEQUE_SYSTEM:
			self.ShopSafeboxValuteTextCheque= self.GetChild("ShopSafeboxValuteTextCheque")
		self.__MakeOpenAuctionOffersTable()

		self.__MakeAuctionListTable()

		self.CreateAuctionCreateAuctionButton 	= self.GetChild("CreateAuctionCreateAuctionButton")
		self.CreateAuctionDaysInput 			= self.GetChild("CreateAuctionDaysInput")
		self.CreateAuctionStartingPriceInput 	= self.GetChild("CreateAuctionStartingPriceInput")

		self.CreateAuctionWindowPos =-1
		self.CreateAuctionSlotPos=-1

		self.CreateAuctionDaysDecreaseButton	= self.GetChild("CreateAuctionIncreaseDaysButton")
		self.CreateAuctionDaysIncreaseButton	= self.GetChild("CreateAuctionDecreaseDaysButton")

		self.CreateAuctionCreateAuctionButton.SAFE_SetEvent(self.__OnClickCreateAuctionButton)
		self.CreateAuctionDaysDecreaseButton.SAFE_SetEvent(self.__OnClickCreateAuctionDaysDecreaseButton)
		self.CreateAuctionDaysIncreaseButton.SAFE_SetEvent(self.__OnClickCreateAuctionDaysIncreaseButton)
		self.__MakeCreateAuctionSlot()
		
		if app.ENABLE_WINDOW_SLIDE_EFFECT:
			self.EnableSlidingEffect()

		items = (
			self.CreateShopNameEdit,
			self.SearchFilterItemNameInput,
			self.SearchFilterItemLevelStart,
			self.SearchFilterItemLevelEnd,
			self.SearchFilterItemYangMin,
			self.SearchFilterItemYangMax,
			self.CreateAuctionStartingPriceInput,
		)

		autokill = lambda arg: arg.KillFocus()

		for item in items:
			item.OnPressEscapeKey = autokill

	def Open(self):
		events = {
			"my_shop"			: self.__OnClickMyShopPage,
			"open_shop"			: self.__OnClickShopListPage,
			"shop_list"			: self.__OnClickShopListPage,
			"shop_safebox"		: self.__OnClickShopSafeboxPage,
			"my_offers"			: self.__OnClickMyOffersPage,

			"search_history"	: self.__OnClickSearchHistoryPage,
			"my_patterns"		: self.__OnClickMyPatternsPage,
			"search_filter"		: self.__OnClickSearchFilterPage,

			"my_auction"		: self.__OnClickMyAuctionPage,
			"auction_list"		: self.__OnClickAuctionListPage,
			"open_auction"		: self.__OnClickAuctionListPage,
		}

		if self.pageCategory in events.keys():
			events[self.pageCategory]()
		self.Show()

	def Close(self):
		self.Hide()
		self.itemTooltip.Hide()
		self.MyShopEditNameDlg.Hide()
		self.CommonQuestionDlg.Hide()
		self.CommonPickValuteDlg.Hide()
		self.CommonInputPriceDlg.Hide()

		offlineshop.SendCloseBoard()

	def __ResetCreateShopPage(self):
		self.CreateShopItemsTable.ClearElement()
		self.CreateShopItemsInfos = {}

	def __MakeCreateShopItemsTable(self):
		board = self.pageBoards["create_shop"]
		
		table = TableWindowWithScrollbar(575, 325, 11,3, TableWindowWithScrollbar.SCROLLBAR_HORIZONTAL)
		table.SetParent(board)
		table.SetPosition(25, 115)
		table.SetDefaultCreateChild(MakeDefaultEmptySlot, self.__OnClickCreateShopEmptySlot)
		
		table.Show()
		self.CreateShopItemsTable = table
	
	def __MakeMyShopItemsTable(self):
		board = self.pageBoards["my_shop"]
		
		table = TableWindowWithScrollbar(588, 324, 11,3, TableWindowWithScrollbar.SCROLLBAR_HORIZONTAL)
		table.SetParent(board)
		table.SetPosition(17, 49)
		table.SetDefaultCreateChild(MakeDefaultEmptySlot, self.__OnClickMyShopEmptySlot)

		table.Show()
		self.MyShopItemsTable = table
	
	def __MakeMyShopOffersTable(self):
		board = self.pageBoards["my_shop"]

		table = TableWindowWithScrollbar(585, 114, 1,5 , TableWindowWithScrollbar.SCROLLBAR_VERTICAL)
		table.SetParent(board)
		table.SetPosition(20, 395)

		table.Show()
		self.MyShopOffersTable = table

	def __MakeShopListTable(self):
		board = self.pageBoards["shop_list"]
		
		table = TableWindowWithScrollbar(570, 476, 1, 23, TableWindowWithScrollbar.SCROLLBAR_VERTICAL)
		table.SetParent(board)
		table.SetPosition(30, 26)
		
		table.Show()
		self.ShopListTable = table

	def __MakeOpenShopItemsTable(self):
		board = self.pageBoards["open_shop"]
		
		table = TableWindowWithScrollbar(584, 440, 11,4, TableWindowWithScrollbar.SCROLLBAR_HORIZONTAL)
		table.SetParent(board)
		table.SetPosition(20,55)
		table.SetDefaultCreateChild(MakeDefaultEmptySlot)
		
		table.Show()
		self.OpenShopItemsTable = table

	def __MakeOpenAuctionOffersTable(self):
		board = self.pageBoards["open_auction"]

		table = TableWindowWithScrollbar(580, 340, 1, 20, TableWindowWithScrollbar.SCROLLBAR_VERTICAL)
		table.SetParent(board)
		table.SetPosition(16, 159)

		table.Show()
		self.OpenAuctionOfferTable = table

	def __MakeMyAuctionOffersTable(self):
		board = self.pageBoards["my_auction"]

		table = TableWindowWithScrollbar(580, 340, 1,20, TableWindowWithScrollbar.SCROLLBAR_VERTICAL)
		table.SetParent(board)
		table.SetPosition(16, 159)

		table.Show()
		self.MyAuctionOfferTable = table

	def __MakeAuctionListTable(self):
		board = self.pageBoards["auction_list"]

		table = TableWindowWithScrollbar(581, 471, 1, 20, TableWindowWithScrollbar.SCROLLBAR_VERTICAL)
		table.SetParent(board)
		table.SetPosition(21, 28)

		table.Show()
		self.AuctionListTable = table

	def __MakeSearchFilterCheckBoxes(self):
		positions = {
			"name" 	: { "x" : 20, 	"y" : 9, 	},
			"type" 	: { "x" : 222, 	"y" : 9, 	},
			"price" : { "x" : 224, 	"y" : 83, 	},
			"level" : { "x" : 338, 	"y" : 9, 	},
			"wear" 	: { "x" : 20, 	"y" : 83, 	},
			"attr" 	: { "x" : 404, 	"y" : 9, 	},
		}

		if app.ENABLE_WOLFMAN:
			race = {
				"warrior" : { "x" 	: 66+18, 	"y" : 88, 	},
				"assassin" 	: { "x" : 66+18, 	"y" : 114, 	},
				"sura" 		: { "x" : 139+10, 	"y" : 88, 	},
				"shaman" 	: { "x" : 139+10, 	"y" : 114, 	},
				"wolfman"	: { "x" : 20, 		"y" : 114,  },
			}

		else:
			race = {
				"warrior" 	: { "x" : 66, 	"y" : 88, 	},
				"assassin" 	: { "x" : 66, 	"y" : 114, 	},
				"sura" 		: { "x" : 139, 	"y" : 88, 	},
				"shaman" 	: { "x" : 139, 	"y" : 114, 	},
			}
		
		for k, v in positions.items():
			checkbox = ui.CheckBox({'base' : "offlineshop/checkbox/base.png", 'tip' : "offlineshop/checkbox/tip.png",})
			checkbox.SetParent(self.pageBoards["search_filter"])
			checkbox.SetPosition(v["x"] , v["y"])
			checkbox.Show()
			
			self.SearchFilterCheckBoxes[k] = checkbox
		
		
		for k, v in race.items():
			checkbox = ui.CheckBox({'base' : "offlineshop/checkbox/%s_base.png"%k , 'tip' : "offlineshop/checkbox/%s_tip.png"%k,})
			checkbox.SetParent(self.pageBoards["search_filter"])
			checkbox.SetPosition(v["x"] , v["y"])
			checkbox.Show()
			checkbox.Enable()
			
			self.SearchFilterCheckBoxesRace[k] = checkbox

	def __MakeSearchFilterResultItemsTable(self):
		board = self.pageBoards["search_filter"]
		
		table = TableWindowWithScrollbar(585, 329, 11, 3, TableWindowWithScrollbar.SCROLLBAR_HORIZONTAL)
		table.SetParent(board)
		table.SetPosition(14,153)
		table.SetDefaultCreateChild(MakeDefaultEmptySlot)

		table.ClearElement()

		table.Show()
		self.SearchFilterResultItemsTable = table

	def __MakeSearchHistoryTable(self):
		board = self.pageBoards["search_history"]

		table = TableWindowWithScrollbar(585, 470, 1, 20 , TableWindowWithScrollbar.SCROLLBAR_VERTICAL)
		table.SetParent(board)
		table.SetPosition(13, 32)

		table.Show()
		self.SearchHistoryTable = table

	def __MakeSearchPatternsTable(self):
		board = self.pageBoards["my_patterns"]

		table = TableWindowWithScrollbar(585, 470, 1, 20, TableWindowWithScrollbar.SCROLLBAR_VERTICAL)
		table.SetParent(board)
		table.SetPosition(13, 36)

		table.Show()
		self.SearchPatternsTable = table

	def __MakeShopSafeboxItemsTable(self):
		board = self.pageBoards["shop_safebox"]

		table = TableWindowWithScrollbar(582, 443, 11, 4, TableWindowWithScrollbar.SCROLLBAR_HORIZONTAL)
		table.SetParent(board)
		table.SetPosition(18, 50)
		table.SetDefaultCreateChild(MakeDefaultEmptySlot)

		table.Show()
		self.ShopSafeboxItemsTable = table

	def __MakeMyOffersTable(self):
		board = self.pageBoards["my_offers"]

		table = TableWindowWithScrollbar(584, 445, 2, 4, TableWindowWithScrollbar.SCROLLBAR_VERTICAL)
		table.SetParent(board)
		table.SetPosition(18, 50)

		table.Show()
		self.MyOffersTable = table

	def __MakeCreateAuctionSlot(self):
		if self.CreateAuctionSlot:
			del self.CreateAuctionSlot

		slot = Slot()
		slot.Show()
		slot.SetParent(self.pageBoards['create_auction'])
		slot.SetPosition(430,152)
		self.CreateAuctionSlot = slot

	def __MakeMyAuctionSlot(self, info):
		if self.MyAuctionSlot:
			del self.MyAuctionSlot

		slot = Slot()
		slot.SetParent(self.pageBoards['my_auction'])
		slot.SetPosition(339+54,97-73)
		slot.SetInfo(info)
		slot.Show()

		self.MyAuctionSlot = slot

	def __MakeOpenAuctionSlot(self, info):
		if self.OpenAuctionSlot:
			del self.OpenAuctionSlot

		slot = Slot()
		slot.SetParent(self.pageBoards['open_auction'])
		slot.SetPosition(339+54,97-73)
		slot.SetInfo(info)
		slot.Show()

		slot.SetOnMouseLeftButtonUpEvent(self.__OnClickOpenAuctionMakeOffer)

		self.OpenAuctionSlot = slot

	def __OnClickCreateShopIncreaseDaysButton(self):
		days = int(self.CreateShopDaysCountText.GetText())
		
		if days == offlineshop.OFFLINESHOP_MAX_DAYS:
			self.CreateShopDaysCountText.SetText("0")
		
		else:
			self.CreateShopDaysCountText.SetText(str(days+1))

	def __OnClickCreateShopDecreaseDaysButton(self):
		days = int(self.CreateShopDaysCountText.GetText())
		
		if days == 0:
			self.CreateShopDaysCountText.SetText(str(offlineshop.OFFLINESHOP_MAX_DAYS))
		
		else:
			self.CreateShopDaysCountText.SetText(str(days-1))

	def __OnClickCreateShopIncreaseHoursButton(self):
		hours = int(self.CreateShopHoursCountText.GetText())
		
		if hours == offlineshop.OFFLINESHOP_MAX_HOURS:
			self.CreateShopHoursCountText.SetText("0")
		
		else:
			self.CreateShopHoursCountText.SetText(str(hours+1))
	
	def __OnClickCreateShopDecreaseHoursButton(self):
		hours = int(self.CreateShopHoursCountText.GetText())
		
		if hours == 0:
			self.CreateShopHoursCountText.SetText(str(offlineshop.OFFLINESHOP_MAX_HOURS))
		
		else:
			self.CreateShopHoursCountText.SetText(str(hours-1))

	def __OnClickCloseButton(self):
		offlineshop.SendForceCloseShop()
	
	def __OnClickMyShopEditNameButton(self):
		self.MyShopEditNameDlg.inputValue.SetText("")
		self.MyShopEditNameDlg.Open()
	
	def __OnClickMyShopPage(self):
		offlineshop.SendCloseBoard()
		offlineshop.SendOpenShopOwner()
		self.EnableRefreshSymbol()

	def __OnClickShopListPage(self):
		offlineshop.SendCloseBoard()
		offlineshop.SendRequestShopList()
		self.EnableRefreshSymbol()

	def __OnClickShopSafeboxPage(self):
		offlineshop.SendCloseBoard()
		offlineshop.SendSafeboxOpen()
		self.EnableRefreshSymbol()

	def __OnClickMyOffersPage(self):
		offlineshop.SendCloseBoard()
		offlineshop.SendOfferListRequest()
		self.EnableRefreshSymbol()

	def __OnClickSearchFilterPage(self):
		offlineshop.SendCloseBoard()
		self.pageCategory = "search_filter"
		for page in self.pageBoards.values():
			page.Hide()
		
		self.pageBoards["search_filter"].Show()

	def __OnSelectSearchFilterTypeComboBox(self, index):
		self.SearchFilterSubTypeComboBox.ClearItem()
		self.SearchFilterSubTypeComboBox.InsertItem(SUBTYPE_NOSET , localeInfo.OFFLINESHOP_TYPE_COMBOBOX_NOSET)
		
		if index == 0:
			self.SearchFilterTypeComboBoxIndex = 0
			self.SearchFilterTypeComboBox.SetCurrentItem(localeInfo.OFFLINESHOP_TYPE_COMBOBOX_NOSET)
			self.SearchFilterSubTypeComboBox.SetCurrentItem(localeInfo.OFFLINESHOP_TYPE_COMBOBOX_NOSET)
			self.SearchFilterSubTypeComboBoxIndex = SUBTYPE_NOSET
			return
		
		self.SearchFilterTypeComboBoxIndex = index
		self.SearchFilterTypeComboBox.SetCurrentItem(self.ITEM_TYPES[index]['name'])
		
		if self.ITEM_TYPES.has_key(index) and self.ITEM_TYPES[index].has_key('subtypes'):
			for sub, name in self.ITEM_TYPES[index]['subtypes'].items():
				self.SearchFilterSubTypeComboBox.InsertItem(sub, name)
		
		else:
			self.SearchFilterSubTypeComboBox.SetCurrentItem(localeInfo.OFFLINESHOP_TYPE_COMBOBOX_NOSET)
			self.SearchFilterSubTypeComboBoxIndex = SUBTYPE_NOSET
	
	def __OnSelectSearchFilterSubTypeComboBox(self, index):
		if index == SUBTYPE_NOSET:
			self.SearchFilterSubTypeComboBox.SetCurrentItem(localeInfo.OFFLINESHOP_TYPE_COMBOBOX_NOSET)
			self.SearchFilterSubTypeComboBoxIndex = SUBTYPE_NOSET
			return
		
		self.SearchFilterSubTypeComboBoxIndex = index
		self.SearchFilterSubTypeComboBox.SetCurrentItem(self.ITEM_TYPES[self.SearchFilterTypeComboBoxIndex]['subtypes'][index])

	def __OnSelectSearchFilterSuggestionSelector(self, index):
		self.SearchFilterAttributeButtons[self.SearchFilterAttributeButtonIndex].SetText(self.ATTRIBUTES[index])
		self.SearchFilterAttributeSetting[self.SearchFilterAttributeButtonIndex] = index
		
		self.SearchFilterAttributeButtonIndex = -1
		self.SearchFilterSuggestionSelector.Hide()
		
	def __OnClickSearchFilterAttributeButton(self, index):
		if self.SearchFilterSuggestionSelector.IsShow():
			self.SearchFilterSuggestionSelector.Hide()
			self.SearchFilterAttributeButtonIndex = -1
		
		else:
			self.SearchFilterAttributeButtonIndex = index
			
			x,y = self.SearchFilterAttributeButtons[index].GetLocalPosition()
			y += self.SearchFilterAttributeButtons[index].GetHeight()
			
			self.SearchFilterSuggestionSelector.SetPosition(x,y)
			self.SearchFilterSuggestionSelector.Show()

	def __OnClickSearchHistoryPage(self):
		offlineshop.SendCloseBoard()
		self.pageCategory = "search_history"

		for v in self.pageBoards.values():
			v.Hide()

		self.pageBoards["search_history"].Show()
		self.RefreshSearchHistoryPage()

	def __OnClickMyPatternsPage(self):
		offlineshop.SendCloseBoard()
		for v in self.pageBoards.values():
			v.Hide()

		self.pageBoards["my_patterns"].Show()
		self.pageCategory = "my_patterns"
		self.RefreshMyPatternsPage()

	def __OnClickSearchPatternElement(self, element):
		info = element.GetInfo()
		self.__SetSearchFilterPattern(info)

		offlineshop.UpdateLastUseFilterPattern(info["id"])
		
	def __OnClickMyAuctionPage(self):
		offlineshop.SendCloseBoard()
		offlineshop.SendAuctionOpenMy()
		self.EnableRefreshSymbol()

	def __OnClickAuctionListPage(self):
		offlineshop.SendCloseBoard()
		offlineshop.SendAuctionListRequest()
		self.EnableRefreshSymbol()
	
	def __OnClickCreateAuctionButton(self):
		if self.CreateAuctionWindowPos == -1:
			return

		if self.CreateAuctionSlotPos == -1:
			return

		daystext = self.CreateAuctionDaysInput.GetText()
		days = int(daystext) if daystext and daystext.isdigit() else 0

		if 0 == days or offlineshop.OFFLINESHOP_MAX_DAYS<days:
			self.__PopupMessage(localeInfo.OFFLINESHOP_CREATE_SHOP_INVALID_DURATION)
			return

		pricetext = self.CreateAuctionStartingPriceInput.GetText()
		price = long(pricetext) if pricetext and pricetext.isdigit() else 0

		if price <= 0:
			self.__PopupMessage(localeInfo.OFFLINESHOP_CREATE_AUCTION_INVALID_PRICE)
			return

		offlineshop.SendAuctionCreate(self.CreateAuctionWindowPos, self.CreateAuctionSlotPos, price, days*24*60)
		self.EnableRefreshSymbol()

	def __OnClickCreateAuctionDaysDecreaseButton(self):
		days = int(self.CreateAuctionDaysInput.GetText())
		if days ==0:
			self.CreateAuctionDaysInput.SetText(str(offlineshop.OFFLINESHOP_MAX_DAYS))
		else:
			self.CreateAuctionDaysInput.SetText(str(days-1))

	def __OnClickCreateAuctionDaysIncreaseButton(self):
		days = int(self.CreateAuctionDaysInput.GetText())
		if days == offlineshop.OFFLINESHOP_MAX_DAYS:
			self.CreateAuctionDaysInput.SetText("0")
		else:
			self.CreateAuctionDaysInput.SetText(str(days + 1))

	def __OnClickSearchFilterResetFilterButton(self):
		for k, v in self.SearchFilterCheckBoxes.items():
			v.Disable()

		for v in self.SearchFilterCheckBoxesRace.values():
			v.Enable()

		self.SearchFilterTypeComboBox.SetCurrentItem(localeInfo.OFFLINESHOP_TYPE_COMBOBOX_NOSET)
		self.SearchFilterTypeComboBoxIndex = 0
		self.SearchFilterSubTypeComboBox.SetCurrentItem(localeInfo.OFFLINESHOP_TYPE_COMBOBOX_NOSET)
		self.SearchFilterSubTypeComboBoxIndex = SUBTYPE_NOSET

		self.SearchFilterItemNameInput.SetText("")
		self.SearchFilterSuggestionObj.Clear()

		self.SearchFilterItemLevelStart.SetText("")
		self.SearchFilterItemLevelEnd.SetText("")

		self.SearchFilterItemYangMin.SetText("")
		self.SearchFilterItemYangMax.SetText("")

		for x in xrange(player.ATTRIBUTE_SLOT_NORM_NUM):
			self.SearchFilterAttributeButtons[x].SetText(localeInfo.OFFLINESHOP_ATTR_UNSET)
			self.SearchFilterAttributeSetting[x] = 0

		self.SearchFilterResultItemsTable.ClearElement()
		self.__OnSelectSearchFilterTypeComboBox(0)
		self.__OnSelectSearchFilterSubTypeComboBox(SUBTYPE_NOSET)
	
	def __OnClickSearchFilterSavePatternButton(self):
		bActiveOne = False

		for v in self.SearchFilterCheckBoxes.values():
			if v.IsEnabled():
				bActiveOne = True
				break

		if not bActiveOne:
			self.__PopupMessage(localeInfo.OFFLINESHOP_NO_FILTER_ACTIVE_MESSAGE)
			return

		if not self.SearchPatternsInputNameDlg.IsShow():
			self.SearchPatternsInputNameDlg.inputValue.SetText("")
			self.SearchPatternsInputNameDlg.Open()

	def __OnClickSearchFilterStartSearch(self):
		bActiveOne = False
		
		for v in self.SearchFilterCheckBoxes.values():
			if v.IsEnabled():
				bActiveOne = True
				break
		
		if not bActiveOne:
			self.__PopupMessage(localeInfo.OFFLINESHOP_NO_FILTER_ACTIVE_MESSAGE)
			return

		if self.SearchFilterCheckBoxes["name"].IsEnabled():
			if not self.SearchFilterItemNameInput.GetText():
				self.__PopupMessage(localeInfo.OFFLINESHOP_SEARCH_FILTER_NAME_NOSET)
				return

		if self.SearchFilterCheckBoxes["price"].IsEnabled():
			if self.SearchFilterItemYangMin.GetText()=="0" and self.SearchFilterItemYangMax.GetText()=="0":
				self.__PopupMessage(localeInfo.OFFLINESHOP_SEARCH_PRICE_ERROR)
				return

		if self.SearchFilterCheckBoxes["level"].IsEnabled():
			if self.SearchFilterItemLevelStart.GetText() == "0" and self.SearchFilterItemLevelEnd.GetText() == "0":
				self.__PopupMessage(localeInfo.OFFLINESHOP_SEARCH_LEVEL_ERROR)
				return

		self.SearchFilterLastUsedSetting = self.GetSearchFilterSettings()
		offlineshop.SendFilterRequest(*self.SearchFilterLastUsedSetting)
		self.EnableRefreshSymbol()

	def __OnClickFilterHistoryElement(self , element):
		info = element.GetInfo()
		self.__SetSearchFilterPattern(info)

	def __OnClickDeleteMyOfferButton(self, id):
		offer = None

		for element in self.MyOffersList:
			if element["offer_id"] == id:
				offer = element
				break

		if not offer:
			return

		accepttext = localeInfo.OFFLINESHOP_CANCEL_OFFER_QUESTION_ACCEPT
		canceltext = localeInfo.OFFLINESHOP_CANCEL_OFFER_QUESTION_CANCEL

		question   = localeInfo.OFFLINESHOP_CANCEL_OFFER_QUESTION

		self.MyOffersCancelOfferInfo = (offer['offer_id'], offer['owner_id'])
		self.__OpenQuestionDialog(len(question)*6 , accepttext, canceltext, self.__OnAcceptCancelOfferQuestion, self.__OnCancelCancelOfferQuestion, question)

	def __SetSearchFilterPattern(self, info):
		self.__OnClickSearchFilterPage()

		for k, v in self.SearchFilterCheckBoxes.items():
			v.Disable()

		raceFlags = {
			"warrior" :item.ITEM_ANTIFLAG_WARRIOR,
			"assassin" :item.ITEM_ANTIFLAG_ASSASSIN,
			"sura" :item.ITEM_ANTIFLAG_SURA,
			"shaman": item.ITEM_ANTIFLAG_SHAMAN,
			"wolfman": item.ITEM_ANTIFLAG_WOLFMAN,
		}

		flag = info["filter_wearflag"]

		for k, v in raceFlags.items():
			if flag & v != 0:
				self.SearchFilterCheckBoxesRace[k].Disable()
				if not self.SearchFilterCheckBoxes["wear"].IsEnabled():
					self.SearchFilterCheckBoxes["wear"].Enable()

			else:
				self.SearchFilterCheckBoxesRace[k].Enable()

		if info["filter_type"] == 0:
			self.SearchFilterTypeComboBox.SetCurrentItem(localeInfo.OFFLINESHOP_TYPE_COMBOBOX_NOSET)
			self.SearchFilterTypeComboBoxIndex = 0
			self.SearchFilterSubTypeComboBox.SetCurrentItem(localeInfo.OFFLINESHOP_TYPE_COMBOBOX_NOSET)
			self.SearchFilterSubTypeComboBoxIndex = SUBTYPE_NOSET

		else:
			if not self.SearchFilterCheckBoxes["type"].IsEnabled():
				self.SearchFilterCheckBoxes["type"].Enable()

			typeCategory = self.ITEM_TYPES[info["filter_type"]]
			self.SearchFilterTypeComboBox.SetCurrentItem(typeCategory["name"])
			self.SearchFilterTypeComboBoxIndex = info["filter_type"]

			subtype = info["filter_subtype"]
			if subtype != SUBTYPE_NOSET and typeCategory.has_key('subtypes'):
				self.SearchFilterSubTypeComboBox.SetCurrentItem(typeCategory['subtypes'][subtype])
				self.SearchFilterSubTypeComboBoxIndex = subtype
			else:
				self.SearchFilterSubTypeComboBox.SetCurrentItem(localeInfo.OFFLINESHOP_TYPE_COMBOBOX_NOSET)
				self.SearchFilterSubTypeComboBoxIndex = SUBTYPE_NOSET

		if info["filter_name"]:
			self.SearchFilterCheckBoxes["name"].Enable()
			self.SearchFilterItemNameInput.SetText(info["filter_name"])

		levelmin = info["level"]["start"]
		levelmax = info["level"]["end"]

		if levelmin != 0:
			self.SearchFilterCheckBoxes["level"].Enable()
			self.SearchFilterItemLevelStart.SetText(str(levelmin))
		if levelmax != 0:
			self.SearchFilterCheckBoxes["level"].Enable()
			self.SearchFilterItemLevelEnd.SetText(str(levelmax))

		yangmin = info["price"]["start"]
		yangmax = info["price"]["end"]

		if yangmin != 0:
			self.SearchFilterCheckBoxes["price"].Enable()
			self.SearchFilterItemYangMin.SetText(str(yangmin))
		if yangmax != 0:
			self.SearchFilterCheckBoxes["price"].Enable()
			self.SearchFilterItemYangMax.SetText(str(yangmax))

		attrs = info["attr"]["type"]
		attr_actived = [v for v in attrs if v != 0]

		for x in xrange(player.ATTRIBUTE_SLOT_NORM_NUM):
			self.SearchFilterAttributeButtons[x].SetText(localeInfo.OFFLINESHOP_ATTR_UNSET)
			self.SearchFilterAttributeSetting[x] = 0

		if attr_actived:
			self.SearchFilterCheckBoxes["attr"].Enable()
			idx = 0
			for attr in attr_actived:
				self.SearchFilterAttributeButtons[idx].SetText(self.ATTRIBUTES[attr])
				self.SearchFilterAttributeSetting[idx] = attr
				idx += 1

	def __PopupMessage(self, message):
		self.popupMessage.SetText(message)
		self.popupMessage.Open()

	def __OpenQuestionDialog(self , width, accepttext, canceltext, acceptevent, cancelevent, text):
		self.CommonQuestionDlg.SetAcceptText(accepttext)
		self.CommonQuestionDlg.SetCancelText(canceltext)
		self.CommonQuestionDlg.SAFE_SetAcceptEvent(acceptevent)
		self.CommonQuestionDlg.SAFE_SetCancelEvent(cancelevent)
		self.CommonQuestionDlg.SetText(text)
		self.CommonQuestionDlg.SetWidth(width)

		self.CommonQuestionDlg.Open()

	if ENABLE_CHEQUE_SYSTEM:
		def __OnPickValute(self, title, acceptEvent, maxValue, maxCheque, max=13):
			self.CommonPickValuteDlg.SetMax(max)
			self.CommonPickValuteDlg.SetTitleName(title)
			self.CommonPickValuteDlg.SetAcceptEvent(acceptEvent)
			self.CommonPickValuteDlg.Open(maxValue, maxCheque)

	
	else:
		def __OnPickValute(self, title, acceptEvent, maxValue, max=13):
			self.CommonPickValuteDlg.SetMax(max)
			self.CommonPickValuteDlg.SetTitleName(title)
			self.CommonPickValuteDlg.SetAcceptEvent(acceptEvent)
			self.CommonPickValuteDlg.Open(maxValue)

	def OnShowPage(self):
		showEvents = {
			"my_shop"		: self.__OnClickMyShopPage,
		}
		
		if self.pageCategory in showEvents:
			showEvents[self.pageCategory]()

	def BINARY_EnableRefreshSymbol(self):
		if self.IsShow():
			self.EnableRefreshSymbol()

	def EnableRefreshSymbol(self):
		self.RefreshSymbol.Show()
	
	
	def DisableRefreshSymbol(self):
		self.RefreshSymbol.Hide()

	def ShopListClear(self):
		self.ShopList = []

	def ShopListAddItem( self, owner_id, duration , count , name):
		newelement = {}
		newelement["owner_id"]		= owner_id
		newelement["duration"]		= duration
		newelement["count"]			= count
		newelement["name"]			= name
		
		self.ShopList.append(newelement)

	def ShopListShow(self):
		self.RefreshShopListPage()
		self.DisableRefreshSymbol()

	def OpenShop( self, owner_id, duration, count, name):
		self.ShopItemSold 		= []
		self.ShopItemForSale 	= []
		
		self.ShopOpenInfo["owner_id"]	= owner_id
		self.ShopOpenInfo["duration"]	= duration
		self.ShopOpenInfo["count"]		= count
		self.ShopOpenInfo["name"]		= name
		self.ShopOpenInfo["my_shop"]	= False

	def OpenShopItem_Alloc(self):
		self.ShopItemForSale.append({})

	def OpenShopItem_SetValue( self, key,	index,	*args):
		if key == "id":
			self.ShopItemForSale[index][key] = args[0]
		
		elif key == "vnum" or key == 'trans':
			self.ShopItemForSale[index][key] = args[0]
		
		elif key == "count":
			self.ShopItemForSale[index][key] = args[0]

		elif key == "attr":
			if not key in self.ShopItemForSale[index]:
				self.ShopItemForSale[index][key] = {}
			
			attr_index = args[0]
			attr_type  = args[1]
			attr_value = args[2]
			
			self.ShopItemForSale[index][key][attr_index] = {}
			self.ShopItemForSale[index][key][attr_index]["type"]  = attr_type
			self.ShopItemForSale[index][key][attr_index]["value"] = attr_value
		
		elif key == "socket":
			if not key in self.ShopItemForSale[index]:
				self.ShopItemForSale[index][key] = {}
			
			socket_index = args[0]
			socket_val	 = args[1]
			
			self.ShopItemForSale[index][key][socket_index] = socket_val
		
		elif key == "price":
			self.ShopItemForSale[index][key] = args[0]

	def OpenShop_End(self):
		self.RefreshOpenShopPage()
		self.DisableRefreshSymbol()

		if not self.IsShow():
			self.Show()

	def OpenShopOwner_Start( self, owner_id, duration , count , name):
		self.ShopItemSold 		= []
		self.ShopItemForSale 	= []
		self.MyShopOffers		= []

		self.ShopOpenInfo["owner_id"]	= owner_id
		self.ShopOpenInfo["duration"]	= duration
		self.ShopOpenInfo["count"]		= count
		self.ShopOpenInfo["name"]		= name
		self.ShopOpenInfo["my_shop"]	= True
	
	def OpenShopOwner_End(self):
		self.DisableRefreshSymbol()
		self.RefreshMyShopPage()

		if not self.IsShow():
			self.Show()

	def OpenShopOwnerItem_Alloc(self):
		self.ShopItemForSale.append({})
	
	def OpenShopOwnerItem_SetValue( self, key, index, *args):
		if key == "id":
			self.ShopItemForSale[index][key] = args[0]
		
		elif key == "vnum" or key == 'trans':
			self.ShopItemForSale[index][key] = args[0]
		
		elif key == "count":
			self.ShopItemForSale[index][key] = args[0]
		
		elif key == "attr":
			if not key in self.ShopItemForSale[index]:
				self.ShopItemForSale[index][key] = {}
			
			attr_index = args[0]
			attr_type  = args[1]
			attr_value = args[2]
			
			self.ShopItemForSale[index][key][attr_index] = {}
			self.ShopItemForSale[index][key][attr_index]["type"]  = attr_type
			self.ShopItemForSale[index][key][attr_index]["value"] = attr_value
		
		elif key == "socket":
			if not key in self.ShopItemForSale[index]:
				self.ShopItemForSale[index][key] = {}
			
			socket_index = args[0]
			socket_val	 = args[1]
			
			self.ShopItemForSale[index][key][socket_index] = socket_val
		
		elif key == "price":
			self.ShopItemForSale[index][key] = args[0]

	def OpenShopOwner_SetOffer(self, itemid , buyerid, offerid, price, is_accept, buyer_name ):
		newoffer = {}
		newoffer["id"] 			= offerid
		newoffer["item_id"]		= itemid
		newoffer["buyer_id"]	= buyerid
		newoffer["price"]		= price
		newoffer["is_accept"]	= is_accept
		newoffer['buyer_name']  = buyer_name

		self.MyShopOffers.append(newoffer)

	def OpenShopOwnerItemSold_Alloc( self ):
		self.ShopItemSold.append({})
	
	def OpenShopOwnerItemSold_SetValue( self,  key , index , *args):
		if key == "id":
			self.ShopItemSold[index][key] = args[0]
		
		elif key == "vnum" or key == 'trans':
			self.ShopItemSold[index][key] = args[0]
		
		elif key == "count":
			self.ShopItemSold[index][key] = args[0]
		
		elif key == "attr":
			if not key in self.ShopItemSold[index]:
				self.ShopItemSold[index][key] = {}
			
			attr_index = args[0]
			attr_type  = args[1]
			attr_value = args[2]
			
			self.ShopItemSold[index][key][attr_index] = {}
			self.ShopItemSold[index][key][attr_index]["type"]  = attr_type
			self.ShopItemSold[index][key][attr_index]["value"] = attr_value
		
		elif key == "socket":
			if not key in self.ShopItemSold[index]:
				self.ShopItemSold[index][key] = {}
			
			socket_index = args[0]
			socket_val	 = args[1]
			
			self.ShopItemSold[index][key][socket_index] = socket_val
		
		elif key == "price":
			self.ShopItemSold[index][key] = args[0]
	
	def OpenShopOwnerItemSold_Show( self):
		pass

	def OpenShopOwnerNoShop(self):
		for v in self.pageBoards.values():
			v.Hide()
		
		self.pageBoards["create_shop"].Show()
		self.pageCategory = "create_shop"

		self.DisableRefreshSymbol()
		self.__ResetCreateShopPage()

	def ClearItemNames(self):
		self.SearchFilterItemsNameDict = {}

	def AppendItemName(self, vnum, name):
		self.SearchFilterItemsNameDict[name] = vnum

	def ShopClose( self):
		pass

	def ShopFilterResult( self , size):
		self.SearchFilterShopItemResult = []
	
	def ShopFilterResultItem_Alloc(self):
		self.SearchFilterShopItemResult.append({})

	def ShopFilterResultItem_SetValue( self,  key, index, *args):
		if key in ( "id", "vnum", "count", "price", "owner", 'trans'):
			self.SearchFilterShopItemResult[index][key] = args[0]
		
		elif key == "attr":
			if not key in self.SearchFilterShopItemResult[index]:
				self.SearchFilterShopItemResult[index][key] = {}
			
			attr_index = args[0]
			attr_type  = args[1]
			attr_value = args[2]
			
			self.SearchFilterShopItemResult[index][key][attr_index] = {}
			self.SearchFilterShopItemResult[index][key][attr_index]["type"]  = attr_type
			self.SearchFilterShopItemResult[index][key][attr_index]["value"] = attr_value
		
		elif key == "socket":
			if not key in self.SearchFilterShopItemResult[index]:
				self.SearchFilterShopItemResult[index][key] = {}
			
			socket_index = args[0]
			socket_val	 = args[1]
			
			self.SearchFilterShopItemResult[index][key][socket_index] = socket_val

	def ShopFilterResult_Show(self):
		self.SearchFilterResultItemsTable.ClearElement()
		
		for info in self.SearchFilterShopItemResult:
			slot = Slot()
			slot.SetInfo(info)
			slot.SetIndex((info["id"], info["owner"]))
			slot.SetOnMouseLeftButtonUpEvent(self.__OnLeftClickSearchFilterShopResultItem)
			slot.Show()
			
			self.SearchFilterResultItemsTable.AddElement(slot)

		if len(self.SearchFilterShopItemResult) == SEARCH_RESULT_LIMIT:
			self.__PopupMessage(localeInfo.OFFLINESHOP_SEARCH_RISE_LIMIT%SEARCH_RESULT_LIMIT)

		setting = self.GetSearchFilterSettings()
		setting = (len(self.SearchFilterShopItemResult),) + setting

		offlineshop.AppendNewFilterHistory(*setting)
		self.DisableRefreshSymbol()
	
	def OfferReceived( self, offer_id , notified, accepted, owner_id, offerer_id, item_id, yang):
		pass
	
	def OfferAccept( self, offer_id , notified, accepted, owner_id, offerer_id, item_id, yang):
		pass

	def ClearFilterHistory(self):
		self.FilterHistory = []
	
	def AllocFilterHistory( self):
		self.FilterHistory.append({'id' : len(self.FilterHistory)})
	
	def SetFilterHistoryValue( self, key, *args):
		elm = self.FilterHistory[-1]
		
		if key == "datetime":
			elm["minute"]	= args[0]
			elm["hour"]		= args[1]
			elm["day"]		= args[2]
			elm["month"]	= args[3]
			elm["year"]		= args[4]
		
		elif key in ('count' , 'filter_type' , 'filter_subtype', 'filter_name' ,'filter_wearflag'):
			elm[key]		= args[0]
		
		
		elif key in ('filter_price_yang_start', 'filter_price_yang_end'):
			if not 'price' in elm:
				elm['price'] = {}
			
			elm['price'][key.replace('filter_price_yang_', '')] = args[0]
		
		
		elif key in ('filter_level_start', 'filter_level_end'):
			if not 'level' in elm:
				elm['level'] = {}
			
			elm['level'][key.replace('filter_level_', '')] = args[0]
		
		
		elif key in ('filter_attr_type' , 'filter_attr_value'):
			if not 'attr' in elm:
				elm['attr'] = {}
				elm['attr']['type']  = [0 for x in xrange(offlineshop.FILTER_ATTRIBUTE_NUM)]
				elm['attr']['value'] = [0 for x in xrange(offlineshop.FILTER_ATTRIBUTE_NUM)]
			
			elm['attr'][key.replace('filter_attr_', '')][args[0]] = args[1]

	def ClearFilterPatterns( self):
		self.FilterPatterns = {}
	
	def AllocFilterPattern( self , id):
		self.FilterPatterns[id] = {"id": id,}
	
	def SetFilterPatternValue( self, key, idx, *args):
		elm = self.FilterPatterns[idx]
		
		if key == "datetime":
			elm["minute"]	= args[0]
			elm["hour"]		= args[1]
			elm["day"]		= args[2]
			elm["month"]	= args[3]
			elm["year"]		= args[4]
		
		elif key in ('filter_type' , 'filter_subtype', 'filter_name' ,'filter_wearflag', 'name'):
			elm[key]		= args[0]
		
		
		elif key in ('filter_price_yang_start', 'filter_price_yang_end'):
			if not 'price' in elm:
				elm['price'] = {}
			
			elm['price'][key.replace('filter_price_yang_', '')] = args[0]
		
		
		elif key in ('filter_level_start', 'filter_level_end'):
			if not 'level' in elm:
				elm['level'] = {}
			
			elm['level'][key.replace('filter_level_', '')] = args[0]
		
		
		elif key in ('filter_attr_type' , 'filter_attr_value'):
			if not 'attr' in elm:
				elm['attr'] = {}
				elm['attr']['type']  = [0 for x in xrange(offlineshop.FILTER_ATTRIBUTE_NUM)]
				elm['attr']['value'] = [0 for x in xrange(offlineshop.FILTER_ATTRIBUTE_NUM)]
			
			elm['attr'][key.replace('filter_attr_', '')][args[0]] = args[1]

	def GetSearchFilterSettings(self):
		name		= ""
		raceflag	= 0
		type		= 0
		subtype		= SUBTYPE_NOSET
		levelmin	= 0
		levelmax	= 0
		
		yangmin		= 0
		yangmax		= 0
		
		attributes	= tuple([(0,0) for x in xrange(player.ATTRIBUTE_SLOT_NORM_NUM)])

		if self.SearchFilterCheckBoxes["name"].IsEnabled():
			name = self.SearchFilterItemNameInput.GetText()
		
		if self.SearchFilterCheckBoxes["type"].IsEnabled():
			type	= self.SearchFilterTypeComboBoxIndex
			subtype = self.SearchFilterSubTypeComboBoxIndex
		
		if self.SearchFilterCheckBoxes["price"].IsEnabled():
			yangminst 	= self.SearchFilterItemYangMin.GetText()
			yangmaxst 	= self.SearchFilterItemYangMax.GetText()
			
			if yangminst and yangminst.isdigit():
				yangmin = long(yangminst)
			if yangmaxst and yangmaxst.isdigit():
				yangmax = long(yangmaxst)
		
		if self.SearchFilterCheckBoxes["level"].IsEnabled():
			levelminst = self.SearchFilterItemLevelStart.GetText()
			levelmaxst = self.SearchFilterItemLevelEnd.GetText()
			
			if levelminst and levelminst.isdigit():
				levelmin = int(levelminst)
			if levelmaxst and levelmaxst.isdigit():
				levelmax = int(levelmaxst)

		if self.SearchFilterCheckBoxes["wear"].IsEnabled():
			raceFlagDct = {
				"warrior"	: item.ITEM_ANTIFLAG_WARRIOR,
				"assassin"	: item.ITEM_ANTIFLAG_ASSASSIN,
				"sura" 		: item.ITEM_ANTIFLAG_SURA,
				"shaman" 	: item.ITEM_ANTIFLAG_SHAMAN,
			}
			
			if app.ENABLE_WOLFMAN:
				raceFlagDct.update({
					"wolfman" : item.ITEM_ANTIFLAG_WOLFMAN,
				})
						
			for k,v in raceFlagDct.items():
				if not self.SearchFilterCheckBoxesRace[k].IsEnabled():
					raceflag |= v

		if self.SearchFilterCheckBoxes["attr"].IsEnabled():
			attributes = tuple([(self.SearchFilterAttributeSetting[x],0) for x in xrange(player.ATTRIBUTE_SLOT_NORM_NUM)])
		
		return (type, subtype, name, (yangmin,yangmax), (levelmin, levelmax), raceflag, attributes)

	def __IsSaleableSlot(self, win , pos):
		if win == player.INVENTORY:
			if player.IsEquipmentSlot(pos):
				return False

		if self.pageCategory in ("create_shop", "my_shop" ) and self.IsForSaleSlot(win, pos):
			return False

		if self.pageCategory == "create_auction" and self.IsForAuctionSlot(win,pos):
			return False

		if not win in (player.INVENTORY, player.DRAGON_SOUL_INVENTORY):
			return False
			
		if app.ENABLE_BINDING_SYSTEM:
			itemSealDate = player.GetItemSealDate(win, pos)
			if itemSealDate != item.E_SEAL_DATE_DEFAULT_TIMESTAMP:
				return False

		itemIndex = player.GetItemIndex(win,pos)
		if itemIndex == 0:
			return False

		item.SelectItem(itemIndex)
		if item.IsAntiFlag(item.ITEM_ANTIFLAG_MYSHOP) or item.IsAntiFlag(item.ITEM_ANTIFLAG_GIVE):
			return False

		return True

	def ShopBuilding_AddInventoryItem(self, slot):
		if player.GetItemIndex(player.INVENTORY, slot) ==0:
			return

		if player.IsEquipmentSlot(slot):
			return

		if self.IsForSaleSlot(player.INVENTORY,slot):
			return
		
		self.OpenInsertPriceDialog(player.INVENTORY,slot)

	def ShopBuilding_AddItem(self, win, pos):
		if self.__IsSaleableSlot(win, pos):
			self.OpenInsertPriceDialog(win,pos)

	def AuctionBuilding_AddInventoryItem(self, slot):
		if player.GetItemIndex(player.INVENTORY, slot) ==0:
			return

		if player.IsEquipmentSlot(slot):
			return

		if self.IsForAuctionSlot(player.INVENTORY, slot):
			return

		self.__OnSetCreateAuctionSlot(player.INVENTORY, slot)

	def AuctionBuilding_AddItem(self, win, pos):
		if self.__IsSaleableSlot(win, pos):
			self.__OnSetCreateAuctionSlot(win, pos)

	def SearchFilter_BuyFromSearch(selfself, ownerid, itemid):
		if self.pageCategory != 'search_filter':
			return

		for item in self.SearchFilterShopItemResult:
			if item['id'] == itemid and item['owner'] == ownerid:
				self.SearchFilterShopItemResult.remove(item)
				break

		self.SearchFilterResultItemsTable.ClearElement()

		for info in self.SearchFilterShopItemResult:
			slot = Slot()
			slot.SetInfo(info)
			slot.SetIndex((info["id"], info["owner"]))
			slot.SetOnMouseLeftButtonUpEvent(self.__OnLeftClickSearchFilterShopResultItem)
			slot.Show()

			self.SearchFilterResultItemsTable.AddElement(slot)

	def ShopSafebox_Clear(self):
		self.ShopSafeboxItems = []
		self.ShopSafeboxItemsTable.ClearElement()

	if ENABLE_CHEQUE_SYSTEM:
		def ShopSafebox_SetValutes(self, yang, cheque):
			self.ShopSafeboxValuteAmount = (yang, cheque)
			self.ShopSafeboxValuteText.SetText(localeInfo.NumberToMoneyString(yang))
			self.ShopSafeboxValuteTextCheque.SetText(localeInfo.NumberToChequeString(cheque))
	else:
		def ShopSafebox_SetValutes(self, yang):
			self.ShopSafeboxValuteAmount = yang
			self.ShopSafeboxValuteText.SetText(localeInfo.NumberToMoneyString(yang))

	def ShopSafebox_AllocItem(self):
		self.ShopSafeboxItems.append({})

	def ShopSafebox_SetValue(self, key , *args):
		elm = self.ShopSafeboxItems[-1]

		if key in ("id", "vnum", "count", 'trans'):
			elm[key] = args[0]

		elif key == "socket":
			if not key in elm:
				elm[key] = [0 for x in xrange(player.METIN_SOCKET_MAX_NUM)]
			elm[key][args[0]] = args[1]

		elif key in ("attr_type", "attr_value"):
			if not 'attr' in elm:
				elm['attr'] = {}

			if not args[0] in elm['attr']:
				elm['attr'][args[0]] = {}

			elm['attr'][args[0]][key.replace('attr_','')] = args[1]

	def ShopSafebox_RefreshEnd(self):
		for item in self.ShopSafeboxItems:
			slot = Slot()
			slot.SetIndex(item["id"])
			slot.SetInfo(item)
			slot.SetOnMouseLeftButtonUpEvent(self.__OnLeftClickShopSafeboxItem)
			slot.Show()

			self.ShopSafeboxItemsTable.AddElement(slot)

		if not self.pageBoards["shop_safebox"].IsShow():
			self.pageCategory = "shop_safebox"

			for page in self.pageBoards.values():
				page.Hide()

			self.pageBoards["shop_safebox"].Show()

		self.DisableRefreshSymbol()

	def OfferList_Clear(self):
		self.MyOffersList = []

	def OfferList_AddOffer(self , shopname, offer_id, buyer_id, owner_id, item_id, yang, is_notified, is_accept):
		self.MyOffersList.append({})
		elm = self.MyOffersList[-1]

		if '@' in shopname:
			shopname = shopname[shopname.find('@') + 1:]

		elm["shop_name"] 	= shopname
		elm["offer_id"]		= offer_id
		elm["buyer_id"]		= buyer_id
		elm["owner_id"]		= owner_id
		elm["item_id"]		= item_id
		elm["price"]		= yang
		elm["is_notified"]	= is_notified
		elm["is_accept"]	= is_accept

	def	OfferList_ItemSetValue(self, key , *args):
		offer = self.MyOffersList[-1]
		if not offer.has_key('item'):
			offer['item']={}

		elm = offer['item']

		if key in ("id", "vnum", "count", "owner", "price", 'trans'):
			elm[key] = args[0]

		elif key == "socket":
			if not key in elm:
				elm[key] = [0 for x in xrange(player.METIN_SOCKET_MAX_NUM)]
			elm[key][args[0]] = args[1]

		elif key in ("attr_type", "attr_value"):
			if not 'attr' in elm:
				elm['attr'] = {}

			if not args[0] in elm['attr']:
				elm['attr'][args[0]] = {}

			elm['attr'][args[0]][key.replace('attr_','')] = args[1]

	def OfferList_End(self):
		self.RefreshMyOffersPage()
		self.DisableRefreshSymbol()

	def AuctionList_Clear(self):
		self.AuctionListInfo = []
		self.AuctionListTable.ClearElement()

	def AuctionList_Alloc(self):
		self.AuctionListInfo.append({"item":{},})

	def AuctionList_SetInto(self, ownerid, owner_name, duration, init_yang, best_yang, offer_count):
		elm = self.AuctionListInfo[-1]

		info = {
			"owner_id" 		: ownerid,
			"owner_name"	: owner_name,
			"duration"		: duration,
			"init_yang"		: init_yang,
			"best_yang"		: best_yang,
			"offer_count"	: offer_count,
		}

		for k,v in info.items():
			elm[k] = v

	def AuctionList_SetItemInfo(self, vnum, count, *args):
		elm = self.AuctionListInfo[-1]['item']
		elm['count'] 	= count
		elm['vnum']		= vnum

		try:
			if app.ENABLE_CHANGELOOK_SYSTEM and len(args) !=0 :
				elm['trans'] = args[0]
		except:
			pass

	def AuctionList_SetItemSocket(self, index, value):
		elm = self.AuctionListInfo[-1]['item']
		if not 'socket' in elm:
			elm['socket'] = [0 for x in xrange(player.METIN_SOCKET_MAX_NUM)]

		elm['socket'][index] = value

	def AuctionList_SetItemAttribute(self, key, index, value):
		elm = self.AuctionListInfo[-1]['item']
		if not 'attr' in elm:
			elm['attr'] = [{'type' :0, 'value': 0,} for x in xrange(player.ATTRIBUTE_SLOT_MAX_NUM)]
		elm['attr'][index][key] = value

	def AuctionList_End(self):
		self.RefreshAuctionListPage()
		self.DisableRefreshSymbol()

	def MyAuction_Clear(self):
		self.MyAuctionInfo = {'item':{},}

	def MyAuction_SetInto(self, owner_id, owner_name, duration, init_yang):
		info = {
			"owner_id" 		: owner_id,
			"owner_name"	: owner_name,
			"duration"		: duration,
			"init_yang"		: init_yang,
		}

		for k ,v in info.items():
			self.MyAuctionInfo[k] = v

	def MyAuction_SetItemInfo(self, vnum, count, *args):
		elm = self.MyAuctionInfo['item']
		elm['vnum']  = vnum
		elm['count'] = count

		try:
			if app.ENABLE_CHANGELOOK_SYSTEM and len(args)>0:
				elm['trans'] = args[0]
		except:
			pass

	def MyAuction_SetItemSocket(self, index , value):
		elm = self.MyAuctionInfo['item']
		if not 'socket' in elm:
			elm['socket'] = [0 for x in xrange(player.METIN_SOCKET_MAX_NUM)]

		elm['socket'][index] = value

	def MyAuction_SetItemAttribute(self, key , index , value):
		elm = self.MyAuctionInfo['item']
		if not 'attr' in elm:
			elm['attr'] = [{'type' :0, 'value': 0,} for x in xrange(player.ATTRIBUTE_SLOT_MAX_NUM)]
		elm['attr'][index][key] = value


	def MyAuction_AddOffer(self, buyer_id, buyer_name, owner_id, price_yang):
		if not 'offers' in self.MyAuctionInfo:
			self.MyAuctionInfo['offers'] = []

		new = {}
		new['buyer_name'] = buyer_name
		new['price_yang'] = price_yang

		self.MyAuctionInfo['offers'].append(new)

	def MyAuction_End(self):
		self.RefreshMyAuctionPage()
		self.DisableRefreshSymbol()

	def OpenAuction_Clear(self):
		self.OpenAuctionInfo = {'item':{},}

	def OpenAuction_SetInto(self, owner_id, owner_name, duration, init_yang):
		info = {
			"owner_id" 		: owner_id,
			"owner_name"	: owner_name,
			"duration"		: duration,
			"init_yang"		: init_yang,
		}

		for k ,v in info.items():
			self.OpenAuctionInfo[k] = v

	def OpenAuction_SetItemInfo(self, vnum, count, *args):
		elm = self.OpenAuctionInfo['item']
		elm['vnum']  = vnum
		elm['count'] = count

		try:
			if app.ENABLE_CHANGELOOK_SYSTEM and len(args!=0):
				elm['trans'] = args[0]
		except:
			pass

	def OpenAuction_SetItemSocket(self, index , value):
		elm = self.OpenAuctionInfo['item']
		if not 'socket' in elm:
			elm['socket'] = [0 for x in xrange(player.METIN_SOCKET_MAX_NUM)]

		elm['socket'][index] = value

	def OpenAuction_SetItemAttribute(self, key , index , value):
		elm = self.OpenAuctionInfo['item']
		if not 'attr' in elm:
			elm['attr'] = [{'type' :0, 'value': 0,} for x in xrange(player.ATTRIBUTE_SLOT_MAX_NUM)]
		elm['attr'][index][key] = value

	def OpenAuction_AddOffer(self, buyer_id, buyer_name, owner_id, price_yang):
		if not 'offers' in self.OpenAuctionInfo:
			self.OpenAuctionInfo['offers'] = []

		new = {}
		new['buyer_name'] = buyer_name
		new['price_yang'] = price_yang

		self.OpenAuctionInfo['offers'].append(new)

	def OpenAuction_End(self):
		self.RefreshOpenAuctionPage()
		self.DisableRefreshSymbol()

	def MyAuction_NoAuction(self):
		self.pageCategory = "create_auction"
		for page in self.pageBoards.values():
			page.Hide()

		self.pageBoards['create_auction'].Show()
		self.DisableRefreshSymbol()

	def OpenInsertPriceDialog(self, win, slot):
		self.AddItemSlotIndex = (win,slot)
		self.CommonInputPriceDlg.Open()
	
	def __OnAcceptMyShopAcceptOffer(self):
		offlineshop.SendOfferAccept(self.MyShopAcceptOfferID)
		self.CommonQuestionDlg.Hide()
		self.MyShopAcceptOfferID = -1

	def __OnCancelMyShopAcceptOffer(self):
		self.CommonQuestionDlg.Hide()
		self.MyShopAcceptOfferID = -1

	def __OnAcceptMyShopCancelOffer(self):
		offlineshop.SendOfferCancel(self.MyShopCancelOfferID, self.ShopOpenInfo['owner_id'])
		self.CommonQuestionDlg.Hide()
		self.MyShopCancelOfferID = -1

	def __OnCancelMyShopCancelOffer(self):
		self.CommonQuestionDlg.Hide()
		self.MyShopCancelOfferID = -1

	def __OnCancelCancelOfferQuestion(self):
		self.CommonQuestionDlg.Hide()
		self.MyOffersCancelOfferInfo = []

	def __OnAcceptCancelOfferQuestion(self):
		info = self.MyOffersCancelOfferInfo
		offlineshop.SendOfferCancel(info[0], info[1])

		self.CommonQuestionDlg.Hide()
		self.MyOffersCancelOfferInfo = []

	def __OnAcceptInputPrice(self):
		yang = self.CommonInputPriceDlg.GetText()
		if not yang.isdigit():
			return
		
		yang = int(yang)
		if not ENABLE_CHEQUE_SYSTEM:
			if yang == 0:
				return
		
		if ENABLE_CHEQUE_SYSTEM:
			cheque = self.CommonInputPriceDlg.GetChequeText()
			if cheque.isdigit():
				cheque = int(cheque)
			else:
				cheque = 0
		
		
			if yang == 0 and cheque == 0:
				return

		if self.pageCategory == "create_shop" and self.AddItemSlotIndex != -1:
			slot = Slot()
			slot.SetIndex(self.AddItemSlotIndex)
			if ENABLE_CHEQUE_SYSTEM:
				slot.SetInfo(MakeSlotInfo(self.AddItemSlotIndex[0] , self.AddItemSlotIndex[1], yang, cheque))
			else:
				slot.SetInfo(MakeSlotInfo(self.AddItemSlotIndex[0] , self.AddItemSlotIndex[1], yang))
			slot.SetOnMouseLeftButtonUpEvent(self.__OnLeftClickInsertedItem)
			slot.Show()
			
			self.CreateShopItemsTable.AddElement(slot)
			self.CreateShopItemsInfos[slot.GetIndex()] = slot.GetInfo()
			self.CommonInputPriceDlg.Hide()
			
			self.AddItemSlotIndex = -1

		elif self.pageCategory == "create_shop" and self.EditPriceSlot != None:
			slot  = self.EditPriceSlot
			index = slot.GetIndex()
			
			info 			= slot.GetInfo()
			info["price"] 	= yang
			if ENABLE_CHEQUE_SYSTEM:
				info['cheque'] = cheque
			self.CreateShopItemsInfos[slot.GetIndex()] = info
			self.EditPriceSlot = None
			self.CommonInputPriceDlg.Hide()

		elif self.pageCategory == "my_shop" and self.AddItemSlotIndex != -1:
			if ENABLE_CHEQUE_SYSTEM:
				offlineshop.SendAddItem(self.AddItemSlotIndex[0],  self.AddItemSlotIndex[1],  yang, cheque)
			else:
				offlineshop.SendAddItem(self.AddItemSlotIndex[0],  self.AddItemSlotIndex[1],  yang)
			self.AddItemSlotIndex = -1
			self.CommonInputPriceDlg.Hide()
		
		elif self.pageCategory == "my_shop" and self.EditPriceSlot != None:
			slot  = self.EditPriceSlot
			index = slot.GetIndex()
			if ENABLE_CHEQUE_SYSTEM:
				offlineshop.SendEditItem(index, yang, cheque)
			else:
				offlineshop.SendEditItem(index, yang)
			self.CommonInputPriceDlg.Hide()
			self.EditPriceSlot = None
	
	def __OnCancelInputPrice(self):
		self.CommonInputPriceDlg.Hide()
		self.AddItemSlotIndex 	= -1
		self.EditPriceSlot		= None

	def __OnAcceptOpenShopBuyItemQuestion(self):
		self.CommonQuestionDlg.Hide()
		offlineshop.SendBuyItem(self.ShopOpenInfo["owner_id"] , self.OpenShopBuyItemID)
		self.OpenShopBuyItemID = -1

	def __OnCancelOpenShopBuyItemQuestion(self):
		self.CommonQuestionDlg.Hide()
		self.OpenShopBuyItemID = -1

	def __OnAcceptSearchFilterBuyItemQuestion(self):
		id,owner = self.SearchFilterResultClickedInfo
		offlineshop.SendBuyItem(owner, id)

		self.CommonQuestionDlg.Hide()
		self.SearchFilterResultClickedInfo = []

	def __OnCancelSearchFilterBuyItemQuestion(self):
		self.CommonQuestionDlg.Hide()
		self.SearchFilterResultClickedInfo = []

	def __OnAcceptChangeShopNameDlg(self):
		newname = self.MyShopEditNameDlg.GetText()
		self.MyShopEditNameDlg.Hide()
		
		offlineshop.SendChangeName(newname)
	
	def __OnCancelChangeShopNameDlg(self):
		self.MyShopEditNameDlg.Hide()

	def __OnAcceptMyPatternInputName(self):
		name = self.SearchPatternsInputNameDlg.GetText()
		self.SearchPatternsInputNameDlg.Hide()
		if name:
			setting = self.GetSearchFilterSettings()
			setting = (name,) + setting
			offlineshop.AppendNewFilterPattern(*setting)
			self.__PopupMessage(localeInfo.OFFLINESHOP_PATTERN_SAVED_SUCCESS)

	def __OnCancelMyPatternInputName(self):
		self.SearchPatternsInputNameDlg.Hide()

	def __OnAcceptShopSafeboxGetItemQuestion(self):
		self.CommonQuestionDlg.Hide()
		offlineshop.SendSafeboxGetItem(self.ShopSafeboxGetItemIndex)
		self.ShopSafeboxGetItemIndex = -1

	def __OnCancelShopSafeboxGetItemQuestion(self):
		self.CommonQuestionDlg.Hide()
		self.ShopSafeboxGetItemIndex = -1

	if ENABLE_CHEQUE_SYSTEM:
		def __OnAcceptShopSafeboxGetValuteInput(self , yang, cheque):
			offlineshop.SendSafeboxGetValutes(yang, cheque)
	else:
		def __OnAcceptShopSafeboxGetValuteInput(self , yang):
			offlineshop.SendSafeboxGetValutes(yang)

	if ENABLE_CHEQUE_SYSTEM:
		def __OnAcceptMakeOffer(self, yang,  cheque):
			offlineshop.SendOfferCreate(self.MakeOfferOwnerID, self.MakeOfferItemID, yang, cheque)
			self.MakeOfferOwnerID 	= -1
			self.MakeOfferItemID	= -1
	else:
		def __OnAcceptMakeOffer(self, yang):
			offlineshop.SendOfferCreate(self.MakeOfferOwnerID, self.MakeOfferItemID, yang)
			self.MakeOfferOwnerID 	= -1
			self.MakeOfferItemID	= -1

	def __OnClickCreateShopButton(self):
		shopname = self.CreateShopNameEdit.GetText()
		if not shopname:
			self.__PopupMessage(localeInfo.OFFLINESHOP_CREATE_SHOP_NO_NAME_INSERTED)
			return
		
		days  = int(self.CreateShopDaysCountText.GetText())
		hours = int(self.CreateShopHoursCountText.GetText())
		
		totaltime = days * 24 * 60
		totaltime += hours * 60
		
		if totaltime > offlineshop.OFFLINESHOP_MAX_MINUTES or totaltime <= 0:
			self.__PopupMessage(localeInfo.OFFLINESHOP_CREATE_SHOP_INVALID_DURATION)
			return
		
		elements = self.CreateShopItemsTable.GetElementDict()
		if not elements:
			self.__PopupMessage(localeInfo.OFFLINESHOP_CREATE_SHOP_NO_ITEMS_INSERTED)
			return
		
		itemLst = []
		
		for dct in elements.values():
			for item in dct.values():
				info = item.GetInfo()
				
				if ENABLE_CHEQUE_SYSTEM:
					tupleinfo = (info["window"], info["slot"] , info["price"] , info.get('cheque', 0))
				else:
					tupleinfo = (info["window"], info["slot"] , info["price"] )
				itemLst.append(tupleinfo)


		itemTuple = tuple(itemLst)
		offlineshop.SendShopCreate(shopname, totaltime, itemTuple)
		self.EnableRefreshSymbol()

	def __OnLeftClickInsertedItem(self, slot):
		if mouseModule.mouseController.isAttached():
			if self.pageCategory == "create_shop":
				self.__OnClickCreateShopEmptySlot()
			elif self.pageCategory == "my_shop":
				self.__OnClickMyShopEmptySlot()
			return


		if app.IsPressed(app.DIK_LCONTROL) or app.IsPressed(app.DIK_RCONTROL):
			self.__OnRemoveShopItem(slot)
		
		else:
			self.EditPriceSlot = slot
			self.CommonInputPriceDlg.Open()

	def __OnLeftClickShopItem(self, slot):
		if app.IsPressed(app.DIK_LCONTROL) or app.IsPressed(app.DIK_RCONTROL):
			itemid  = slot.GetIndex()
			ownerid = self.ShopOpenInfo["owner_id"]

			self.__OnMakeShopItemOffer(itemid, ownerid)
		else:
			self.OpenShopBuyItemID = slot.GetIndex()
			info = slot.GetInfo()

			item.SelectItem(info["vnum"])
			name 	= item.GetItemName()
			count	= info["count"]
			yang	= info["price"]
			if ENABLE_CHEQUE_SYSTEM:
				cheque	= info["cheque"]

			if count > 1:
				if ENABLE_CHEQUE_SYSTEM:
					text	= localeInfo.OFFLINESHOP_BUY_ITEM_QUESTION_COUNT2 % (name, count, yang, cheque)
				else:
					text	= localeInfo.OFFLINESHOP_BUY_ITEM_QUESTION_COUNT % (name, count, yang)
			else:
				if ENABLE_CHEQUE_SYSTEM:
					text	= localeInfo.OFFLINESHOP_BUY_ITEM_QUESTION2 % (name, yang, cheque)
				else:
					text	= localeInfo.OFFLINESHOP_BUY_ITEM_QUESTION % (name, yang)


			width		= 6 * len(text)
			accept_text	= localeInfo.OFFLINESHOP_BUY_ITEM_QUESTION_ACCEPT
			cancel_text	= localeInfo.OFFLINESHOP_BUY_ITEM_QUESTION_CANCEL
			accept_event= self.__OnAcceptOpenShopBuyItemQuestion
			cancel_event= self.__OnCancelOpenShopBuyItemQuestion

			self.__OpenQuestionDialog(width, accept_text, cancel_text, accept_event, cancel_event, text)

	def __OnLeftClickSearchFilterShopResultItem(self, slot):
		self.SearchFilterResultClickedInfo = slot.GetIndex()
		info = slot.GetInfo()

		item.SelectItem(info["vnum"])
		name = item.GetItemName()
		count = info["count"]
		yang = info["price"]
		if ENABLE_CHEQUE_SYSTEM:
			cheque	= info["cheque"]

		if count > 1:
			if ENABLE_CHEQUE_SYSTEM:
				text = localeInfo.OFFLINESHOP_BUY_ITEM_QUESTION_COUNT2 % (name, count, yang, cheque)
			else:
				text = localeInfo.OFFLINESHOP_BUY_ITEM_QUESTION_COUNT % (name, count, yang)

		else:
			if ENABLE_CHEQUE_SYSTEM:
				text = localeInfo.OFFLINESHOP_BUY_ITEM_QUESTION2 % (name, yang, cheque)
			else:
				text = localeInfo.OFFLINESHOP_BUY_ITEM_QUESTION % (name, yang)

		width = 6 * len(text)
		accept_text = localeInfo.OFFLINESHOP_BUY_ITEM_QUESTION_ACCEPT
		cancel_text = localeInfo.OFFLINESHOP_BUY_ITEM_QUESTION_CANCEL
		accept_event = self.__OnAcceptSearchFilterBuyItemQuestion
		cancel_event = self.__OnCancelSearchFilterBuyItemQuestion

		self.__OpenQuestionDialog(width, accept_text, cancel_text, accept_event, cancel_event, text)

	def __OnClickCreateShopEmptySlot(self, *args):
		if not mouseModule.mouseController.isAttached():
			return

		type = mouseModule.mouseController.GetAttachedType()
		slot = mouseModule.mouseController.GetRealAttachedSlotNumber() #update
		
		type = player.SlotTypeToInvenType(type)

		mouseModule.mouseController.DeattachObject()
		self.ShopBuilding_AddItem(type,slot)

	def __OnClickMyShopEmptySlot(self, *args):
		print("my shop shop empty slot")

		if not mouseModule.mouseController.isAttached():
			return

		type = mouseModule.mouseController.GetAttachedType()
		slot = mouseModule.mouseController.GetRealAttachedSlotNumber() #update

		type = player.SlotTypeToInvenType(type)
		
		mouseModule.mouseController.DeattachObject()
		self.ShopBuilding_AddItem(type,slot)

	def __OnClickShopListOpenShop(self, id):
		offlineshop.SendOpenShop(id)

	def __OnClickOpenAuctionMakeOffer(self, slot):
		if ENABLE_CHEQUE_SYSTEM:
			self.__OnPickValute(localeInfo.OFFLINESHOP_AUCTION_MAKE_OFFER, self.__OnClickAcceptOpenAuctionPickValute, player.GetMoney(), player.GetCheque())
		else:
			self.__OnPickValute(localeInfo.OFFLINESHOP_AUCTION_MAKE_OFFER, self.__OnClickAcceptOpenAuctionPickValute, player.GetMoney())

	if ENABLE_CHEQUE_SYSTEM:
		def __OnClickAcceptOpenAuctionPickValute(self, yang, cheque):
			if not self.OpenAuctionInfo.has_key('min_raise'):
				return

			if (yang + (cheque * YANG_PER_CHEQUE)) < self.OpenAuctionInfo['min_raise']:
				self.__PopupMessage(localeInfo.OFFLINESHOP_AUCTION_MIN_RAISE%NumberToString(self.OpenAuctionInfo['min_raise']) )
				return

			offlineshop.SendAuctionAddOffer(self.OpenAuctionInfo['owner_id'] , yang, cheque)
	else:
		def __OnClickAcceptOpenAuctionPickValute(self, yang):
			if not self.OpenAuctionInfo.has_key('min_raise'):
				return

			if yang < self.OpenAuctionInfo['min_raise']:
				self.__PopupMessage(localeInfo.OFFLINESHOP_AUCTION_MIN_RAISE%NumberToString(self.OpenAuctionInfo['min_raise']) )
				return

			offlineshop.SendAuctionAddOffer(self.OpenAuctionInfo['owner_id'] , yang)

	def __OnClickShopSafeboxWithdrawYang(self):
		maxval = self.ShopSafeboxValuteAmount
		if ENABLE_CHEQUE_SYSTEM:
			self.__OnPickValute(localeInfo.OFFLINESHOP_SAFEBOX_GET_VALUTE_TITLE, self.__OnAcceptShopSafeboxGetValuteInput, maxval[0], maxval[1])
		else:
			self.__OnPickValute(localeInfo.OFFLINESHOP_SAFEBOX_GET_VALUTE_TITLE, self.__OnAcceptShopSafeboxGetValuteInput, maxval)

	def __OnClickMyShopOfferAcceptButton(self, offer):
		self.MyShopAcceptOfferID = offer.GetInfo()['id']

		question 	= localeInfo.OFFLINESHOP_ACCEPT_OFFER_QUESTION
		accepttext 	= localeInfo.OFFLINESHOP_ACCEPT_OFFER_QUESTION_ACCEPT
		canceltext 	= localeInfo.OFFLINESHOP_ACCEPT_OFFER_QUESTION_CANCEL

		self.__OpenQuestionDialog(len(question)*6 , accepttext, canceltext, self.__OnAcceptMyShopAcceptOffer, self.__OnCancelMyShopAcceptOffer, question)

	def __OnClickMyShopOfferCancelButton(self, offer):
		self.MyShopCancelOfferID = offer.GetInfo()['id']
		question 	= localeInfo.OFFLINESHOP_CANCEL_OFFER_QUESTION
		accepttext 	= localeInfo.OFFLINESHOP_CANCEL_OFFER_QUESTION_ACCEPT
		canceltext 	= localeInfo.OFFLINESHOP_CANCEL_OFFER_QUESTION_CANCEL

		self.__OpenQuestionDialog(len(question) * 6, accepttext, canceltext, self.__OnAcceptMyShopCancelOffer, self.__OnCancelMyShopCancelOffer, question)

	def __OnClickOpenAuctionButton(self, idx):
		offlineshop.SendAuctionOpenAuction(idx)
		self.EnableRefreshSymbol()

	def __OnLeftClickShopSafeboxItem(self, slot):
		id = slot.GetIndex()
		self.ShopSafeboxGetItemIndex = id

		if ENABLE_ITEM_WITHDRAW_QUESTION_SHOP_SAFEBOX:
			info = slot.GetInfo()

			vnum = info["vnum"]
			count = info["count"]

			item.SelectItem(vnum)
			name = item.GetItemName()
			if count > 1:
				text = localeInfo.OFFLINESHOP_SHOP_SAFEBOX_QUESTION_GET_ITEM_COUNT%(name,count)
			else:
				text = localeInfo.OFFLINESHOP_SHOP_SAFEBOX_QUESTION_GET_ITEM%name

			width = len(text)*6
			self.__OpenQuestionDialog(width, localeInfo.OFFLINESHOP_SHOP_SAFEBOX_ACCEPT, localeInfo.OFFLINESHOP_SAFEBOX_CANCEL, self.__OnAcceptShopSafeboxGetItemQuestion, self.__OnCancelShopSafeboxGetItemQuestion, text)

		else:
			self.__OnAcceptShopSafeboxGetItemQuestion()

	def __OnRemoveShopItem(self, slot):
		if self.pageCategory == "create_shop":
			self.__OnRemoveShopItemCreateShopPage(slot)
		
		elif self.pageCategory == "my_shop":
			self.__OnRemoveShopItemMyShop(slot)

	def __OnRemoveShopItemCreateShopPage(self, slot):
		slot.Hide()
		del self.CreateShopItemsInfos[slot.GetIndex()]
		self.CreateShopItemsTable.ClearElement()
		
		for k, v in self.CreateShopItemsInfos.items():
			newslot = Slot()
			newslot.SetIndex(k)
			newslot.SetInfo(v)
			newslot.SetOnMouseLeftButtonUpEvent(self.__OnLeftClickInsertedItem)
			newslot.Show()
			
			self.CreateShopItemsTable.AddElement(newslot)

	def __OnRemoveShopItemMyShop(self, slot):
		offlineshop.SendRemoveItem(slot.GetIndex())

	def __OnMakeShopItemOffer(self ,  itemid , ownerid):
		self.MakeOfferItemID = itemid
		self.MakeOfferOwnerID= ownerid

		title = localeInfo.OFFLINESHOP_MAKE_OFFER_TITLE

		if ENABLE_CHEQUE_SYSTEM:
			self.__OnPickValute(title, self.__OnAcceptMakeOffer, player.GetMoney(), player.GetCheque())
		else:
			self.__OnPickValute(title, self.__OnAcceptMakeOffer, player.GetMoney())

	def __OnSetCreateAuctionSlot(self, win,slot):
		vnum = player.GetItemIndex(win,slot)
		if vnum ==0:
			return

		item.SelectItem(vnum)
		if item.IsAntiFlag(item.ITEM_ANTIFLAG_MYSHOP) or item.IsAntiFlag(item.ITEM_ANTIFLAG_GIVE):
			return

		self.CreateAuctionWindowPos = win
		self.CreateAuctionSlotPos	= slot

		count 	= player.GetItemCount(win,slot)
		attrs 	= [{"type" : player.GetItemAttribute(win, slot, x)[0], "value" : player.GetItemAttribute(win, slot, x)[1]} for x in xrange(player.ATTRIBUTE_SLOT_MAX_NUM)]
		sockets = [player.GetItemMetinSocket(win,slot,x) for x in xrange(player.METIN_SOCKET_MAX_NUM)]

		try:
			if app.ENABLE_CHANGELOOK_SYSTEM:
				trans = player.GetItemTransmutation(win, slot)
		except:
			pass

		info = {
			"vnum" 		: vnum,
			"count" 	: count,
			'attr'		: attrs,
			'socket'	: sockets,
		}

		try:
			if app.ENABLE_CHANGELOOK_SYSTEM:
				info.update({
					'trans' : trans,
				})
		except:
			pass


		self.CreateAuctionSlot.SetInfo(info)

	def IsBuildingShop(self):
		return self.IsShow() and (self.pageCategory == "create_shop" or self.pageCategory=="my_shop")

	def IsForSaleSlot(self,win,slot):
		idx = (win,slot)

		if not self.pageCategory=="create_shop" or not self.IsShow():
			return False
		
		if not self.CreateShopItemsTable:
			return False
		
		elementDict = self.CreateShopItemsTable.GetElementDict()
		for dct in elementDict.values():
			for value in dct.values():
				if value.GetIndex() == idx:
					return True
		return False

	def IsBuildingAuction(self):
		if not self.IsShow():
			return False

		if self.pageCategory != "create_auction":
			return False
		return True

	def IsForAuctionSlot(self, win, slot):
		if not self.IsShow():
			return False

		if self.pageCategory != "create_auction":
			return False

		return self.CreateAuctionWindowPos == win and self.CreateAuctionSlotPos == slot

	def __SetTooltip(self,info):
		infocolor = COLOR_TEXT_SHORTCUT

		is_building = self.IsBuildingShop()
		
		self.itemTooltip.ClearToolTip()
		
		sockets = [info["socket"][num] for num in xrange(player.METIN_SOCKET_MAX_NUM)]
		attrs	= [(info["attr"][num]['type'], info["attr"][num]['value']) for num in xrange(player.ATTRIBUTE_SLOT_MAX_NUM)]

		self.itemTooltip.AddItemData(info["vnum"], sockets, attrs)

		try:
			if app.ENABLE_CHANGELOOK_SYSTEM:
				trans = info.get('trans',0)
				if trans !=0 and trans != -1:
					self.itemTooltip.AppendTransmutation(0,0, trans)
		except:
			pass

		if self.pageCategory in ("open_shop", "my_shop", "search_filter", "create_shop"):
			self.itemTooltip.AppendPrice(info["price"])
			if ENABLE_CHEQUE_SYSTEM:
				cheque = info.get('cheque',0)
				if cheque!=0:
					self.itemTooltip.AppendSellingChequePrice(cheque)
		
		if is_building and not info.get('sold', False):
			self.itemTooltip.AppendSpace(10)
			self.itemTooltip.AppendTextLine(localeInfo.OFFLINESHOP_BUILDING_LEFT_CLICK_EDIT_PRICE, infocolor)
			self.itemTooltip.AppendTextLine(localeInfo.OFFLINESHOP_BUILDING_LEFT_CLICK_CTRL_REMOVE_ITEM, infocolor)
		
		elif self.pageCategory == "open_shop":
			self.itemTooltip.AppendSpace(10)
			self.itemTooltip.AppendTextLine(localeInfo.OFFLINESHOP_OPEN_SHOP_LEFT_CLICK_BUY_ITEM, infocolor)
			self.itemTooltip.AppendTextLine(localeInfo.OFFLINESHOP_OPEN_SHOP_LEFT_CLICK_MAKE_OFFER, infocolor)

		elif self.pageCategory == "shop_safebox":
			self.itemTooltip.AppendSpace(10)
			self.itemTooltip.AppendTextLine(localeInfo.OFFLINESHOP_SAFEBOX_TOOLTIP_GETITEM, infocolor)

	def __SetTooltipMyOffer(self, offer):
		slot = offer.slot
		self.__SetTooltip(slot.GetInfo())
		index = offer.info['offer_id']

		for elm in self.MyOffersList:
			if elm['offer_id'] == index:
				self.itemTooltip.AppendSpace(10)
				self.itemTooltip.AppendPrice(elm['item']['price'])
				self.itemTooltip.AppendTextLine(localeInfo.OFFLINESHOP_MY_OFFER_OFFER_TEXT%localeInfo.NumberToMoneyString(elm['price']))
				break

	def __SetTooltipSearch(self, element):
		print("updating tooltip")

		appendLine = lambda arg : (
			uiToolTip.ToolTip.AutoAppendTextLine(self.itemTooltip, arg),
			self.itemTooltip.AlignHorizonalCenter())

		self.itemTooltip.ClearToolTip()

		info = element.info

		name 		= info.get('filter_name', "")
		wearflag	= info.get('filter_wearflag', 0)

		price_min	= info.get('price', {}).get('start', 0)
		price_max	= info.get('price', {}).get('end', 0)

		level_min	= info.get('level', {}).get('start',0)
		level_max	= info.get('level', {}).get('end',0)

		type		= info.get('filter_type', 0)
		subtype		= info.get('filter_subtype', 255)

		attrs		= info.get('attr')

		if name:
			appendLine(localeInfo.OFFLINESHOP_TOOLTIP_NAME_INFO +" "+ name)

		if type:
			type_phrase  = localeInfo.OFFLINESHOP_TOOLTIP_TYPE_INFO+  " "
			type_phrase += self.ITEM_TYPES[type]['name']

			if subtype != SUBTYPE_NOSET and self.ITEM_TYPES[type].has_key('subtypes'):
				type_phrase += ", "+self.ITEM_TYPES[type]['subtypes'][subtype]

			appendLine(type_phrase)

		if price_min or price_max:
			price_phrase = localeInfo.OFFLINESHOP_TOOLTIP_PRICE_INFO + " "
			if price_min and price_max:
				price_phrase += NumberToString(price_min) + "  -  "+ localeInfo.NumberToMoneyString(price_max)

			elif price_min:
				price_phrase += " > "+ localeInfo.NumberToMoneyString(price_min)
			elif price_max:
				price_phrase += " < "+ localeInfo.NumberToMoneyString(price_max)

			appendLine(price_phrase)

		if level_min or level_max:
			level_phrase = localeInfo.OFFLINESHOP_TOOLTIP_LEVEL_INFO + " "
			if level_min and level_max:
				level_phrase += "%d  -  %d"%(level_min, level_max)

			elif level_min:
				level_phrase += " >= %d"%level_min

			elif level_max:
				level_phrase += " <= %d"%level_max

			appendLine(level_phrase)

		raceFlagDct = {
			item.ITEM_ANTIFLAG_WARRIOR : localeInfo.OFFLINESHOP_WEAR_WARRIOR,
			item.ITEM_ANTIFLAG_ASSASSIN	: localeInfo.OFFLINESHOP_WEAR_ASSASSIN,
			item.ITEM_ANTIFLAG_SURA		: localeInfo.OFFLINESHOP_WEAR_SURA,
			item.ITEM_ANTIFLAG_SHAMAN	: localeInfo.OFFLINESHOP_WEAR_SHAMAN,
		}

		if app.ENABLE_WOLFMAN:
			raceFlagDct.update({
				item.ITEM_ANTIFLAG_WOLFMAN: localeInfo.OFFLINESHOP_WEAR_WOLFMAN,
			})

		wearphrase = ""
		for k ,v in raceFlagDct.items():
			if not wearflag & k:
				if wearphrase:
					wearphrase += ", "
				wearphrase += v

		if wearphrase:
			wearphrase = localeInfo.OFFLINESHOP_TOOLTIP_WEAR_INFO + wearphrase
			appendLine(wearphrase)

		attribute_phrase = ""
		for type in attrs['type']:
			if type != 0:
				if attribute_phrase:
					attribute_phrase += ', '
				attribute_phrase += self.ATTRIBUTES[type]

		if attribute_phrase:
			attribute_phrase = localeInfo.OFFLINESHOP_TOOLTIP_ATTR_INFO +' '+attribute_phrase
			appendLine(attribute_phrase)

		self.itemTooltip.ShowToolTip()

	def __SetTooltipAuctionList(self, element):
		itemInfo = element.GetInfo()['item']
		self.__SetTooltip(itemInfo)

	def OnUpdate(self):
		if self.pageCategory in self.updateEvents:
			self.updateEvents[self.pageCategory]()
	
	def __OnUpdateMyShopPage(self):
		self.__RefreshingTooltip(self.MyShopItemsTable)

	def __OnUpdateCreateShopPage(self):
		self.__RefreshingTooltip(self.CreateShopItemsTable)

	def __OnUpdateOpenShopPage(self):
		self.__RefreshingTooltip(self.OpenShopItemsTable)
	
	def __OnUpdateSearchFilterPage(self):
		self.__RefreshingTooltip(self.SearchFilterResultItemsTable)

	def __OnUpdateSearchHistoryPage(self):
		self.__RefreshingTooltip(self.SearchHistoryTable , self.__SetTooltipSearch)

	def __OnUpdateMyPatternPage(self):
		self.__RefreshingTooltip(self.SearchPatternsTable, self.__SetTooltipSearch)

	def __OnUpdateShopSafeboxPage(self):
		self.__RefreshingTooltip(self.ShopSafeboxItemsTable)

	def __OnUpdateMyOffersPage(self):
		self.__RefreshingTooltip(self.MyOffersTable, self.__SetTooltipMyOffer)

	def __OnUpdateCreateAuctionPage(self):
		if self.CreateAuctionSlot.IsShow():
			if self.itemTooltip.IsShow():
				if not self.CreateAuctionSlot.IsInSlot():
					self.itemTooltip.Hide()
			else:
				if self.CreateAuctionSlot.IsInSlot() and self.CreateAuctionSlot.GetInfo():
					self.__SetTooltip(self.CreateAuctionSlot.GetInfo())

	def __OnUpdateMyAuctionPage(self):
		if self.MyAuctionSlot.IsShow():
			if self.itemTooltip.IsShow():
				if not self.MyAuctionSlot.IsInSlot():
					self.itemTooltip.Hide()
			else:
				if self.MyAuctionSlot.IsInSlot() and self.MyAuctionSlot.GetInfo():
					self.__SetTooltip(self.MyAuctionSlot.GetInfo())


	def __OnUpdateOpenAuctionPage(self):
		if self.OpenAuctionSlot.IsShow():
			if self.itemTooltip.IsShow():
				if not self.OpenAuctionSlot.IsInSlot():
					self.itemTooltip.Hide()
			else:
				if self.OpenAuctionSlot.IsInSlot() and self.OpenAuctionSlot.GetInfo():
					self.__SetTooltip(self.OpenAuctionSlot.GetInfo())
					self.itemTooltip.AppendTextLine(localeInfo.OFFLINESHOP_TOOLTIP_LEFT_CLICK_MAKE_OFFER)


	def __OnUpdateAuctionListPage(self):
		self.__RefreshingTooltip(self.AuctionListTable, self.__SetTooltipAuctionList)

	def __RefreshingTooltip(self, table, event = None):
		if not self.itemTooltip.IsShow():
			elementDict = table.GetElementDict()
			for dct in elementDict.values():
				for value in dct.values():
					if value.IsInSlot():
						self.slotTooltipIndex = value.GetIndex()
						if event == None:
							self.__SetTooltip(value.GetInfo())
						else:
							event(value)

						return

			self.itemTooltip.Hide()
			self.slotTooltipIndex = -1

		else:
			elementDict = table.GetElementDict()
			for dct in elementDict.values():
				for value in dct.values():
					if value.IsInSlot():
						if self.slotTooltipIndex == value.GetIndex():
							return

						self.slotTooltipIndex = value.GetIndex()
						if event == None:
							self.__SetTooltip(value.GetInfo())
						else:
							event(value)

						return

			self.itemTooltip.Hide()
			self.slotTooltipIndex = -1

	def RefreshMyShopPage(self):
		self.pageCategory = "my_shop"
		for board in self.pageBoards.values():
			board.Hide()
		
		self.pageBoards["my_shop"].Show()
		name = self.ShopOpenInfo["name"]

		if '@' in name:
			name = name[name.find('@')+1:]

		self.MyShopShopTitle.SetText(name + "  " + localeInfo.OFFLINESHOP_ITEMS_COUNT_TEXT%self.ShopOpenInfo["count"])
		self.MyShopShopDuration.SetText(GetDurationString(self.ShopOpenInfo["duration"]))

		self.MyShopItemsTable.ClearElement()
		for item_info in self.ShopItemForSale:
			
			slot = Slot()
			slot.SetInfo(item_info)
			slot.SetIndex(item_info["id"])
			slot.SetOnMouseLeftButtonUpEvent(self.__OnLeftClickInsertedItem)
			slot.Show()
			
			self.MyShopItemsTable.AddElement(slot)

		for item_info in self.ShopItemSold:
			item_info["sold"] = True

			slot = Slot(isSold=True)
			slot.SetInfo(item_info)
			slot.SetIndex(item_info["id"])
			slot.Show()

			self.MyShopItemsTable.AddElement(slot)

		self.MyShopOffersTable.ClearElement()
		for info in self.MyShopOffers:
			itemname = ""
			for saleitem in self.ShopItemForSale:
				if saleitem["id"] == info["item_id"]:
					item.SelectItem(saleitem["vnum"])
					itemname = item.GetItemName()
					break

			if not itemname:
				continue

			offer = Offer(info)
			offer.Show()
			offer.SetItemName(itemname)
			if not info["is_accept"]:
				offer.SetAcceptButtonEvent(self.__OnClickMyShopOfferAcceptButton)
				offer.SetDeleteButtonEvent(self.__OnClickMyShopOfferCancelButton)

			self.MyShopOffersTable.AddElement(offer)
	
	def RefreshShopListPage(self):
		self.pageCategory = "shop_list"
		for board in self.pageBoards.values():
			board.Hide()
		
		self.pageBoards["shop_list"].Show()
		self.ShopListTable.ClearElement()
		
		for shop in self.ShopList:
			element = ShopListElement(shop)
			element.SetOnClickOpenShopButton(self.__OnClickShopListOpenShop)
			element.Show()

			self.ShopListTable.AddElement(element)
		
		if self.itemTooltip.IsShow():
			self.itemTooltip.Hide()

	def RefreshOpenShopPage(self):
		self.pageCategory = "open_shop"
		for board in self.pageBoards.values():
			board.Hide()
		
		self.pageBoards["open_shop"].Show()
		
		self.OpenShopItemsTable.ClearElement()
		for info in self.ShopItemForSale:
			slot = Slot()
			slot.SetInfo(info)
			slot.SetIndex(info["id"])
			slot.SetOnMouseLeftButtonUpEvent(self.__OnLeftClickShopItem)
			slot.Show()
			
			self.OpenShopItemsTable.AddElement(slot)
		
		name 		= self.ShopOpenInfo["name"]
		duration	= self.ShopOpenInfo["duration"]
		
		if '@' in name:
			name = name[name.find('@')+1:]
		
		self.OpenShopShopTitle.SetText(name+ "  " + localeInfo.OFFLINESHOP_ITEMS_COUNT_TEXT%self.ShopOpenInfo["count"])
		self.OpenShopShopDuration.SetText(GetDurationString(duration))

	def RefreshSearchHistoryPage(self):
		self.SearchHistoryTable.ClearElement()
		offlineshop.RefreshFilterHistory()

		SortByDatetime(self.FilterHistory)

		for elm in self.FilterHistory:
			newElement = FilterHistoryElement(elm)
			newElement.SetButtonEvent(self.__OnClickFilterHistoryElement)
			newElement.Show()
			self.SearchHistoryTable.AddElement(newElement)

	def RefreshMyPatternsPage(self):
		self.SearchPatternsTable.ClearElement()
		offlineshop.RefreshFilterPatterns()

		lst = self.FilterPatterns.values()
		SortByDatetime(lst)

		for v in lst:
			pattern = FilterPatternElement(v)
			pattern.SetButtonEvent(self.__OnClickSearchPatternElement)
			pattern.Show()

			self.SearchPatternsTable.AddElement(pattern)

	def RefreshMyOffersPage(self):
		self.pageCategory = "my_offers"
		for board in self.pageBoards.values():
			board.Hide()

		self.pageBoards["my_offers"].Show()

		self.MyOffersTable.ClearElement()

		for info in self.MyOffersList:

			offer = MyOffer(info)
			offer.Show()
			offer.SetCancelButtonEvent(self.__OnClickDeleteMyOfferButton)
			slot  = offer.slot


			if not info['is_accept']:
				viewImage = MakeOfferViewImage(info['is_notified'])
				slot.AppendChild(viewImage)
				viewImage.SetPosition(5,5)
				viewImage.Show()
			#updated 25-01-2020 #topatch
			else:
				offer.DisableCancelButton()

			self.MyOffersTable.AddElement(offer)

	def RefreshAuctionListPage(self):
		self.pageCategory = "auction_list"
		for board in self.pageBoards.values():
			board.Hide()

		self.pageBoards["auction_list"].Show()

		self.AuctionListTable.ClearElement()

		for info in self.AuctionListInfo:
			elm = AuctionListElement(info)
			elm.Show()
			elm.SetIndex(info['owner_id'])
			elm.SetOnClickOpenAuctionButton(self.__OnClickOpenAuctionButton)

			self.AuctionListTable.AddElement(elm)

	def RefreshMyAuctionPage(self):
		self.pageCategory = "my_auction"
		for board in self.pageBoards.values():
			board.Hide()

		self.pageBoards["my_auction"].Show()

		self.MyAuctionOfferTable.ClearElement()

		if 'offers' in self.MyAuctionInfo:
			for info in SortOffersByPrice(self.MyAuctionInfo['offers']):
				elm = AuctionOffer(info)
				elm.Show()

				self.MyAuctionOfferTable.AddElement(elm)

		best_price = GetBestOfferPriceYang(self.MyAuctionInfo.get('offers', []))
		if best_price ==0:
			min_raise  = self.MyAuctionInfo['init_yang']

		else:
			if best_price < 1000:
				min_raise = best_price + 1000
			else:
				min_raise  = long(float(best_price)*1.1)

		self.MyAuctionBestOffer.SetText(localeInfo.NumberToMoneyString(best_price))
		self.MyAuctionDuration.SetText(GetDurationString(self.MyAuctionInfo['duration']))
		self.MyAuctionMinRaise.SetText(localeInfo.NumberToMoneyString(min_raise))
		self.MyAuctionOwnerName.SetText(self.MyAuctionInfo['owner_name'])

		if 'item' in self.MyAuctionInfo:
			self.__MakeMyAuctionSlot(self.MyAuctionInfo['item'])

	def RefreshOpenAuctionPage(self):
		self.pageCategory = "open_auction"
		for board in self.pageBoards.values():
			board.Hide()

		self.pageBoards["open_auction"].Show()

		self.OpenAuctionOfferTable.ClearElement()

		if 'offers' in self.OpenAuctionInfo:
			for info in SortOffersByPrice(self.OpenAuctionInfo['offers']):
				elm = AuctionOffer(info)
				elm.Show()

				self.OpenAuctionOfferTable.AddElement(elm)

		best_price = GetBestOfferPriceYang(self.OpenAuctionInfo.get('offers' , []))
		if best_price == 0:
			min_raise = self.OpenAuctionInfo['init_yang']

		else:
			if best_price < 1000:
				min_raise = best_price + 1000
			else:
				min_raise = long(float(best_price) * 1.1)

		self.OpenAuctionInfo['min_raise'] = min_raise

		self.OpenAuctionBestOffer.SetText(localeInfo.NumberToMoneyString(best_price))
		self.OpenAuctionDuration.SetText(GetDurationString(self.OpenAuctionInfo['duration']))
		self.OpenAuctionMinRaise.SetText(localeInfo.NumberToMoneyString(min_raise))
		self.OpenAuctionOwnerName.SetText(self.OpenAuctionInfo['owner_name'])

		if 'item' in self.OpenAuctionInfo:
			self.__MakeOpenAuctionSlot(self.OpenAuctionInfo['item'])