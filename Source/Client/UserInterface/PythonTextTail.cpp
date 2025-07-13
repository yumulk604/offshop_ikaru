1* Search for:
void CPythonTextTail::RenderTextTailBox(TTextTail * pTextTail)
{

2* Add below:
#ifdef __ENABLE_NEW_OFFLINESHOP__
#	ifdef __ENABLE_NEW_SHOP_IN_CITIES__
	if (pTextTail->bIsShop)
	{
		CPythonGraphic::Instance().SetDiffuseColor(0.0f, 0.0f, 0.0f, 1.0f);
		CPythonGraphic::Instance().RenderBox2d(pTextTail->x + pTextTail->xStart - 10.f,
			pTextTail->y + pTextTail->yStart - 10.f,
			pTextTail->x + pTextTail->xEnd + 10.f,
			pTextTail->y + pTextTail->yEnd + 10.f,
			pTextTail->z);
		CPythonGraphic::Instance().SetDiffuseColor(0.0f, 0.0f, 0.0f, 0.3f);
		CPythonGraphic::Instance().RenderBar2d(pTextTail->x + pTextTail->xStart - 10.f,
			pTextTail->y + pTextTail->yStart - 10.f,
			pTextTail->x + pTextTail->xEnd + 10.f,
			pTextTail->y + pTextTail->yEnd + 10.f,
			pTextTail->z);
		return;
	}
#	endif
#endif

3* Search for:
	pTextTail->dwVirtualID = dwVirtualID;

4* Add above:
#ifdef __ENABLE_NEW_OFFLINESHOP__
#	ifdef __ENABLE_NEW_SHOP_IN_CITIES__
	pTextTail->bIsShop = false;
	pTextTail->bRender = false;
#	endif
#endif

5* Search for:
#include "PythonTextTail.h"

6* Add below:
#ifdef __ENABLE_NEW_SHOP_IN_CITIES__
#include "PythonApplication.h"
#endif

7* Search for in void CPythonTextTail::UpdateAllTextTail():
		for (TChatTailMap::iterator itorChat = m_ChatTailMap.begin(); itorChat != m_ChatTailMap.end(); ++itorChat)
		{

*8 Add above:
#ifdef __ENABLE_NEW_SHOP_IN_CITIES__
		for (itorMap = m_ShopTextTailMap.begin(); itorMap != m_ShopTextTailMap.end(); ++itorMap)
		{
			UpdateDistance(pixelPos, itorMap->second);
		}
#endif

9* Search for in void CPythonTextTail::UpdateShowingTextTail():
	for (itor = m_CharacterTextTailList.begin(); itor != m_CharacterTextTailList.end(); ++itor)
	{

10* Add above:
#ifdef __ENABLE_NEW_SHOP_IN_CITIES__
	for (auto itorMap = m_ShopTextTailMap.begin(); itorMap != m_ShopTextTailMap.end(); ++itorMap)
	{
		if (itorMap->second->bRender)
		{
			UpdateTextTail(itorMap->second);
		}
	}
#endif

11* Search for in void CPythonTextTail::ArrangeTextTail():
	for (itor = m_CharacterTextTailList.begin(); itor != m_CharacterTextTailList.end(); ++itor)
	{

12* Add above:
#ifdef __ENABLE_NEW_SHOP_IN_CITIES__
	for (auto itorMap = m_ShopTextTailMap.begin(); itorMap != m_ShopTextTailMap.end(); ++itorMap)
	{
		if (!itorMap->second->bRender)
		{
			continue;
		}

		TTextTail * pInsertTextTail = itorMap->second;

		if (pInsertTextTail->pOwnerTextInstance)
		{
			pInsertTextTail->pOwnerTextInstance->SetPosition(pInsertTextTail->x, pInsertTextTail->y, pInsertTextTail->z);
			pInsertTextTail->pOwnerTextInstance->Update();
			pInsertTextTail->pTextInstance->SetColor(pInsertTextTail->Color.r, pInsertTextTail->Color.g, pInsertTextTail->Color.b);
			pInsertTextTail->pTextInstance->SetPosition(pInsertTextTail->x, pInsertTextTail->y + 15.0f, pInsertTextTail->z);
			pInsertTextTail->pTextInstance->Update();
		}
		else
		{
			pInsertTextTail->pTextInstance->SetColor(pInsertTextTail->Color.r, pInsertTextTail->Color.g, pInsertTextTail->Color.b);
			pInsertTextTail->pTextInstance->SetPosition(pInsertTextTail->x, pInsertTextTail->y, pInsertTextTail->z);
			pInsertTextTail->pTextInstance->Update();
		}
	}
#endif

13* Search for in void CPythonTextTail::Render():
	for (TChatTailMap::iterator itorChat = m_ChatTailMap.begin(); itorChat != m_ChatTailMap.end(); ++itorChat)
	{

14* Add above:
#ifdef __ENABLE_NEW_SHOP_IN_CITIES__
	for (auto itorMap = m_ShopTextTailMap.begin(); itorMap != m_ShopTextTailMap.end(); ++itorMap)
	{
		if (!itorMap->second->bRender)
		{
			continue;
		}
		TTextTail * pTextTail = itorMap->second;
		RenderTextTailBox(pTextTail);
		pTextTail->pTextInstance->Render();
		if (pTextTail->pOwnerTextInstance)
		{
			pTextTail->pOwnerTextInstance->Render();
		}
	}
#endif

15* Search for in void CPythonTextTail::HideAllTextTail():
	m_ItemTextTailList.clear();

16* Add below:
#ifdef __ENABLE_NEW_SHOP_IN_CITIES__
	for (auto& iter : m_ShopTextTailMap)
	{
		iter.second->bRender = false;
	}
#endif

17* Search for in void CPythonTextTail::ShowAllTextTail():
	for (itor = m_ItemTextTailMap.begin(); itor != m_ItemTextTailMap.end(); ++itor)
	{
		TTextTail* pTextTail = itor->second;

		if (pTextTail->fDistanceFromPlayer < 3500.0f)
		{
			ShowItemTextTail(itor->first);
		}
	}

18* Add below:
#ifdef __ENABLE_NEW_SHOP_IN_CITIES__
	for (itor = m_ShopTextTailMap.begin(); itor != m_ShopTextTailMap.end(); ++itor)
	{
		TTextTail* pTextTail = itor->second;
		pTextTail->bRender = pTextTail->fDistanceFromPlayer < 3500.f;
	}
#endif

19* Search for:
bool CPythonTextTail::isIn(CPythonTextTail::TTextTail* pSource, CPythonTextTail::TTextTail* pTarget)
{

20* Add above:
#ifdef __ENABLE_NEW_SHOP_IN_CITIES__
void CPythonTextTail::RegisterShopInstanceTextTail(DWORD dwVirtualID, const char* c_szName, CGraphicObjectInstance* pOwner)
{
	TTextTail * pTextTail = RegisterShopTextTail(dwVirtualID, c_szName, pOwner);
	m_ShopTextTailMap.insert(TTextTailMap::value_type(dwVirtualID, pTextTail));
}
#endif

21* Search for:
void CPythonTextTail::DeleteCharacterTextTail(DWORD VirtualID)
{

22* Add above:
#ifdef __ENABLE_NEW_SHOP_IN_CITIES__
void CPythonTextTail::DeleteShopTextTail(DWORD VirtualID)
{
	TTextTailMap::iterator itor = m_ShopTextTailMap.find(VirtualID);

	if (m_ShopTextTailMap.end() == itor)
	{
		Tracef(" CPythonTextTail::DeleteShopTextTail - None Item Text Tail\n");
		return;
	}

	DeleteTextTail(itor->second);
	m_ShopTextTailMap.erase(itor);
}
#endif

23* Search for:
void CPythonTextTail::DeleteItemTextTail(DWORD VirtualID)

24* Add after all function:
#ifdef __ENABLE_NEW_SHOP_IN_CITIES__
CPythonTextTail::TTextTail * CPythonTextTail::RegisterShopTextTail(DWORD dwVirtualID, const char * c_szText, CGraphicObjectInstance * pOwner)
{
	const D3DXCOLOR & c_rColor = D3DXCOLOR(1.0, 1.0, 0.5, 1.0);
	TTextTail * pTextTail = m_TextTailPool.Alloc();
	pTextTail->bIsShop = true;
	pTextTail->dwVirtualID = dwVirtualID;
	pTextTail->pOwner = pOwner;
	pTextTail->pTextInstance = CGraphicTextInstance::New();
	pTextTail->pOwnerTextInstance = NULL;
	pTextTail->fHeight = 180.f;
	pTextTail->pTextInstance->SetTextPointer(ms_pFont);
	pTextTail->pTextInstance->SetHorizonalAlign(CGraphicTextInstance::HORIZONTAL_ALIGN_CENTER);
	pTextTail->pTextInstance->SetValue(c_szText);
	pTextTail->pTextInstance->SetColor(c_rColor.r, c_rColor.g, c_rColor.b);
	pTextTail->pTextInstance->Update();
	int xSize, ySize;
	pTextTail->pTextInstance->GetTextSize(&xSize, &ySize);
	pTextTail->xStart = (float)(-xSize / 2 - 2);
	pTextTail->yStart = -2.0f;
	pTextTail->xEnd = (float)(xSize / 2 + 2);
	pTextTail->yEnd = (float)ySize;
	pTextTail->Color = c_rColor;
	pTextTail->fDistanceFromPlayer = 0.0f;
	pTextTail->x = -100.0f;
	pTextTail->y = -100.0f;
	pTextTail->z = 0.0f;
	pTextTail->pMarkInstance = NULL;
	pTextTail->pGuildNameTextInstance = NULL;
	pTextTail->pTitleTextInstance = NULL;
	pTextTail->pLevelTextInstance = NULL;
	// pTextTail->pQuestTitleTextInstance = NULL;
#ifdef ENABLE_GLOBAL_LANGUAGE
	pTextTail->pCFlagInstance = NULL;
	pTextTail->pLanguageImageInstance = NULL;
#endif
#ifdef ENABLE_REBORN_SYSTEM
	pTextTail->pReborn_TitleText_tInstance = NULL;
#endif
	return pTextTail;
}

bool CPythonTextTail::GetPickedNewShop(DWORD* pdwVID)
{
	*pdwVID = 0;

	if (!CPythonOfflineshop::instance().GetShowNameFlag() && !CPythonSystem::instance().IsAlwaysShowName())
	{
		return false;
	}

	long ixMouse = 0, iyMouse = 0;

	POINT p;
	CPythonApplication::Instance().GetMousePosition(&p);

	ixMouse = p.x;
	iyMouse = p.y;

	for (auto itor = m_ShopTextTailMap.begin(); itor != m_ShopTextTailMap.end(); ++itor)
	{
		TTextTail * pTextTail = itor->second;
		if (ixMouse >= pTextTail->x + (pTextTail->xStart - 10) && ixMouse <= pTextTail->x + (pTextTail->xEnd + 10) &&
			iyMouse >= pTextTail->y + (pTextTail->yStart - 10) && iyMouse <= pTextTail->y + (pTextTail->yEnd + 10))
		{
			*pdwVID = itor->first;
			return true;
		}
	}

	return false;
}
#endif

25* Search for in void CPythonTextTail::Clear():
	m_TextTailPool.Clear();

26* Add below:
#ifdef __ENABLE_NEW_SHOP_IN_CITIES__
	m_ShopTextTailMap.clear();
#endif