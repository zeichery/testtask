#include "../fixtures/fixtures.h"
using namespace test_functions;

namespace billmgr {

/**
* @brief Редактирование аккаунта
*
*/
TEST_F(AccountFixture, AccountEdit) {
	account->Read();
	string name = account->GetProp("name");
	string valid_phone = account->GetProp("valid_phone");
	string note = account->GetProp("note");
	string registration_ip = account->GetProp("registration_ip");
	string registration_date = account->GetProp("registration_date");
	string attitude = account->GetProp("attitude");
	string label =  account->GetProp("label");
	string employee = account->GetProp("employee");
	string nocalcstat =  "";

	account->SetProp("name", GenName(6) + " (" +GenMailBoxName()+")");
	account->SetProp("valid_phone", str::url::Encode("+1 000 000 0000"));
	account->SetProp("note", GenName());
	account->SetProp("registration_ip", str::url::Encode("1.1.1.1"));
	account->SetProp("registration_date", str::url::Encode("2015-01-01 00:02:41"));
	account->SetProp("attitude", "2");
	account->SetProp("label", GenName());
	account->SetProp("employee", "null");

	ASSERT_NO_THROW(account->Update()) << "Account wasn't updated";
	account->Read();

	EXPECT_NE(name, account->GetProp("name")) << "Account realname wasn't changed";
	EXPECT_NE(valid_phone, account->GetProp("valid_phone")) << "Account valid_phone wasn't changed";
	EXPECT_NE(note, account->GetProp("note")) << "Account note wasn't changed";
	EXPECT_EQ(registration_ip, account->GetProp("registration_ip")) << "Account registration_ip was changed";
	EXPECT_NE(attitude, account->GetProp("attitude")) << "Account attitude wasn't changed";
	EXPECT_NE(label, account->GetProp("label")) << "Account label wasn't changed";
	EXPECT_NE(employee, account->GetProp("employee")) << "Account employee wasn't changed";

}

/**
* @brief Удаление аккаунта
*
*/
TEST_F(AccountFixture, AccountDelete) {
	account->Read();
	string name = account->GetProp("name");
	object::AccountList<> accountlist;

	//Проверка удаления аккаунта
	ASSERT_NO_THROW(account->Delete()) << "Account wasn't deleted!";

	//Проверка отсутствия аккаунта в списке аккаунтов
	EXPECT_FALSE(accountlist.hasElementWithNode("name", name)) << "Accounts list has account with name: " << name;
	TearDownTestCase();
	SetUpTestCase();
}


/**
* @brief Создание аккаунта, проверка соответствия параметров клиента введенным при создании
*
*/
TEST(Account, AccountCreateCheckParams) {
	auto& env = GetEnvironment();
	object::Account<> account_new;
	object::AccountList<> accountlist;
	string email = GenMailBoxName();
	string realname = GenName();
	string name = realname + " (" + email + ")"; //имя клиента составляется из введенных контактного лица и email
	string passwd = GenName(12);
	string note = GenName();
	string employee_id = "";

	account_new.SetProp("email", email);
	account_new.SetProp("realname", realname);
	account_new.SetProp("passwd", passwd);
	account_new.SetProp("note", note);
	account_new.SetProp("country", env.def_obj->country->GetProp("id"));
	//Начиная с версии Corporate при создании аккаунта вводится сотрудник и провайдер
	account_new.SetProp("project", env.def_obj->project->GetProp("id"));
	employee_id = env.def_obj->employee->GetProp("id");
	account_new.SetProp("employee", employee_id);

	//Проверка создания аккаунта
	ASSERT_NO_THROW(account_new.Create()) << "Account wasn't created!";
	account_new.Read();

	//Проверяем, что параметры клиента соответствуют введенным при создании
	EXPECT_EQ(name, account_new.GetProp("name")) << "Account name isn't the same with entered";
	EXPECT_EQ(note, account_new.GetProp("note")) << "Account note isn't the same with entered";
	EXPECT_EQ(employee_id, account_new.GetProp("employee")) << "Account employee isn't the same with entered";
}

/**
* @brief Создание аккаунта, проверка cоздания пользователя вместе с аккаунтом
*
*/
TEST_F(AccountFixture, AccountCreateCheckUser) {
	auto& env = GetEnvironment();
	account->Read();
	string email = account->GetProp("email");
	string realname = account->GetProp("realname");
	string passwd = account->GetProp("passwd");

	object::UserList<> userlist;
	//Проверяем наличие созданного вместе с аккаунтом пользователя в списке пользователей
	EXPECT_TRUE(userlist.hasElementWithNode("name", email)) << "Users list doesn't have user with name: " << email;
	EXPECT_TRUE(userlist.hasElementWithNode("email", email)) << "Users list doesn't have user with email: " << email;
	EXPECT_TRUE(userlist.hasElementWithNode("realname", realname)) << "Users list doesn't have user with realname: " << realname;
	EXPECT_TRUE(userlist.hasElementWithNode("account", account->GetProp("name"))) << "Users list doesn't have user which belongs to account: " << account->GetProp("name");

	//Проверяем возможность авторизоваться под созданным пользователем
	string projectId="";
	projectId = env.def_obj->project->GetProp("id");
	EXPECT_TRUE(test_functions::CheckAuthenticate(str::url::Encode(email), passwd, projectId)) << "User has not been authorized";
}

/**
* @brief Проверка прикрепления аккаунта к провайдеру
*
*/
TEST_F(AccountFixture, AccountCreateCheckProject) {
	account->Read();
	string project = account->GetProp("project");
	object::AccountProjectList<> account_project_list(account->GetProp("id"));
	ASSERT_TRUE(account_project_list.IsProjectEnabled(project)) << "Project is disable for created account";
}
} //end of billmgr namespace











