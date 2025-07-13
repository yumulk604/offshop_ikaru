1* Search for:
class ScriptWindow(Window):

2* Add above:
if app.__ENABLE_NEW_OFFLINESHOP__:
	class ExpandedButton(Window):
		STATUS_DEFAULT 	= 0
		STATUS_DOWN		= 1
		STATUS_OVER		= 2

		def __init__(self, template):
			Window.__init__(self)
			self.SetWindowName("ExpandedButton")
			self.defaultImage = None
			self.downImage = None
			self.overImage = None
			self.eventLeftClick = None
			self.updateEvent = None
			self.downEvent = None
			self.baseInfo = {}
			self.status = 0

			default = template.get('default', 	"")
			down	= template.get('down', 		"")
			over	= template.get('over',		"")
			event	= template.get('event',		None)
			update	= template.get('update',	None)
			rotation= template.get('rotation',	0)
			downEv	= template.get('downevent', None)

			if rotation!=0:
				self.baseInfo['rotated'] = True

			if not default or not down or not over:
				dbg.TraceError("RotatedButton : cannot set template [%s]"%str(template))
				return

			self.__SetDefaultVisual(default)
			self.__SetDownVisual(down)
			self.__SetOverVisual(over)

			if event:
				self.__SetEvent(event)

			if rotation:
				self.__SetRotation(rotation)

			self.SetOnMouseLeftButtonUpEvent(self.__OnClickMe)

			if update:
				self.updateEvent = update

			if downEv:
				self.downEvent = downEv

		def __del__(self):
			self.defaultImage = None
			self.downImage = None
			self.overImage = None
			self.eventLeftClick = None
			self.updateEvent = None
			self.baseInfo = {}
			self.status = 0
			self.downEvent = None
			Window.__del__(self)

		def __SetDefaultVisual(self, default):
			img = ExpandedImageBox()
			img.LoadImage(default)
			img.SetParent(self)
			img.SetPosition(0,0)
			img.Show()
			img.SetOnMouseLeftButtonUpEvent(self.__OnClickMe)
			img.OnMouseOverIn = self.__OverIn
			img.OnMouseOverOut= self.__OverOut
			img.OnMouseLeftButtonDown = self.__OnMouseDown
			img.SetWindowName("ExpandedButton_Default")

			self.baseInfo['default'] 	= {'width': img.GetWidth(), 'height': img.GetHeight(),}
			self.defaultImage 			= img
			self.SetSize(img.GetWidth(), img.GetHeight())

		def __SetDownVisual(self, down):
			img = ExpandedImageBox()
			img.LoadImage(down)
			img.SetParent(self)
			img.SetPosition(0, 0)
			img.Hide()
			img.SetOnMouseLeftButtonUpEvent(self.__OnClickMe)
			img.OnMouseOverIn = self.__OverIn
			img.OnMouseOverOut = self.__OverOut
			img.OnMouseLeftButtonDown = self.__OnMouseDown
			img.SetWindowName("ExpandedButton_Down")
			self.baseInfo['down'] = {'width': img.GetWidth(), 'height': img.GetHeight(), }
			self.downImage = img

		def __SetOverVisual(self, over):
			img = ExpandedImageBox()
			img.LoadImage(over)
			img.SetParent(self)
			img.SetPosition(0, 0)
			img.Hide()
			img.SetOnMouseLeftButtonUpEvent(self.__OnClickMe)
			img.OnMouseOverIn = self.__OverIn
			img.OnMouseOverOut = self.__OverOut
			img.OnMouseLeftButtonDown = self.__OnMouseDown
			img.SetWindowName("ExpandedButton_Over")
			self.baseInfo['over'] = {'width': img.GetWidth(), 'height': img.GetHeight(), }
			self.overImage = img

		def __SetEvent(self, event):
			self.eventLeftClick = event

		def __SetRotation(self, rotation):
			self.defaultImage.SetRotation(rotation)
			self.downImage.SetRotation(rotation)
			self.overImage.SetRotation(rotation)

		def __OnClickMe(self):
			if self.eventLeftClick:
				self.eventLeftClick()
			self.status = self.STATUS_DEFAULT
			self.__RefreshView()

		def __OverIn(self):
			if self.status==self.STATUS_DOWN:
				return

			self.status = self.STATUS_OVER
			self.__RefreshView()

		def __OverOut(self):
			if self.status != self.STATUS_DOWN:
				self.status = self.STATUS_DEFAULT
				self.__RefreshView()
			print("over out")

		def __OnMouseDown(self):
			self.status = self.STATUS_DOWN
			self.__RefreshView()
			if self.downEvent:
				self.downEvent()
			print("mouse down")

		def __RefreshView(self):
			images = {
				self.STATUS_DEFAULT : self.defaultImage,
				self.STATUS_DOWN	: self.downImage,
				self.STATUS_OVER	: self.overImage,
			}

			for image in images.values():
				image.Hide()
			images[self.status].Show()

		def OnUpdate(self):
			if self.updateEvent:
				self.updateEvent()

		def SetScale(self, x , y):
			self.defaultImage.SetScale(x,y)
			self.downImage.SetScale(x,y)
			self.overImage.SetScale(x,y)
			if self.baseInfo.get('rotated', False):
				width = self.baseInfo['default']['height']
				height= self.baseInfo['default']['width']
			else:
				height = self.baseInfo['default']['height']
				width = self.baseInfo['default']['width']
			newwidth  = int( float(width)  * x)
			newheight = int( float(height) * y)
			self.SetSize(newwidth, newheight)

	class CustomScrollBar(Window):
		HORIZONTAL 	= 1
		VERTICAL	= 2
		BOTTOM		= 1
		TOP			= 2
		RIGHT		= 3
		LEFT		= 4

		def __init__(self, template):
			Window.__init__(self)
			self.baseImage = None
			self.button1 = None
			self.button2 = None
			self.middleButton = None
			self.onScroll = None
			self.parent = None
			self.orientation = 0
			self.pos = 0.0
			self.middleScale = 0.1
			self.step = 0.1
			self.baseInfo = {}
			self.mouseOffset = {}

			base	= template.get('base', 			"")
			button1 = template.get('button1', 		{})
			button2 = template.get('button2', 		{})
			middle	= template.get('middle', 		{})
			onscroll= template.get('onscroll', 		None)
			orient	= template.get('orientation',	0)
			align	= template.get('align',			{})
			parent	= template.get('parent',		None)
			position= template.get('position',		{})

			if not base or not button1 or not button2 or not middle or not onscroll or not orient or not parent:
				dbg.TraceError("CustomScrollBar : cannot set template [%s]"%str(template))
				return

			self.__SetParent(parent)
			self.__SetOrientation(orient)
			self.__LoadBaseImage(base)
			self.__LoadButton1(button1)
			self.__LoadButton2(button2)
			self.__LoadMiddleButton(middle)
			self.__SetOnScrollEvent(onscroll)
			if template.has_key('align'):
				self.__SetAlign(align)

			elif template.has_key('position'):
				self.__SetPosition(position)

		def __SetParent(self, parent):
			if parent:
				self.parent = parent
				self.SetParent(parent)

		def __SetOrientation(self, orient):
			self.orientation = orient

		def __LoadBaseImage(self, base):
			bg = ExpandedImageBox()
			bg.LoadImage(base)
			bg.SetParent(self)
			bg.SetPosition(0,0)
			bg.Show()
			w , h = (bg.GetWidth() , bg.GetHeight())
			self.baseInfo = {'base' : {'width':w, 'height':h,}}
			self.SetSize(w,h)
			bg.OnMouseLeftButtonDown = self.__OnClickBaseImage
			self.baseImage = bg

		def __LoadButton1(self, button1):
			button1['event'] = self.__OnClickButton1
			btn = ExpandedButton(button1)
			btn.SetParent(self.baseImage)
			btn.SetPosition(0,0)
			btn.Show()
			self.button1 = btn

		def __LoadButton2(self, button2):
			button2['event'] = self.__OnClickButton2
			btn = ExpandedButton(button2)
			btn.SetParent(self.baseImage)

			if self.orientation == self.HORIZONTAL:
				leng = btn.GetWidth()
				btn.SetPosition(self.GetWidth() - leng , 0)

			elif self.orientation == self.VERTICAL:
				leng = btn.GetHeight()
				btn.SetPosition(0, self.GetHeight() - leng)
			btn.Show()

			self.button2 = btn

		def __LoadMiddleButton(self, middle):
			middle['downevent'] = self.__OnClickMiddle
			middle['update']	= self.__OnUpdateMiddleBar
			btn = ExpandedButton(middle)
			btn.SetParent(self.baseImage)

			if self.orientation == self.HORIZONTAL:
				btn.SetPosition(self.button1.GetWidth(), 0)

			elif self.orientation == self.VERTICAL:
				btn.SetPosition(0, self.button1.GetHeight())
			btn.Show()
			self.middleButton = btn

		def __SetOnScrollEvent(self, onscroll):
			self.onScroll = onscroll

		def __SetAlign(self, align):
			mode	= align['mode']
			offset1	= align.get('offset1',0)
			offset2	= align.get('offset2',0)
			if not self.parent:
				return
			if self.orientation == self.HORIZONTAL:
				if mode == self.TOP:
					self.SetPosition(offset1, 0)
				if mode == self.BOTTOM:
					self.SetPosition(offset1, self.parent.GetHeight() - self.GetHeight())
				self.SetScrollBarLength(self.parent.GetWidth() - (offset1 + offset2))
			elif self.orientation == self.VERTICAL:
				if mode == self.RIGHT:
					self.SetPosition(self.parent.GetWidth()-self.GetWidth(),  offset1 )
				elif mode == self.LEFT:
					self.SetPosition(0, offset1)
				self.SetScrollBarLength(self.parent.GetHeight() - (offset1 + offset2))

		def __SetPosition(self, position):
			self.SetPosition(position['x'] , position['y'])

		def SetScrollBarLength(self, leng):
			if self.orientation == self.VERTICAL:
				self.SetSize(self.GetWidth(), leng)
				baseScale = float(leng) / float(self.baseInfo['base']['height'])
				self.baseImage.SetScale(1.0, baseScale)
				scrollsize  = leng - (self.__GetElementLength(self.button1) + self.__GetElementLength(self.button2))
				middle_leng = int(self.middleScale * scrollsize)
				init_middle = float(self.middleButton.baseInfo['default']['height'])
				self.middleButton.SetScale(1.0, float(middle_leng)/init_middle)
				self.middleButton.SetPosition(0,self.__GetElementLength(self.button1) + int((scrollsize - self.__GetElementLength(self.middleButton))* self.pos))
				self.button2.SetPosition(0, self.GetHeight()-self.button2.GetHeight())
			elif self.orientation == self.HORIZONTAL:
				self.SetSize(leng, self.GetHeight())
				baseScale = float(leng) / float(self.baseInfo['base']['width'])
				self.baseImage.SetScale(baseScale, 1.0)
				scrollsize = leng - (self.__GetElementLength(self.button1) + self.__GetElementLength(self.button2))
				middle_leng = int(self.middleScale * scrollsize)
				init_middle = float(self.middleButton.baseInfo['default']['width'])
				self.middleButton.SetScale(float(middle_leng) / init_middle, 1.0)
				self.middleButton.SetPosition(self.__GetElementLength(self.button1) + int((scrollsize - self.__GetElementLength(self.middleButton)) * self.pos),0)
				self.button2.SetPosition(self.GetWidth() - self.button2.GetWidth(), 0)

		def __GetElementLength(self, element):
			if self.orientation == self.VERTICAL:
				return element.GetHeight()
			if self.orientation == self.HORIZONTAL:
				return element.GetWidth()
			return 0

		def __OnUpdateMiddleBar(self):
			if self.middleButton.status != ExpandedButton.STATUS_DOWN:
				return
			x,y 	= wndMgr.GetMousePosition()
			gx,gy	= self.middleButton.GetGlobalPosition()
			gx += self.mouseOffset.get('x',0)
			gy += self.mouseOffset.get('y',0)
			if self.orientation == self.VERTICAL:
				if y == gy:
					return
			elif self.orientation == self.HORIZONTAL:
				if x == gx:
					return
			self.__OnMoveMiddleBar(x,y)

		def __OnClickBaseImage(self):
			x,y 	= wndMgr.GetMousePosition()
			gx,gy	= self.middleButton.GetGlobalPosition()
			offset = self.__GetElementLength(self.middleButton)/2
			gx += offset
			gy += offset
			if self.orientation == self.VERTICAL:
				if y == gy:
					return
			elif self.orientation == self.HORIZONTAL:
				if x == gx:
					return
			self.mouseOffset = {'x' : offset, 'y': offset}
			self.__OnMoveMiddleBar(x,y)

		def __OnClickButton2(self):
			self.mouseOffset={'x' : 0, 'y' :0}
			gx,gy = self.middleButton.GetGlobalPosition()
			if self.orientation == self.VERTICAL:
				gy += self.__GetElementLength(self.middleButton)
			elif self.orientation == self.HORIZONTAL:
				gx += self.__GetElementLength(self.middleButton)
			self.__OnMoveMiddleBar(gx,gy)

		def __OnClickButton1(self):
			self.mouseOffset={'x' : 0, 'y' :0}
			gx, gy = self.middleButton.GetGlobalPosition()
			if self.orientation == self.VERTICAL:
				gy -= self.__GetElementLength(self.middleButton)
			elif self.orientation == self.HORIZONTAL:
				gx -= self.__GetElementLength(self.middleButton)
			self.__OnMoveMiddleBar(gx, gy)

		def __OnMoveMiddleBar(self, x , y):
			gx, gy = self.GetGlobalPosition()
			x -= self.mouseOffset.get('x', 0)
			y -= self.mouseOffset.get('y', 0)
			if self.orientation == self.VERTICAL:
				min_ = gy  + self.__GetElementLength(self.button1)
				max_ = min_ + (self.GetHeight() - (self.__GetElementLength(self.button1) + self.__GetElementLength(self.button2) + self.__GetElementLength(self.middleButton)))
				if max_ < y and self.pos == 1.0:
					return
				if min_ > y and self.pos == 0.0:
					return
				realy = max(y, min_)
				realy = min(realy, max_)
				scroll= max_-min_
				if scroll == 0.0:
					return
				self.pos = float(realy-min_) / float(scroll)
				self.middleButton.SetPosition(0, realy-gy)
				self.__OnScroll()

			elif self.orientation == self.HORIZONTAL:
				min_ = gx + self.__GetElementLength(self.button1)
				max_ = min_ + (self.GetWidth() - (self.__GetElementLength(self.button1) + self.__GetElementLength(self.button2) + self.__GetElementLength(self.middleButton)))
				if max_ < x and self.pos == 1.0:
					return
				if min_ > x and self.pos == 0.0:
					return
				realx = max(x, min_)
				realx = min(realx, max_)
				scroll = max_ - min_
				if scroll == 0.0:
					return
				self.pos = float(realx - min_) / float(scroll)
				self.middleButton.SetPosition(realx-gx, 0)
				self.__OnScroll()

		def __OnScroll(self):
			if self.onScroll:
				self.onScroll()

		def __OnClickMiddle(self):
			x,y 	= wndMgr.GetMousePosition()
			gx,gy	= self.middleButton.GetGlobalPosition()
			x-= gx
			y-= gy
			self.mouseOffset = {"x" : x, "y": y,}

		def GetPos(self):
			return self.pos

		def GetStep(self):
			return self.step

		def SetScrollStep(self, step):
			step = min(1.0, max(0.1 , step))
			self.middleScale = step
			self.step = step
			self.SetScrollBarLength(self.__GetElementLength(self.baseImage))

		if app.ENABLE_WHEEL_MOUSE:
			def OnRunMouseWheel(self, nLen):
				if nLen > 0:
					self.__OnClickButton1()
				else:
					self.__OnClickButton2()	

	class CheckBox(Window):
		def __init__(self, images = {}):
			Window.__init__(self)
			self.clear()
			self.__loadCheckBox(images)
			self.SetOnMouseLeftButtonUpEvent(self.__OnClick)
			self.SetWindowName("checkbox")

		def clear(self):
			self.status = "disabled"
			self.bg_image = None
			self.tip_image = None

		def __del__(self):
			self.status		= "disabled"
			self.bg_image	= None
			self.tip_image	= None
			Window.__del__(self)
		
		def __loadCheckBox(self, images):
			bg = ImageBox()
			bg.SetParent(self)
			bg.SetPosition(0,0)
			if not images:
				bg.LoadImage("d:/ymir work/ui/pattern/checkbox_bg.png")
			else:
				bg.LoadImage(images['base'])
			bg.SetOnMouseLeftButtonUpEvent(self.__OnClick)
			bg.SetWindowName("checkbox_bg")
			self.SetSize(bg.GetWidth(), bg.GetHeight())
			bg.Show()
			self.bg_image = bg
			tip = ImageBox()
			tip.SetParent(self.bg_image)
			if not images:
				tip.LoadImage("d:/ymir work/ui/pattern/checkbox_tip.png")
			else:
				tip.LoadImage(images['tip'])
			tip.SetPosition(0 , 0)
			tip.Show()
			tip.SetWindowName("checkbox_tip")
			tip.SetOnMouseLeftButtonUpEvent(self.__OnClick)
			self.tip_image = tip
			self.__refreshView()

		def __refreshView(self):
			if self.status == "enabled":
				self.tip_image.Show()
			else:
				self.tip_image.Hide()

		def __OnClick(self):
			if self.status == "disabled":
				self.status = "enabled"
			else:
				self.status = "disabled"
			print("clicked!")
			self.__refreshView()

		def IsEnabled(self):
			return self.status == "enabled"

		def Enable(self):
			self.status = "enabled"
			self.__refreshView()

		def Disable(self):
			self.status = "disabled"
			self.__refreshView()
