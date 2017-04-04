#ifndef OBJ_BILLMGR_USER_H
#define OBJ_BILLMGR_USER_H
#include "../../mgr/mgrobject.h"
#include "obj.h"
#include "../main/utils.h"

namespace billmgr {
namespace object {
/**
* @brief Класс для работы с пользователями BILLmanager
* 
*/
template <typename Provider = DefaultAdminQuery>
class User : public BILLmgrElem
{
public:
	User(): BILLmgrElem(new Provider) {
		key = "id";
		props["sok"] = "ok";
		func = "user";
	}

	void Open(const string& key_value) {
		CheckCreate();
		deleteOnDestroy = false;
		BILLmgrElem::Open(key_value);
	}

	void CreateDefault() {
		props["realname"] = GenName(6);
		props["email"] = GenMailBoxName();
		props["passwd"] = GenName(12);
		Create();
	}
};

/**
* @brief Класс для работы со списком пользователей
* 
*/
template <typename Provider = DefaultAdminQuery>
class UserList : public BILLmgrList
{
public:
	UserList() : BILLmgrList(new Provider) {
		func = "user";
		key = "id";
	}

	/**
	 * @brief	Получаем список email пользователей по accountid
	 * @param	[in] accountid.
	 * @return	Возвращает Stringlist для account
	 * 
	 */
	StringList GetUsersByAccount(const string& accountid) const {
		StringMap filter;
		StringList resultList, fields;
		fields.push_back("name");
		filter["account_id"] = accountid;
		GetList(resultList, fields, filter);
		if (resultList.empty())
			resultList.push_back("");
		return resultList;
	}

	/**
	 * @brief	Получаем id пользователя по accountId
	 * @param	[in] accountId.
	 * @return	Возвращает id пользователя
	 * 
	 */
	string GetUserIdByAccount(const string& accountId) const {
		StringMap filter;
		StringList resultList, fields;
		fields.push_back("id");
		filter["account_id"] = accountId;
		GetList(resultList, fields, filter);
		if (resultList.empty())
			resultList.push_back("");
		return resultList.front();
	}
};

}
}
#endif // OBJ_BILLMGR_USER_H
