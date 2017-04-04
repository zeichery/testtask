#include "fixtures.h"
#include <mgr/mgrxml.h>
#include "../main/environment.h"

using namespace test_mgrobject;
using namespace test_functions;
using namespace billmgr;


/* ProjectFixture */
void ProjectFixture::SetUp() {
	auto& env = GetEnvironment();
	if (*env.version > Version::Advanced) {
		projecteuro.reset(new object::Project<>());
		projecteuro->SetProp("currency", env.def_obj->currencyeuro->GetProp("id"));
		projecteuro->SetProp("billurl", GetManagerUrl());
		projecteuro->CreateDefault();
	}
	company.reset(new object::Company<>());
	company->SetProp("country_legal",  env.def_obj->country->GetProp("id"));
	company->SetProp("country_physical",  env.def_obj->country->GetProp("id"));
	company->CreateDefault();
}

/* AccountFixture */
void AccountFixture::SetUpTestCase() {
   auto& env = GetEnvironment();
   account.reset(new object::Account<>());
   if (*env.version > Version::Advanced) {
	   account->SetProp("project", env.def_obj->project->GetProp("id"));
   }
   account->SetProp("currency", env.def_obj->currency->GetProp("id"));
   account->SetProp("country", env.def_obj->country->GetProp("id"));
   account->CreateDefault();
}
void AccountFixture::TearDownTestCase() {
	account->Delete();
	account = NULL;
}
OBJ_PTR(object::Account) AccountFixture::account;
