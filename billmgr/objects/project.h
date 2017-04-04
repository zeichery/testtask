#ifndef OBJ_BILLMGR_PROVIDER_H
#define OBJ_BILLMGR_PROVIDER_H
#include "../../mgr/mgrobject.h"
#include "obj.h"
#include "../main/provider.h"

namespace billmgr {
namespace object {
/**
* @brief Класс для работы с провайдерами BILLmanager
*
*/
template <typename Provider = DefaultAdminQuery>
class Project : public test_mgrobject::MgrElem
{
public:
	Project(): test_mgrobject::MgrElem(new Provider) {

		key = "id";
		props["sok"] = "ok";
		func = "project";
	}

	void CreateDefault() {
		props["name"] = GenName();
		props["notifyemail"] = GenMailBoxName();
		Create();
	}

	/**
	* @brief Подключить валюту к провайдеру
	* @param [id] Id валюты
	*
	*/
	void AddCurrency(const string& id) {
		Query("func=project.currency.resume&elid=" + id + "&plid=" + props["id"]);
	}

	/**
	* @brief Подключить компанию к проекту
	* @param [companyId] Id провайдера
	*
	*/
	void ConnectCompany(const string& companyId) {
		Query("func=project.company.resume"
			"&elid=" + companyId  +
			"&plid=" + props["id"]);
	}

	/**
	* @brief Отключить компанию от проекта
	* @param [companyId] Id компании
	*
	*/
	void DisconnectCompany(const string& companyId) {
		Query("func=project.company.suspend"
			"&elid=" + companyId +
			"&plid=" + props["id"]);
	}

	~Project() {
		DELETE_OBJECT();
	}
};

/**
* @brief Класс для работы с группами провайдеров BILLmanager
*
*/
template <typename Provider = DefaultAdminQuery>
class ProjectGroup : public BILLmgrElem
{
public:
	ProjectGroup(const string &projectId):  BILLmgrElem(new Provider){

		props["id"] = "";
		props["plid"]=projectId;
		key = "id";
		func = "project.group";
		props["sok"] = "ok";
	}

	void CreateDefault() {
		props["name"]=GenName();
		Create();
	}

	//Включить провайдера в группу провайдеров
	void ProjectResume(const string& projectgroup_id) {
		string query = "func=" + func +".resume_ext&elid=" + props["id"]  + "&plid=" + projectgroup_id;
		Query(query);
	}

	//Исключить провайдера из группы провайдеров
	void ProjectSuspend(const string& projectgroup_id) {
		string query = "func=" + func +".suspend&elid=" + props["id"] + "&plid=" + projectgroup_id;
		Query(query);
	}


};

/**
* @brief Класс для работы со списком групп провайдеров
*
*/
template <typename Provider = DefaultAdminQuery>
class ProjectGroupList : public BILLmgrList
{
public:
	ProjectGroupList(const string &projectId) : BILLmgrList(new Provider) {
		func = "project.group";
		props["elid"]=projectId;
		key = "id";
	}

	bool IsGroupEnabled(const string &groupid) const{
		StringMap filter;
		filter["status"] = "on";
		filter["id"] = groupid;
		return HasItems(filter);
	}
};

/**
* @brief Класс для работы со списком провайдеров
*
*/
template <typename Provider = DefaultAdminQuery>
class ProjectList : public test_mgrobject::MgrList
{
public:
	ProjectList() : test_mgrobject::MgrList(new Provider) {
		func = "project";
		key = "id";
	}
};

/**
* @brief Класс для работы со списком компаний провайдера
*
*/
template <typename Provider = DefaultAdminQuery>
class ProjectCompanyList : public test_mgrobject::MgrList
{
public:
	ProjectCompanyList(const string &elid) : test_mgrobject::MgrList(new Provider) {
		func = "project.company";
		props["elid"] = elid;
		key = "id";
	}
	/**
	 * @brief	Проверяем, подключена ли компания к провайдеру
	 * @param	[in] companyId
	 * @return	Возвращает true, если подключена
	 *
	 */
	bool IsCompanyEnabled(const string &companyid) const{
			StringMap filter;
			filter["active"] = "on";
			filter["id"] = companyid;
			return HasItems(filter);
		}
};
}
}
#endif // OBJ_BILLMGR_PROVIDER_H
