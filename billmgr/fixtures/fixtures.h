#ifndef BILLMGR_FIXTURES_H
#define BILLMGR_FIXTURES_H

#include "../main/define.h"
#include "../main/environment.h"
#include "../main/utils.h"
#include "../objects/account.h"
#include "../objects/company.h"
#include "../objects/country.h"
#include "../objects/currency.h"
#include "../objects/department.h"
#include "../objects/employee.h"
#include "../objects/project.h"
#include "../objects/user.h"
#include "../../mgr/mgrobject.h"
#include <mgr/mgrstr.h>
#include <mgr/mgraccess.h>
#include <mgr/mgrxml.h>

using namespace test_functions;
using namespace billmgr;

/**
* @brief Фикстура создаёт проект
*/
class ProjectFixture : public virtual testing::Test {
protected:
	OBJ_PTR(object::Project) projecteuro;
	OBJ_PTR(object::Company) company;
	void SetUp();
};


/**
* @brief Фикстура создаёт клиента
*
*/
class AccountFixture: public virtual ::testing::Test, public ::testing::WithParamInterface<StringMap> {
public:
	static OBJ_PTR(object::Account) account;
	static void SetUpTestCase();
	static void TearDownTestCase();
};

#endif // BILLMGR_FIXTURES_H
