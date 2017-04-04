#ifndef OBJ_BILLMGR_OBJ_H
#define OBJ_BILLMGR_OBJ_H
#include <mgr/mgrxml.h>
#include <mgr/mgrssh.h>
#include <mgr/mgraccess.h>
#include "../../mgr/mgrobject.h"
#include "../../coremgr/obj_core_common.h"

namespace billmgr {
namespace object {

class BILLmgrList : public test_mgrobject::MgrList {
public:
    BILLmgrList(test_mgrobject::IQuery* provider) : test_mgrobject::MgrList(provider) {}

    /**
     * @brief	Проверяет, есть ли в списке элемент с указанным id.
     * @param	[in] id элемента.
     * @return	Возвращает true, если элемент в списке присутсвует; false в противном случае.
	 *
     */
    bool HasItemWithId(const std::string& id) const {
        StringMap filter;
        filter["id"] = id;

        return HasItems(filter);
    }

};

class BILLmgrElem : public CoreObjects::CommonElem {

protected:
    bool deleteOnDestroy; ///< Определяет, вызывать ли метод Delete() в деструкторе, используется для вложенных списков
public:
	/**
	* @brief	Конструктор по умолчанию.
	*
	*/
    BILLmgrElem(test_mgrobject::IQuery* provider) : CoreObjects::CommonElem(provider) {
        props["id"] = "";
        deleteOnDestroy = true;
	}

    /**
     * @brief	Деструктор. Выполняет удаление объекта.
	 *
     */
    virtual ~BILLmgrElem() {
        if (deleteOnDestroy) DELETE_OBJECT();
    }

	/**
	* @brief	Добавить prop имя которого заранее неизвестно.
	* @param	[in] prop_name имя добавляемого prop.
	*
	*/
	void AddNewProp(const string& prop_name) {
		props[prop_name] = "";
	}

	/**
	* @brief	Получить id объекта по его внутреннему имени когда оно не передается в функцию.
	* @param	[in] intname внутреннее имя объекта.
	*
	*/
	string GetIdByIntName(const string& intname) {
		auto all_xml = XmlQuery("func=" + func + "&out=xml");
		mgr_xml::XPath all_xp(all_xml, "//elem");
		string id;
		ForEachI(all_xp, xp) {
			auto processing_xml = XmlQuery("func=" + func +".edit&out=xml&elid=" + xp->FindNode("id"));
			if(processing_xml.GetNode("/doc/intname").Str() == intname) {
				id = processing_xml.GetNode("/doc/id").Str();
				break;
			}
		}
		return id;
	}

	/**
	* @brief	Получить id объекта.
	*
	*/
	string GetId() {
		auto pms = XmlQuery("func=" + func + "&out=xml");
		props["id"] = pms.GetNode("//elem[name=" + mgr_xml::EscapePropValue(props["name"]) + "]/id").Str();
		return props["id"];
	}

	/**
	* @brief	Получить id объекта через указанное поле, следить чтобы оно было уникальным.
	* * @param	[in] value значение поля.
	* * @param	[in] field имя поля.
	*
	*/
	string GetIdByField(const string& value, const string& field) {
		auto result_xml = XmlQuery("func=" + func + "&elid=" + props["plid"]);
		props["id"] = result_xml.GetNode("//elem[" + field + "=" + mgr_xml::EscapePropValue(value) + "]/id").Str();
		return props["id"];
	}

};

}
}

#endif // OBJ_BILLMGR_OBJ_H
