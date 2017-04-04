#include "environment.h"
#include "define.h"
#include "../../mgr/mgrobject.h"
#include "../../mgr/environment_registrator.h"
//#include <util/extutils.h>

//В методах InitObjects и DeleteObjects порядок создания и удаления объектов важен


namespace testing
{
 namespace internal
 {
  enum GTestColor {
	  COLOR_DEFAULT,
	  COLOR_RED,
	  COLOR_GREEN,
	  COLOR_YELLOW
  };

  extern void ColoredPrintf(GTestColor color, const char* fmt, ...);
 }
}
#define PRINTF(...)  do { testing::internal::ColoredPrintf(testing::internal::COLOR_GREEN, "[          ] "); testing::internal::ColoredPrintf(testing::internal::COLOR_YELLOW, __VA_ARGS__); testing::internal::ColoredPrintf(testing::internal::COLOR_YELLOW, "\n");} while(0)

namespace billmgr {
Environment::Environment(){}

void Environment::SetUp() {
	EXPECT_TRUE(test_functions::WaitForComplete(SHORT_WAITER_EXISTING, billmgr::functions::CheckProcessActiveWaiter("mysqld"))) << "MySQL server is down";
	CheckMgrConnection(localHost);
	version.reset(new Version(localHost));
	PRINTF("BILLmanager %s", version->operator string().c_str());
	try {
		PRINTF("Creating basic entities");
		InitObjects();
	} catch(...) {
		PRINTF("Error in Environment::SetUp()");
		DeleteObjects();
		throw;
	}
}


void Environment::TearDown() {
	try {
		DeleteObjects();
	} catch (...) {
		throw;
	}
}


void EnvironmentObjects::Init(Environment& env)
{
	//выключение проверки пароля
	mgrConfig = new MgrConfig (BILLMGR_CONF_FILE, true);
	employee.reset(new object::Employee<>());
	EXPECT_NO_THROW(employee->CreateDefault());
	functions::GetDefaultAdmin((*employee)["name"]);

	currency.reset(new object::Currency<>(RUSSIAN_RUBLE));
	currency->UseExisting();
	currency->Resume();

	currencyusd.reset(new object::Currency<>(USD));
	currencyusd->UseExisting();
	currencyusd->Resume();

	country.reset(new object::Country<>(RUSSIAN_COUNTRY));
	country->UseExisting();
	country->Resume();

	locale.reset(new object::Locale<>(RUSSIAN_LANGCODE));
	locale->UseExisting();
	locale->Resume();

	currencyeuro.reset(new object::Currency<>(EURO));
	currencyeuro->UseExisting();
	currencyeuro->Resume();

	department.reset(new object::Department<>());
	EXPECT_NO_THROW(department->CreateDefault());
	department->AddEmployee(employee->GetProp("id"));

	company.reset(new object::Company<>());
	company->SetProp("country_legal", country->GetProp("id"));
	company->SetProp("country_physical", country->GetProp("id"));
	company->SetProp("locale", locale->GetProp("id"));
	EXPECT_NO_THROW(company->CreateDefault());
	project.reset(new object::Project<>);
	project->SetProp("currency", currency->GetProp("id"));
	project->SetProp("billurl", GetManagerUrl(URL_HTTP + URL_IP + URL_MGR).c_str());
	EXPECT_NO_THROW(project->CreateDefault());
	company->ConnectToProject(project->GetProp("id"));
}


void Environment::InitObjects() {
	def_obj.reset(new EnvironmentObjects());
	def_obj->Init(*this);
}

void Environment::DeleteObjects() {
	def_obj.reset();
}

template <class MgrType>
void Environment::DeleteLongtaskFiles(MgrType &mgr, const string &remoteHost) {
	EXPECT_TRUE(mgr.System("rm -rf /usr/local/mgr5/var/run/*") == EXIT_SUCCESS) << "Error on delete longtask files" + remoteHost;
}

template <class MgrType>
void Environment::CheckMgrConnection(MgrType &mgr, const string& remoteHost)
{
	string mgrName = "";
	if (remoteHost.empty())
			mgrName = "billmgr";
	EXPECT_NO_THROW(mgr.Query("func=whoami")) << "No connection to " + mgrName + remoteHost;
}

class BILLMgrEnvironmentRegistrator : public EnvironmentRegistrator {
public:
	::testing::Environment* Register() {
			return ::testing::AddGlobalTestEnvironment(new billmgr::Environment);
	}
} BILLMgrEnvReg;

billmgr::Version::Version(const int version)
	: m_version	(version)
{
	if (m_version < 0)
		throw mgr_err::Value("version");
}
template <class MgrType>
billmgr::Version::Version(MgrType mgr)
	: m_version	(str::Int(mgr.Query("func=whoami").GetNode("/doc/mgr").GetProp("dist")))
{
	if (m_version < 0)
		throw mgr_err::Value("version");
}

billmgr::Version::operator string() const {
	string res;
	switch (m_version) {
	case Advanced :
		res = "Advanced";
		break;
	case Corporate :
		res = "Corporate";
		break;
	case ISPsystem :
		res = "ISPsystem";
		break;
	}

	return res;
}
}
