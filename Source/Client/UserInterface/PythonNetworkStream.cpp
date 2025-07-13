1* Search for:
			Set(HEADER_GC_LOVE_POINT_UPDATE, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCLovePointUpdate), STATIC_SIZE_PACKET));

2* Add below:
#ifdef __ENABLE_NEW_OFFLINESHOP__
			Set(HEADER_GC_NEW_OFFLINESHOP, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCNewOfflineshop), DYNAMIC_SIZE_PACKET));
#endif