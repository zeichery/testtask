#ifndef LOCALE_H
#define LOCALE_H
#include "../../mgr/mgrobject.h"
#include "obj.h"

namespace billmgr {
namespace object {

/**
* @brief Класс для работы со списком локалей
* 
*/
template <typename Provider = DefaultAdminQuery>
class LocaleList : public object::BILLmgrList
{
public:
	LocaleList() : object::BILLmgrList(new Provider) {
		func = "locale";
		key = "id";
	}
};

/**
* @brief Класс для работы с локалями BILLmanager
* 
*/
template <typename Provider = DefaultAdminQuery>
class Locale : public BILLmgrElem
{
private:
	const string m_langcode;
public:
	Locale(const string& langcode)
		: BILLmgrElem	(new Provider)
		, m_langcode			(langcode)
	{
		props["langcode"] = m_langcode;

		key = "id";
		props["sok"] = "ok";
		func = "locale";
		deleteOnDestroy = false;
	}

	void UseExisting() {
		Open(GetIdByField(m_langcode, "langcode"));
	}
};

}
}


#endif // LOCALE_H

