#ifndef OBJ_BILLMGR_PROFILE_H
#define OBJ_BILLMGR_PROFILE_H
#include "../../mgr/mgrobject.h"
#include "account.h"
#include "../main/provider.h"

namespace billmgr {
namespace object {


/**
* @brief Класс для работы с плательщиками BILLmanager
* 
*/
template <typename Provider = DefaultAdminQuery>
class Profile : public BILLmgrElem
{
public:
	enum ProfileType {	  prPersonal		= 1
						, prCompany			= 2
						, prSoleProprietor	= 3
					 };
	Profile()
		: BILLmgrElem(new Provider)
	{
		props["address_legal"] = "";
		props["passwd"] = "";
		props["project"] = "";
		props["currency"] = "";
		props["realname"] = "";
		props["note"] = "";
		props["country_legal"] = "14";
		props["profiletype"] = str::Str(prPersonal);
		props["person"] = GenName(6);
		props["name"] = GenName(6);

		key = "id";
		props["sok"] = "ok";
		func = "profile";
		su = "";
	}
};

}
}
#endif // OBJ_BILLMGR_PROFILE_H
