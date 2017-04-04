#ifndef OBJ_BILLMGR_COMPANY_H
#define OBJ_BILLMGR_COMPANY_H
#include "../../mgr/mgrobject.h"
#include "obj.h"


namespace billmgr {
namespace object {
/**
* @brief Класс для работы с компаниями BILLmanager
* 
*/
template <typename Provider = DefaultAdminQuery>
class Company : public BILLmgrElem
{
private:
	test_mgrobject::AutoWizard m_wizard;
public:
	Company()
		: BILLmgrElem(new Provider)
		, m_wizard		(*this, "company.add")
	{
		key = "id";
		props["sok"] = "ok";
		func = "company";
	}

	/**
	* @brief Подключить компанию к провайдеру
	* @param [providerId] Id провайдера
	* 
	*/
	void ConnectToProject(const string& projectId) {
		Query("func=company.project.resume"
			"&elid=" + projectId  +
			"&plid=" + props["id"]);
	}

	/**
	* @brief Отключить компанию от провайдера
	* @param [providerId] Id провайдера
	* 
	*/
	void DisconnectFromProject(const string& projectId) {
		Query("func=company.project.suspend"
			"&elid=" + projectId  +
			"&plid=" + props["id"]);
	}


	~Company() {
		DELETE_OBJECT();
	}

	void CreateByWizard() {
		m_wizard.GoAllSteps();
	}

	void CreateDefault() {
		props["name"] = GenName();
		CreateByWizard();
	}
};

/**
* @brief Класс для работы со списком компаний
* 
*/
template <typename Provider = DefaultAdminQuery>
class CompanyList : public test_mgrobject::MgrList
{
public:
	CompanyList() : test_mgrobject::MgrList(new Provider) {
		func = "company";
		key = "id";
	}
};

/**
* @brief Класс для работы со списком провайдеров компании
* 
*/
template <typename Provider = DefaultAdminQuery>
class CompanyProjectList : public test_mgrobject::MgrList
{
public:
	CompanyProjectList(const string &elid) : test_mgrobject::MgrList(new Provider) {
		func = "company.project";
		props["elid"] = elid;
		key = "id";
	}
	/**
	 * @brief	Проверяем, подключен ли провайдер к компании
	 * @param	[in] projectId
	 * @return	Возвращает true, если подключен
	 *
	 */
	bool IsProjectEnabled(const string &projectId) const{
		StringMap filter;
		filter["active"] = "on";
		filter["id"] = projectId;
		return HasItems(filter);
	}
};
}
}

#endif // OBJ_BILLMGR_COMPANY_H
