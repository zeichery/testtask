#ifndef OBJ_BILLMGR_ACCOUNT_H
#define OBJ_BILLMGR_ACCOUNT_H
#include "../../mgr/mgrobject.h"
#include "obj.h"
#include "../main/utils.h"
#include "user.h"
namespace billmgr {
namespace object {
/**
* @brief Класс для работы с клиентами BILLmanager
*
*/
template <typename Provider = DefaultAdminQuery>
class Account : public BILLmgrElem
{
public:
	Account(): BILLmgrElem(new Provider) {

		key = "id";
		props["sok"] = "ok";
		func = "account";
		// Для последующего получения данных аккаунта
		props["registration_date"] = "";
		props["registration_ip"] = "";
	}

	~Account() {
		DELETE_OBJECT();
	}

	void CreateDefault() {
		props["realname"] = GenName(6);
		props["email"] = GenMailBoxName();
		props["passwd"] = GenName();
		Create();
	}

	/**
	* @brief	Создает дефолтного клиента без почты
	*
	*/
	void CreateWithoutEmail(const string &country) {
		props["realname"] = GenName(6);
		props["passwd"] = GenName();
		props["country"] = country;
		Create();
		}

	/**
	 * @brief	Возвращает полное имя клиента realname+email
	 *
	 */
	std::string GetAccountName() const {
		return props["realname"] + " (" + props["email"] + ")";
	}

	/**
	 * @brief	Возвращает id, чтобы использовать для Su
	 *
	 */
	std::string SU() const {
		object::UserList <Provider> userList;
		return userList.GetUserIdByAccount(props[key]);
	}

	/**
	 * @brief В биллинге есть лонгтаск на создание клиента. Из-за него в кэше даже после удаления остаются записи
	 */
	void DeleteBody() {
		BILLmgrElem::DeleteBody();
		functions::ClearCache("account");
	}

	void AccountCreditLimit(const string& subaccount_id, const string& account_id, const string& creditlimit) {
		string query = "elid=" + subaccount_id + "&plid=" + account_id +"&func=subaccount.edit&sok=ok&creditlimit=" + creditlimit;
		Query(query);
	}

};

/**
* @brief Класс для работы со списком клиентов
*
*/
template <typename Provider = DefaultAdminQuery>
class AccountList : public BILLmgrList
{
public:
	AccountList() : BILLmgrList(new Provider) {
		func = "account";
		key = "id";
	}

	/**
	 * @brief	Проверяем, есть ли скидка у клиента
	 * @param	[in] accountid.
	 * @return	Возвращает true если есть.
	 *
	 */
	bool HasDiscount(const string& accountid) {
		StringMap filter;
		filter["has_discount"] = "on";
		filter["id"] = accountid;
		return HasItems(filter);
	}
};

/**
* @brief Класс для работы с персональными скидками
*
*/
template <typename Provider = DefaultAdminQuery>
class AccountDiscount : public BILLmgrElem
{
private:
	Account<Provider>* p_account;
public:
	enum type	 {		percent		= 0
					,	specprice		= 1
				 };
	AccountDiscount(Account<Provider>* account)
		: BILLmgrElem(new Provider)
		, p_account		(account)
	{
		props["id"] = "";
		props["plid"] = p_account->GetProp("id");
		//необходимые поля
		props["percentage"] = "30";
		props["type"] = str::Str(percent);
		props["pricelist_tree"] = "null"; //формат pl_6, it_31, i_1
		props["fromdate"] = mgr_date::Date().AddMonth(-1).AsDate();
		props["todate"] = mgr_date::Date().AddMonth().AsDate();
		props["monthly126"] = "";//если спеццена

		key = "id";
		func ="account.discount";
		props["sok"] = "ok";
	}

	~AccountDiscount() {
		DELETE_OBJECT();
	}

	//Перегруженный Create, потому что визард
	void Create() {
		BILLmgrElem::Create("", false, "", "", "account.discount.add.product");
	}
};

/**
* @brief Класс для работы со списком персональных скидок клиентов под админом
*
*/
template <typename Provider = DefaultAdminQuery>
class AccountDiscountList : public BILLmgrList
{
public:
	AccountDiscountList() : BILLmgrList(new Provider) {
		func = "account.discount";
		key = "id";
	}

	/**
	 * @brief	Проверяем, есть ли скидка у клиента
	 * @param	[in] accountid.
	 * @return	Возвращает true если есть.
	 *
	 */
	bool HasDiscount(const string& discountid) const{
		StringMap filter;
		filter["id"] = discountid;
		return HasItems(filter);
	}
};

/**
* @brief Класс для работы со списком скидок под клиентом
*
*/
template <typename Provider = DefaultAdminQuery>
class DiscountInfoList : public BILLmgrList
{
public:
	DiscountInfoList() : BILLmgrList(new Provider) {
		func = "account.discountinfo";
		key = "id";
	}

	/**
	 * @brief	Проверяем, есть ли скидка у клиента
	 * @param	[in] accountid.
	 * @return	Возвращает true если есть.
	 *
	 */
	bool HasDiscount(const string& discountid) const{
		StringMap filter;
		filter["id"] = discountid;
		return HasItems(filter);
	}
};

/**
* @brief Класс для работы со списком провайдеров клиента
*
*/
template <typename Provider = DefaultAdminQuery>
class AccountProjectList : public BILLmgrList
{
public:
	AccountProjectList(const string &elid) : BILLmgrList(new Provider) {
		func = "account.project";
		props["elid"] = elid;
		key = "id";
	}

	/**
	 * @brief	Проверяем, подключен ли клиент к проекту
	 * @param	[in] projectid.
	 * @return	Возвращает true если подключен.
	 *
	 */
	bool IsProjectEnabled(const string &projectid) const{
		StringMap filter;
		filter["active"] = "on";
		filter["id"] = projectid;
		return HasItems(filter);
	}
};

}
}
#endif // OBJ_BILLMGR_ACCOUNT_H
