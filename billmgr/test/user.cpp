#include "../fixtures/fixtures.h"
using namespace test_functions;

namespace billmgr {
/**
* @brief Создание пользователя, проверка соответствия параметров пользователя введенным при создании, проверка авторизации
*
*/
TEST_F(AccountFixture, UserCreate) {
	auto& env = GetEnvironment();
	account->Read();
	object::User<> user;
	object::UserList<> userlist;
	user.setSU(account->SU());
	string email = GenMailBoxName();
	string realname = GenName();
	string passwd = GenName(12);
	string phone = "+1 000 000 0000";
	user.SetProp("email", email);
	user.SetProp("realname", realname);
	user.SetProp("passwd", passwd);
	user.SetProp("phone", str::url::Encode(phone));

	//Проверка создания пользователя
	ASSERT_NO_THROW(user.Create()) << "User wasn't created!";

	user.Read();
	//Проверяем, что параметры пользователя соответствуют введенным при создании
	EXPECT_EQ(email, user.GetProp("email")) << "User email isn't the same with entered";
	EXPECT_EQ(realname, user.GetProp("realname")) << "User realname isn't the same with entered";
	EXPECT_EQ(phone, user.GetProp("phone")) << "User phone isn't the same with entered";

	string projectId="";
	projectId = env.def_obj->project->GetProp("id");
	//Проверяем возможность авторизоваться под созданным пользователем
	EXPECT_TRUE(test_functions::CheckAuthenticate(str::url::Encode(email), passwd, projectId)) << "User has not been authorized";
}

/**
* @brief Создание пользователя с существующим email
*
*/
TEST_F(AccountFixture, UserCreateWithExistEmail) {
	account->Read();
	object::User<> user1;
	object::User<> user2;
	user1.setSU(account->SU());
	user2.setSU(account->SU());

	//Проверка создания пользователя
	ASSERT_NO_THROW(user1.CreateDefault()) << "User wasn't created!";
	user1.Read();
	string email = user1.GetProp("email");

	string realname = GenName();
	string passwd = GenName(12);
	user2.SetProp("email", email);
	user2.SetProp("realname", realname);
	user2.SetProp("passwd", passwd);

	//Проверка создания пользователя с существующим email
	EXPECT_ANY_THROW(user2.Create()) << "User with exist email was created!";
}

/**
* @brief Удаление пользователя
*
*/
TEST_F(AccountFixture, UserDelete) {
	account->Read();
	object::User<> user;
	object::UserList<> userlist;
	user.setSU(account->SU());

	//Cоздание пользователя
	ASSERT_NO_THROW(user.CreateDefault()) << "User wasn't created!";
	user.Read();
	string user_id = user.GetProp("id");

	//Проверка удаления пользователя
	ASSERT_NO_THROW(user.Delete()) << "User wasn't deleted!";

	//Проверяем отсутствие пользователя в списке пользователей
	EXPECT_FALSE(userlist.HasItemWithId(user_id)) << "Users list has user with id: " << user_id;
}


/**
* @brief Выключение/Включение пользователя
*
*/
TEST_F(AccountFixture, UserSuspendResume) {
	auto& env = GetEnvironment();
	account->Read();
	object::User<> user;
	user.setSU(account->SU());
	string email = GenMailBoxName();
	string realname = GenName();
	string passwd = GenName(12);
	user.SetProp("email", email);
	user.SetProp("realname", realname);
	user.SetProp("passwd", passwd);

	//Cоздание пользователя
	ASSERT_NO_THROW(user.Create()) << "User wasn't created!";
	user.Read();

	user.Suspend();
	string projectId="";
	projectId = env.def_obj->project->GetProp("id");
	//Проверяем невозможность авторизоваться под отключенным пользователем
	EXPECT_FALSE(test_functions::CheckAuthenticate(str::url::Encode(email), passwd, projectId)) << "User has not been authorized";

	user.Resume();
	//Проверяем возможность авторизоваться под включенным пользователем
	EXPECT_TRUE(test_functions::CheckAuthenticate(str::url::Encode(email), passwd, projectId)) << "User has not been authorized";
}

/**
* @brief Редактирование пароля, номера телефона пользователя под клиентом
*
*/
TEST_F(AccountFixture, UserEditPhonePasswd) {
	auto& env = GetEnvironment();
	account->Read();
	object::User<> user;
	user.setSU(account->SU());
	string email = GenMailBoxName();
	string realname = GenName();
	string passwd = GenName(12);
	string phone = "+1 000 000 0000";
	user.SetProp("email", email);
	user.SetProp("realname", realname);
	user.SetProp("passwd", passwd);
	user.SetProp("phone", str::url::Encode(phone));

	//Проверка создания пользователя
	ASSERT_NO_THROW(user.Create()) << "User wasn't created!";
	user.Read();

	string passwd_new = GenName(12);
	string phone_new = "+1 000 000 0001";
	user.SetProp("passwd", passwd_new);
	user.SetProp("phone", str::url::Encode(phone_new));

	//Изменение пароля, телефона пользователя
	ASSERT_NO_THROW(user.Update()) << "User wasn't updated!";
	user.Read();
	string projectId="";
	projectId = env.def_obj->project->GetProp("id");
	//Проверяем невозможность авторизоваться cо старым паролем
	EXPECT_FALSE(test_functions::CheckAuthenticate(str::url::Encode(email), passwd, projectId)) << "User has been authorized with old password";

	//Проверяем, что новое значение телефона применилось
	EXPECT_EQ(phone_new, user.GetProp("phone")) << "User phone isn't the same with entered";

	//Проверяем возможность авторизоваться c новым паролем
	EXPECT_TRUE(test_functions::CheckAuthenticate(str::url::Encode(email), passwd_new, projectId)) << "User has not been authorized with new password";
}

/**
* @brief Попытка редактирования email, ФИО пользователя под клиентом
*
*/
TEST_F(AccountFixture, UserEditEmailRealname) {
	account->Read();
	object::User<> user;
	user.setSU(account->SU());
	string email = GenMailBoxName();
	string realname = GenName();
	string passwd = GenName(12);
	user.SetProp("email", email);
	user.SetProp("realname", realname);
	user.SetProp("passwd", passwd);

	//Проверка создания пользователя
	ASSERT_NO_THROW(user.Create()) << "User wasn't created!";
	user.Read();

	string email_new = GenMailBoxName();
	string realname_new = GenName();
	user.SetProp("email", email_new);
	user.SetProp("realname", realname_new);

	ASSERT_NO_THROW(user.Update()) << "User wasn't updated!";
	user.Read();

	//Проверяем, что новые значения email, ФИО не  применились
	EXPECT_EQ(email, user.GetProp("email")) << "User email was changed";
	EXPECT_EQ(realname, user.GetProp("realname")) << "User realname was changed";
}

/**
* @brief Удаление последнего пользователя клиента
*
*/
TEST_F(AccountFixture, AccountDeleteLastUser) {
	account->Read();
	string email = account->GetProp("email");

	object::User<> user;
	string user_id = user.GetIdByField(email, "name");
	user.Open(user_id);

	//Попытка удалить последнего пользователя
	EXPECT_ANY_THROW(user.Delete()) << "Last user was deleted!";
}

/**
* @brief Выключение/Включение пользователя под администратором
*
*/
TEST_F(AccountFixture, UserSuspendResumeUnderAdmin) {
	auto& env = GetEnvironment();
	account->Read();
	string email = account->GetProp("email");
	string passwd = account->GetProp("passwd");

	object::User<> user;
	string user_id = user.GetIdByField(email, "name");
	user.Open(user_id);

	user.Suspend();
	string projectId="";
	projectId = env.def_obj->project->GetProp("id");
	//Проверяем невозможность авторизоваться под отключенным пользователем
	EXPECT_FALSE(test_functions::CheckAuthenticate(str::url::Encode(email), passwd, projectId)) << "User has not been authorized";

	user.Resume();
	//Проверяем возможность авторизоваться под включенным пользователем
	EXPECT_TRUE(test_functions::CheckAuthenticate(str::url::Encode(email), passwd, projectId)) << "User has not been authorized";
}

/**
* @brief Редактирование пользователя под администратором
*
*/
TEST_F(AccountFixture, UserEditUnderAdmin) {
	auto& env = GetEnvironment();
	account->Read();
	string email = account->GetProp("email");
	string passwd = account->GetProp("passwd");

	object::User<> user;
	string user_id = user.GetIdByField(email, "name");
	user.Open(user_id);
	string name = user.GetProp("name");

	string email_new = GenMailBoxName();
	string name_new = GenName();
	string realname_new = GenName();
	string phone_new = "+1 000 000 0000";
	string passwd_new = GenName(12);
	user.SetProp("email", email_new);
	user.SetProp("name", name_new);
	user.SetProp("realname", realname_new);
	user.SetProp("phone", str::url::Encode(phone_new));
	user.SetProp("passwd", passwd_new);

	//Редактирование пользователя
	ASSERT_NO_THROW(user.Update()) << "User wasn't updated!";
	user.Read();

	//Проверяем, что новые значения применились
	EXPECT_EQ(email_new, user.GetProp("email")) << "User email isn't the same with entered";
	EXPECT_EQ(name_new, user.GetProp("name")) << "User name isn't the same with entered";
	EXPECT_EQ(realname_new, user.GetProp("realname")) << "User realname isn't the same with entered";
	EXPECT_EQ(phone_new, user.GetProp("phone")) << "User phone isn't the same with entered";

	string projectId="";
	projectId = env.def_obj->project->GetProp("id");
	//Проверяем возможность авторизоваться c новыми именем пользователя и паролем
	EXPECT_TRUE(test_functions::CheckAuthenticate(str::url::Encode(name_new), passwd_new, projectId)) << "User has not been authorized with new password and name";

	//Проверяем невозможность авторизоваться cо старым именем пользователя и паролем
	EXPECT_FALSE(test_functions::CheckAuthenticate(str::url::Encode(name), passwd, projectId)) << "User has  been authorized with old password and name";

	//Возвращаются старые значения, чтобы не влиять на другие тесты
	user.SetProp("email", email);
	user.SetProp("passwd", passwd);
	user.SetProp("name", name);
	user.Update();
}

} //end of billmgr namespace



