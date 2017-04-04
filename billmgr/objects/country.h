#ifndef OBJ_BILLMGR_COUNTRY_H
#define OBJ_BILLMGR_COUNTRY_H
#include "../../mgr/mgrobject.h"
#include "obj.h"

namespace billmgr {
namespace object {


/**
* @brief Класс для работы со страной BILLmanager
* 
*/
template <typename Provider =  DefaultAdminQuery>
class Country : public BILLmgrElem
{
private:
	const string m_iso;
public:
	Country(const string& iso)
		: BILLmgrElem(new Provider)
		, m_iso			(iso)
	{
		props["id"] = "";
		props["iso2"] = m_iso;
		props["name"] = "";
		props["phonecode"] = "";

		key = "id";
		props["sok"] = "ok";
		func = "country";
		deleteOnDestroy = false;
	}

	~Country() {
		DELETE_OBJECT();
	}

	void UseExisting() {
		SetProp("id", GetIdByField(m_iso, "iso2"));
	}
};

/**
* @brief Класс для работы со списком стран
* 
*/
template <typename Provider =  DefaultAdminQuery>
class CountryList : public test_mgrobject::MgrList
{
public:
	CountryList() : test_mgrobject::MgrList(new Provider) {
		func = "country";
		key = "id";
	}

	/**
	 * @brief	Получение id страны по ISO коду
	 * @param	[iso] iso страны
	 * @return	Возвращает id страны
	 *
	 */
	string GetIdByIso(const string &iso){
		StringList outList, fields;
		StringMap filter;
		fields.push_back("id");
		filter["iso2"]=iso;
		GetList(outList, fields, filter);
		return outList.front();
	}
};

/**
* @brief Класс для работы со списком регионов стран
* 
*/
template <typename Provider =  DefaultAdminQuery>
class CountryRegionsList : public test_mgrobject::MgrList
{
private:
public:
	CountryRegionsList(const string &elid) : test_mgrobject::MgrList(new Provider) {
		func = "country.state";
		props["elid"]=elid;
		key = "id";
	}

	/**
	 * @brief	Получение id региона по названию
	 * @param	[name] название региона
	 * @return	Возвращает id региона
	 *
	 */
	string GetIdByName(const string &name){
		StringList outList, fields;
		StringMap filter;
		fields.push_back("id");
		filter["name"]=name;
		GetList(outList, fields, filter);
		return outList.front();
	}


};

}
}

#endif // OBJ_BILLMGR_COUNTRY_H
