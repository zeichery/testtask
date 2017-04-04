#ifndef OBJ_BILLMGR_DEPARTMENT_H
#define OBJ_BILLMGR_DEPARTMENT_H
#include "../../mgr/mgrobject.h"
#include "../main/provider.h"

using namespace test_functions;

namespace billmgr {
namespace object {
/**
* @brief Класс для работы с отделами BILLmanager
*
*/
template <typename Provider = DefaultAdminQuery>
class Department : public test_mgrobject::MgrElem
{
public:
	Department(): test_mgrobject::MgrElem(new Provider) {
		key = "id";
		props["sok"] = "ok";
		func = "department";
	}

	~Department() {
		//т.к. при интеграции с VMManager используется вирт.машина, с которой взаимодействуют не только тесты BillManager,
		//то после завершения тестов из базы удаляются задачи ("сервер не найден в BillManager").
#if defined(BILL_VMMGR) || defined(BILL_VEMGR)
		test_functions::dbConnect<Provider> db;
		db.GetStr("DELETE from task WHERE department='" + props[key] + "'");
		functions::ClearCache("task");
//		test_functions::mgrRestart<Provider>();
#endif
		DELETE_OBJECT();
	}
	void CreateDefault() {
		props["name"] = GenName(9);
		auto& locales = functions::GetLocaleList();
		ForEachI(locales, e) {
			if (*e != "en")
				props["name_" + *e] = props["name"];
		}
		Create();
	}
	/**
	* @brief Подключаем сотрудника к отделу
	*
	*/
	void AddEmployee(const string& employeeid) {
		string query = "func=" + func + ".employee.resume&plid=" + props[key] + "&elid=" + employeeid;
		Query(query);
	}
};

/**
* @brief Класс для работы со списком отделов
*
*/
template <typename Provider = DefaultAdminQuery>
class DepartmentList : public test_mgrobject::MgrList
{
public:
	DepartmentList() : test_mgrobject::MgrList(new Provider) {
		func = "department";
		key = "id";
	}
};
}
}
#endif // OBJ_BILLMGR_DEPARTMENT_H
