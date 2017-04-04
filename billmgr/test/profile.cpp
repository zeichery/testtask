#include "../fixtures/fixtures.h"
#include "../objects/profile.h"

using namespace test_functions;

namespace billmgr {
TEST_F(AccountFixture, ProfileCreate) {
	object::Profile<> profile;
	profile.setSU(account->SU());
	//Проверка создания профиля
	ASSERT_NO_THROW(profile.Create()) << "Profile was not created!";
}
}
