*1 Search for:
CItemData * CItemManager::GetSelectedItemDataPointer()

*2 Add above:
#ifdef __ENABLE_NEW_OFFLINESHOP__
void CItemManager::GetItemsNameMap(std::map<DWORD, std::string>& inMap)
{
	inMap.clear();
	for (auto& it : m_ItemMap)
	{
		inMap.insert(std::make_pair(it.first, it.second->GetName()));
	}
}
#endif
