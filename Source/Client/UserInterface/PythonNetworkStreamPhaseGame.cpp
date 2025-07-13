1* Search for:
			case HEADER_GC_SPECIFIC_EFFECT:
				ret = RecvSpecificEffect();
				break;

2* Add below:
#ifdef __ENABLE_NEW_OFFLINESHOP__
			case HEADER_GC_NEW_OFFLINESHOP:
				ret = RecvOfflineshopPacket();
				break;
#endif

3* Search for:
void CPythonNetworkStream::__LeaveGamePhase()
{
	CInstanceBase::ClearPVPKeySystem();

	__ClearNetworkActorManager();

	m_bComboSkillFlag = FALSE;

	IAbstractCharacterManager& rkChrMgr=IAbstractCharacterManager::GetSingleton();
	rkChrMgr.Destroy();

	CPythonItem& rkItemMgr=CPythonItem::Instance();
	rkItemMgr.Destroy();
}

4* Reaplce the function with:
void CPythonNetworkStream::__LeaveGamePhase()
{
	CInstanceBase::ClearPVPKeySystem();

	__ClearNetworkActorManager();

	m_bComboSkillFlag = FALSE;

	IAbstractCharacterManager& rkChrMgr=IAbstractCharacterManager::GetSingleton();
	rkChrMgr.Destroy();

	CPythonItem& rkItemMgr=CPythonItem::Instance();
	rkItemMgr.Destroy();

#ifdef __ENABLE_NEW_OFFLINESHOP__
#	ifdef __ENABLE_NEW_SHOP_IN_CITIES__
	CPythonOfflineshop::instance().DeleteEntities();
#	endif
#endif
}