1* Search for:
	Set(HEADER_CG_PARTY_USE_SKILL, sizeof(TPacketCGPartyUseSkill), "PartyUseSkill", true);

2* Add below:
#ifdef __ENABLE_NEW_OFFLINESHOP__
	Set(HEADER_CG_NEW_OFFLINESHOP, sizeof(TPacketCGNewOfflineShop), "NewOfflineshop", true);
#endif