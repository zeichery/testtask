#ifndef BILLMGR_ENVIRONMENT_H
#define BILLMGR_ENVIRONMENT_H

#include "utils.h"
#include "define.h"
#include "../objects/employee.h"
#include "../objects/project.h"
#include "../objects/department.h"
#include "../objects/currency.h"
#include "../objects/account.h"
#include "../objects/company.h"
#include "../objects/country.h"
#include "../objects/locale.h"


#define GetEnvironment() \
	(*(dynamic_cast<billmgr::Environment*>(environment)))

/**
  * Если условия не выполняются, то тест прерывается
  */
#define CheckVersion(statement) \
	if (!(statement)) { \
		return; \
	}

using namespace test_functions;

namespace billmgr {


class Version {
private:
	const int m_version;
public:
	enum Versions {
		Advanced = 2,
		Corporate = 3,
		ISPsystem = 5
	} ;

template <class MgrType>
	Version(MgrType mgr);
	Version(const int version);

	bool operator<=(int a) const { return m_version <= a; }
	bool operator<(int a) const { return m_version < a; }
	bool operator>(int a) const { return m_version > a; }
	bool operator>=(int a) const { return m_version >= a; }
	bool operator!=(int a) const { return m_version != a; }
	bool operator==(int a) const { return m_version == a; }
	inline operator int() const { return m_version; }
	inline operator string() const;

};

#define OBJ_PTR(type) \
	std::shared_ptr< type <> >

#define REMOTE_OBJ_PTR(type2, templ) \
	std::shared_ptr< type2 <templ> >

class Environment;
class EnvironmentObjects {
public:
	//объекты удаляются в обратном порядке. Поэтому порядок важен
	MgrConfig *mgrConfig;
	test_functions::dbConnect<test_mgrobject::LocalQuery> db;
	OBJ_PTR(object::Employee) employee;
	OBJ_PTR(object::Locale) locale;
	OBJ_PTR(object::Department) department;
	OBJ_PTR(object::Project) project;

	OBJ_PTR(object::Company) company;
	OBJ_PTR(object::Company) company2;
	OBJ_PTR(object::Currency) currency;//рубли
	OBJ_PTR(object::Currency) currencyeuro;//евро
	OBJ_PTR(object::Currency) currencyusd;//доллары
	OBJ_PTR(object::Country) country;//РФ

	void Init(Environment& env);
private:

};
class Environment : public ::testing::Environment {
public:
	test_mgrobject::LocalQuery localHost;
	std::shared_ptr<EnvironmentObjects> def_obj;
	std::shared_ptr<Version> version;
	Environment();

	virtual ~Environment() {}

	/**
	 * @brief	Глобальный SetUp() тестовой среды.
	 *
	 */
	void SetUp();

	/**
	 * @brief	Глобальный TearDown() тестовой среды.
	 *
	 */
	void TearDown();

private:
	std::vector<std::shared_ptr<test_mgrobject::MgrObject>> m_objects_for_delete;
	/**
	 * @brief	Выполняет создание необходимых объектов окружения.
	 *
	 */
	void InitObjects();

	/**
	 * @brief	Выполняет удаление необходимых объектов окружения.
	 *
	 */
	void DeleteObjects();
	/**
	 * @brief Проверяет что к биллингу можно подключиться
	 *
	 */
	template <class MgrType>
	void CheckMgrConnection(MgrType &mgr, const string& remoteHost = "");

	template <class MgrType>
	void DeleteLongtaskFiles(MgrType& mgr, const string& remoteHost = "");
};

}

#endif // BILLMGR_ENVIRONMENT_H
