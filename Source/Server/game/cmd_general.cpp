1* Add to the end of file:
#ifdef __ENABLE_NEW_OFFLINESHOP__
ACMD(do_showofflineshop)
{
	ch->ChatPacket(CHAT_TYPE_COMMAND, "ShowMeOfflineShop");
}
#endif