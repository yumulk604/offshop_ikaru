1* Search for:
		case HEADER_GC_QUICKSLOT_ADD:
			if (RecvQuickSlotAddPacket())
				return;
			break;

2* Add below:
#ifdef __ENABLE_NEW_OFFLINESHOP__
		case HEADER_GC_NEW_OFFLINESHOP:
			RecvOfflineshopPacket();
			return;
#endif