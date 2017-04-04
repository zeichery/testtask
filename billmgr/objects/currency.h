#ifndef OBJ_BILLMGR_CURRENCY_H
#define OBJ_BILLMGR_CURRENCY_H
#include "../../mgr/mgrobject.h"
#include "obj.h"

namespace billmgr {
namespace object {
/**
* @brief Класс для работы с валютой BILLmanager
* 
*/
template <typename Provider = DefaultAdminQuery>
class Currency : public BILLmgrElem
{
private:
	const string m_iso;
public:
	Currency(const string& iso)
		: BILLmgrElem	(new Provider)
		, m_iso			(iso)
	{
		props["iso"] = m_iso;
		key = "id";
		props["sok"] = "ok";
		func = "currency";
	}

	~Currency() {
		DELETE_OBJECT();
	}

	void UseExisting() {
		SetProp("id", GetIdByField(m_iso, "iso"));
		SetProp("symbol", ""); //убираем символ, чтобы использовалось iso в качестве обозначения валюты
		Update();
	}


	/**
	* @brief Обновить курсы валют для валют с автоматическим обновлением
	* @param [id] Id валюты
	* 
	*/
	void UpdateCurrencyRate() {
		Query("clicked_button=ok&func=currency.upload&sok=ok");
	}

};
/**
* @brief Класс для работы со списком валют
* 
*/
template <typename Provider = DefaultAdminQuery>
class CurrencyList : public object::BILLmgrList
{
public:
    CurrencyList() : object::BILLmgrList(new Provider) {
		func = "currency";
		key = "id";
	}
};


/**
* @brief Класс для работы со списком курсов валюты
* 
*/
template <typename Provider = DefaultAdminQuery>
class CurrencyRateList : public object::BILLmgrList
{
public:
	CurrencyRateList(const string &elid) : object::BILLmgrList(new Provider) {
		func = "currency.relate";
		props["elid"] = elid;
		key = "id";
	}

	/**
	* @brief Получение текущего курса валюты
	* @param [currencyId] id курса валюты
	* 
	*/
	string GetCurrentCurrencyRate(const string& currencyId){
		StringList outList,fields;
		fields.push_back("rate");
		StringMap filter;
		filter["id"] = currencyId;
		GetList(outList, fields, filter);
		string curRate;
		ForEachI(outList, item){
			curRate=*item;
		}
		return curRate;
	}
};

/**
* @brief Класс для работы со списком динамики курса валюты
* 
*/
template <typename Provider = DefaultAdminQuery>
class CurrencyRateDynamicList : public object::BILLmgrList
{
public:
	CurrencyRateDynamicList(const string &elid,const string &plid) : object::BILLmgrList(new Provider) {
		func = "currencyrate";
		props["elid"] = elid;
		props["plid"] = plid;
		key = "id";
	}

	/**
	* @brief Обновить курсы валют
	* 
	*/
	void UpdateCurrencyRateFromDynamic(){
		bool empty_rates = true;
		int try_count = 10;
		while (empty_rates && try_count) {
			Query("func=" + func + ".upload&plid="+ props["plid"] + "/" + props["elid"] + "&upload=on&clicked_button=ok&sok=ok");
			--try_count;

			StringList outList,fields;
			fields.push_back("rate");
			StringMap filter;
			GetList(outList, fields, filter);

			empty_rates = outList.empty();

			if (empty_rates) mgr_proc::Sleep(1250);
		}
	}

	/**
	* @brief Получение курса валюты за определенную дату
	* @param [dateCurrencyRate] дата для курса валюты
	* 
	*/
	string GetCurrencyRateByDate(const string& dateCurrencyRate){
		StringList outList,fields;
		fields.push_back("rate");
		StringMap filter;
		filter["ratedate"] = dateCurrencyRate;
		GetList(outList, fields, filter);
		string curRate;
		ForEachI(outList, item){
			if (!item->empty())
				curRate=*item;
		}
		return curRate;
	}
};
}
}
#endif // OBJ_BILLMGR_CURRENCY_H
