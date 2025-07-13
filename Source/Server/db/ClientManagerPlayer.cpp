1* Search for:
}
void CClientManager::ItemAward(CPeer * peer,char* login)

or

}

void CClientManager::ItemAward(CPeer * peer,char* login)

2* Add above:
#ifdef __ENABLE_NEW_OFFLINESHOP__
	OfflineshopLoadShopSafebox(peer, packet->player_id);
#endif

3* Search for:
	char szQuery[128];
	snprintf(szQuery, sizeof(szQuery), "SELECT p.id, p.level, p.name FROM player_index%s AS i, player%s AS p WHERE pid%u=%u AND pid%u=p.id",
			GetTablePostfix(), GetTablePostfix(), packet->account_index + 1, packet->player_id, packet->account_index + 1);

	ClientHandleInfo * pi = new ClientHandleInfo(dwHandle, packet->player_id);
	pi->account_index = packet->account_index;

	sys_log(0, "PLAYER_DELETE TRY: %s %d pid%d", packet->login, packet->player_id, packet->account_index + 1);
	CDBManager::instance().ReturnQuery(szQuery, QID_PLAYER_DELETE, peer->GetHandle(), pi);
}

4* Add above:
#ifdef __ENABLE_NEW_OFFLINESHOP__
	if (IsUsingOfflineshopSystem(packet->player_id))
	{
		sys_log(0, "PLAYER_DELETE FAILED %u IS USING OFFLINESHOP SYSTEM", packet->player_id);
		peer->EncodeHeader(HEADER_DG_PLAYER_DELETE_FAILED, dwHandle, 1);
		peer->EncodeBYTE(packet->account_index);
		return;
	}
#endif