1* Search for:
#include "cmd.h"

2* Add below:
#ifdef __ENABLE_NEW_OFFLINESHOP__
#include "new_offlineshop.h"
#include "new_offlineshop_manager.h"
#endif

3* Add to the end of file:
#ifdef __ENABLE_NEW_OFFLINESHOP__
std::string GetNewShopName(const std::string& oldname, const std::string& newname)
{
	auto nameindex = oldname.find('@');
	if (nameindex == std::string::npos)
	{
		return newname;
	}
	else
	{
		auto playername = oldname.substr(0, nameindex);
		return playername + '@' + newname;
	}
}

ACMD(do_offshop_change_shop_name)
{
	char arg1[50]; char arg2[256];
	argument = one_argument(argument, arg1, sizeof(arg1));
	argument = one_argument(argument, arg2, sizeof(arg2));

	if (arg1[0] != 0 && isdigit(arg1[0]) && arg2[0] != 0)
	{
		DWORD id = 0;
		str_to_number(id, arg1);

		if (id == 0)
		{
			ch->ChatInfoTrans(("syntax: offshop_change_shop_name <player-id> <new-name>"));
			return;
		}
		else 
		{
			offlineshop::CShop* pkShop = offlineshop::GetManager().GetShopByOwnerID(id);
			if (!pkShop)
			{
			ch->ChatInfoTrans(("Cannot find shop by id %u "), id);
				return;
			} else
			{
				std::string oldname = pkShop->GetName();
				offlineshop::GetManager().SendShopChangeNameDBPacket(id, GetNewShopName(oldname, arg2).c_str());
				ch->ChatInfoTrans(("shop's name changed."));
			}
		}

	}
	else
	{
		ch->ChatInfoTrans(("syntax: offshop_change_shop_name <player-id> <new-name>"));
		return;
	}
}

ACMD(do_offshop_force_close_shop)
{
	char arg1[50];
	argument = one_argument(argument, arg1, sizeof(arg1));
	if (arg1[0] != 0 && isdigit(arg1[0]))
	{

		DWORD id = 0;
		str_to_number(id, arg1);

		if (id == 0)
		{
			ch->ChatInfoTrans(("syntax: offshop_force_close_shop <player-id>"));
			return;
		}
		else
		{
			offlineshop::CShop* pkShop = offlineshop::GetManager().GetShopByOwnerID(id);
			if (!pkShop)
			{
				ch->ChatInfoTrans(("Cannot find shop by id %u"), id);
				return;
			}
			else
			{
				offlineshop::GetManager().SendShopForceCloseDBPacket(id);
				ch->ChatInfoTrans(("shop closed successfully."));
			}
		}

	}
	else
	{
		ch->ChatInfoTrans(("syntax: offshop_force_close_shop <player-id>"));
		return;
	}
}
#endif
