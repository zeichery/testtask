#ifndef OBJ_BILLMGR_EMPLOYEE_H
#define OBJ_BILLMGR_EMPLOYEE_H
#include "../../mgr/mgrobject.h"
#include "../../coremgr/obj_core_common.h"
using namespace test_functions;

namespace billmgr {
namespace object {
/**
* @brief Класс для работы с сотрудниками BILLmanager
*
*/
template <typename Provider = test_mgrobject::LocalQuery>
class Employee : public CoreObjects::CommonElem
{
public:
	Employee(): CoreObjects::CommonElem(new Provider) {

		key = "id";
		props["sok"] = "ok";
		func = "employee";
	}

	~Employee() {
		DELETE_OBJECT();
	}

	void CreateDefault() {
		props["name"] = GenName(6);
		props["passwd"] = GenName();
		props["phone"] = functions::GenPhoneNumber();
		props[functions::AddLocalePostfix("realname")] = GenName();
		props["email"] = GenMailBoxName();
		props["default_access_allow"] = "on";
		Create();
	}
};

/**
* @brief Класс для работы со списком сотрудников
*
*/
template <typename Provider = test_mgrobject::LocalQuery>
class EmployeeList : public test_mgrobject::MgrList
{
public:
	EmployeeList() : test_mgrobject::MgrList(new Provider) {
		func = "employee";
		key = "id";
	}
};

}
}
#endif // OBJ_BILLMGR_EMPLOYEE_H
