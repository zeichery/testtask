#ifndef OBJ_COMMON_H
#define	OBJ_COMMON_H

#include "../mgr/mgrobject.h"

namespace CoreObjects {
/**
  * @brief Обобщённый элемент, который умеет включаться и выключаться
  *
  */
class CommonElem : public test_mgrobject::MgrElem {
public:
		CommonElem(test_mgrobject::IQuery* Provider) : test_mgrobject::MgrElem(Provider) {}

		/**
		 * @brief Проверка включен елемент или выключен
		 * @return true - если включен
		 */
		virtual bool Active() const {
			string query = "func=" + func + "&filter=on&" + key + "=" + str::url::Encode(props[key]);
			if (props.find("elid")!=props.end()) query += "&elid=" + props[key];
			mgr_xml::XPath xp = QueryXpath(query, "/doc/elem[" + key + "='" + props[key] + "']/active");
			return (xp[0].Str() == "on");
		}

		/**
		  * @brief Выключение
		  */
		virtual void Suspend() const {
                        Query("func=" + func + ".suspend&elid=" + props[key] + "&sok=ok&out=xml");
		}

		/**
		  * @brief Включение
		  */
		virtual void Resume() const {
                        Query("func=" + func + ".resume&elid=" + props[key] + "&sok=ok&out=xml");
		}
};

/**
 * Обобщенный список элементов, которые умеют включаться и выключаться
 */
class CommonList : public test_mgrobject::MgrList {
private:
	string BuildQueryElids(const StringList& elidlist) {
		string elids;
		ForEachI(elidlist, iter) {
			elids += *iter + ", ";
		}
		elids.resize(elids.size() - 2);
		return elids;
	}
public:
 	CommonList(test_mgrobject::IQuery* Provider) : test_mgrobject::MgrList(Provider) {}

	/**
	* Выключение группы пользователей
	*/
	void SuspendGroup(const StringList& elidList) {
		string query = "func=" + func + ".suspend" + "&elid=" + BuildQueryElids(elidList);
		try {
			Query(query);
		} catch (const mgr_err::Error& e) {
			throw mgr_err::Error("Error", "Can't exec group suspend", e.what());
		}
	}

	/**
	* Включение группы
	*/
	void ResumeGroup(const StringList& elidList) {
		string query = "func=" + func + ".resume" + "&elid=" + BuildQueryElids(elidList);
		try {
			Query(query);
		} catch (const mgr_err::Error& e) {
				throw mgr_err::Error("Error", "Can't exec group resume", e.what());
		}
	}
};
} //CoreObjects
#endif	// OBJ_COMMON_H

