#ifndef TEMPLATE_H
#define TEMPLATE_H

#include <list>
#include <iostream>

#include <mgr/mgrclient.h>
#include <mgr/mgrxml.h>
#include <mgr/mgrstr.h>
#include <mgr/mgrproc.h>
#include <mgr/mgrerr.h>
#include <mgr/mgrnet.h>
#include <mgr/mgrssh.h>
#include <mgr/mgrenv.h>
#include <mgr/mgrthread.h>

#include "isptest.h"

#define DELETE_OBJECT() \
	try {\
		Delete();\
	} catch (const mgr_err::Error& e) {\
	    ADD_FAILURE() << "There is an error while removing object : " << e.what();\
	} catch(...) {}

namespace test_mgrobject {
typedef std::map<std::string, std::string>::iterator mapIterator;

/**
  * Абстрактный класс запросов к манагеру
  */
class IQuery {
public:
	virtual mgr_xml::Xml Query(const string& query) = 0;
	virtual int System(const string& cmd) const = 0;
	virtual int SystemIn(const string& cmd, const string& in) const = 0;
	virtual int SystemOut(const string& cmd, string& out) const = 0;
	virtual int SystemInOut(const string& cmd, const string& in, string& out) const = 0;
	virtual ~IQuery() {}
};

/**
  * Класс запросов к манагеру
  * Подразумевается что тесты запускаются на той же машине, где установлен манагер
  */
class LocalQuery: public IQuery {
public:
	virtual mgr_xml::Xml Query(const string& query) {
		return mgr_client::Local(MGR, "coretest").Query(QueryHook(query)).xml;
	}
	/**
	  * @brief метод вызывающий указанную команду локально
	  * @param [in] cmd - ссылка на константную строку, содержащую команду
	  * @return код завершения выполненной команды
	  */
	int System(const string& cmd) const {
		mgr_proc::Execute command(cmd);
		return command.Result();
	}
	/**
	  * @brief метод вызывающий указанную команду локально, с определённым stdin
	  * @param [in] cmd - ссылка на константную строку, содержащую команду
	  * @param [in] in - ссылка на константную строку, содержащую stdin, который будет передан команде
	  * @return код завершения выполненной команды
	  */
	int SystemIn(const string& cmd, const string& in) const {
		mgr_proc::Execute command(cmd);
		command << in;
		return command.Result();
	}
	/**
	  * @brief метод вызывающий указанную команду локально и stdout
	  * @param [in] cmd - ссылка на константную строку, содержащую команду
	  * @param [out] in - ссылка строку, содержащую stdout команды
	  * @return код завершения выполненной команды
	  */
	int SystemOut(const string& cmd, string& out) const {
		mgr_proc::Execute command(cmd);
		out = command.Str();
		return command.Result();
	}
	/**
	  * @brief метод вызывающий указанную команду локально и возвращающий stderr
	  * @param [in] cmd - ссылка на константную строку, содержащую команду
	  * @param [out] err - ссылка строку, содержащую stderr команды
	  * @return код завершения выполненной команды
	  */
	int SystemErr(const string& cmd, string& err) const {
		mgr_proc::Execute command(cmd, mgr_proc::Execute::efErr);
		err = command.Str();
		return command.Result();
	}
	/**
	  * @brief метод вызывающий указанную команду локально и возвращающий stdout в случае успеха, stderr - в обратном
	  * @param [in] cmd - ссылка на константную строку, содержащую команду
	  * @param [out] out - ссылка строку, содержащую stdout или stderr команды
	  * @return код завершения выполненной команды
	  */
	int SystemOutErr(const string& cmd, string& out) const {
		mgr_proc::Execute command(cmd, mgr_proc::Execute::efOutErr);
		out = command.Str();
		return command.Result();
	}
	/**
	  * @brief метод вызывающий указанную команду локально и stdout
	  * @param [in] cmd - ссылка на константную строку, содержащую команду
	  * @param [in] in - ссылка на константную строку, содержащую stdin, который будет передан команде
	  * @param [out] in - ссылка строку, содержащую stdout команды
	  * @return код завершения выполненной команды
	  */
	int SystemInOut(const string& cmd, const string& in, string& out) const {
		mgr_proc::Execute command(cmd, mgr_proc::Execute::efIn | mgr_proc::Execute::efOut);
		command << in;
		out = command.Str();
		return command.Result();
	}

	/**
	  * @brief метод, вызывающий перезапуск панели
	  *   e.zhigunova@ispsystem.com
	  */
	void Restart() {
		mgr_client::Local(MGR, "coretest").Restart();
	}

private:
	static int& QueryCount() {
		static int counter = 0;
		return counter;
	}

	string QueryHook(const string& query) {
		return query + "&_test_sequence_id=" + str::Str(QueryCount()++);
	}
};

class IServer {
public:
	virtual string GetURL() const = 0;
	virtual string GetUsername() const = 0;
	virtual string GetPassword() const = 0;
};
/**
*   rick@ispsystem.com
* @brief Класс для описания серверов, с которыми интегрируемся.
*/
class Server : public IServer {
public:
	Server(const string& username, const string& password, const string& mgr = "core", const string& host = "", const string& ip = "" , const string& root_passwd = "", const mgr_env::OsTypeEnum& os_type = mgr_env::osUnknown);
	virtual ~Server() {}

	virtual string GetURL() const;
	virtual string GetIP() const;
	virtual string GetUsername() const;
	virtual void SetUsrName(const string & name);
	virtual string GetPassword() const;
	virtual void SetPassWord(const string & pass);
	virtual string GetRootPasswd() const;
	virtual mgr_env::OsTypeEnum GetOSType() const;
	virtual void AddAuthorizedKey(const std::string &key = "") const;
	virtual void RemoveAuthorizedKey(const std::string& key = "") const;
	virtual string GetPublicKey(const std::string& path = "") const;

protected:
	string m_Username;
	string m_Password;
	string m_Manager;
	string m_Host;
	string m_IP;
	string m_RootPasswd;
	mgr_env::OsTypeEnum m_os_type;
};

//Server - должен быть потомком класса IServer
template<typename Server>
class RemoteQuery : public IQuery {
public:
	RemoteQuery(const Server *server = nullptr) {
		if(server != nullptr)
			m_server = *server;
	}

	virtual mgr_xml::Xml Query(const string& query) {
		string internal_query = query;
		QueryHook(internal_query);
		try {
			//mgr_proc::Sleep( 1000 ); //ждём секунду из-за древнего косяка в openssl, который они не хотят править
			return mgr_client::Remote(m_server.GetURL()).Query(internal_query).xml;
		} catch (mgr_err::Error& e) {
			auto node = e.xml().GetNode("/doc/error");
			string type = node.GetProp("type");
			string object = node.GetProp("object");
			string value;
			if (auto tmp = node.FindNode("value"))
				value = tmp.Str();

			mgr_err::Error err(type, object, value);
			node = node.FirstChild();
			while (node) {
				if (node.Name() != "msg"
						&& node.GetProp("name") != "object"
						&& node.GetProp("name") != "value") {
					if (node.GetProp("type") == "msg")
						err.add_message(node.GetProp("name"), node.Str());
					else
						err.add_param(node.GetProp("name"), node.Str());
				}
				node = node.Next();
			}
			throw err;
		}
	}
	/**
	  * @brief метод вызывающий указанную команду на удаленном сервере
	  * @param [in] cmd - ссылка на константную строку, содержащую команду
	  * @return код завершения выполненной команды
	  */
	virtual int System(const string& cmd) const {
#ifndef WIN32
		int result = 127;//not found;
		mgr_rpc::SSHpass ssh(m_server.GetIP(), "root", m_server.GetRootPasswd());
		ssh.Connect();
		ssh.Execute(cmd);
		result = ssh.Result();
		ssh.Disconnect();
		return result;
#else
		return 0;
#endif
	}
	/**
	  * @brief метод вызывающий указанную команду на удаленном сервере, с определённым stdin
	  * @param [in] cmd - ссылка на константную строку, содержащую команду
	  * @param [in] in - ссылка на константную строку, содержащую stdin, который будет передан команде
	  * @return код завершения выполненной команды
	  */
	virtual int SystemIn(const string& cmd, const string& in) const {
#ifndef WIN32
		int result = 127;//not found;
		mgr_rpc::SSHpass ssh(m_server.GetIP(), "root", m_server.GetRootPasswd());
		ssh.Connect();
		ssh.Execute(cmd);
		ssh << in;
		result = ssh.Result();
		ssh.Disconnect();
		return result;
#else
		return 0;
#endif
	}
	/**
	  * @brief метод вызывающий указанную команду на удаленном сервере и stdout
	  * @param [in] cmd - ссылка на константную строку, содержащую команду
	  * @param [out] in - ссылка строку, содержащую stdout команды
	  * @return код завершения выполненной команды
	  */
	virtual int SystemOut(const string& cmd, string& out) const {
#ifndef WIN32
		int result = 127;//not found;
		mgr_rpc::SSHpass ssh(m_server.GetIP(), "root", m_server.GetRootPasswd());
		ssh.Connect();
		ssh.Execute(cmd);
		out = ssh.Str();
		result = ssh.Result();
		ssh.Disconnect();
		return result;
#else
		return 0;
#endif
	}
	virtual int SystemInOut(const string& cmd, const string& in, string& out) const {
		return 0;
	}

	/**
	  * @brief метод, вызывающий перезапуск панели
	  *   e.zhigunova@ispsystem.com
	  */
	void Restart() {
		mgr_client::Remote mgr(m_server.GetURL());
		mgr.AddParam("authinfo", m_server.GetUsername() + ":" + m_server.GetPassword());
		mgr.Restart();
	}
private:
	Server m_server;

	void QueryHook(string& query) {
		//В случае вызова функции авторизации не используем authinfo.
		str::inpl::Trim(query);
		if ((query + "&").find("func=auth&") == string::npos)
			query += "&authinfo=" + str::url::Encode(m_server.GetUsername() + ":" + m_server.GetPassword());
	}
};

class AutoWizard;

class MgrObject {
public:
	static mgr_thread::SafeLock m_called_functions_lock;
	static mgr_thread::SafeLock m_all_functions_lock;
	static mgr_thread::SafeLock m_skip_functions_lock;

	static StringSet m_called_functions;
	static StringSet m_all_functions;
	static StringSet m_skip_functions;

	static StringSet& CalledFunctions(); ///< Множество функций вызванных при тестировании
	static StringSet& AllFunctions(); ///< Множество всех возможных функций
	static StringSet& SkipFunctions(); ///< Множество функций, которые не нужно учитывать при подсчете покрытия
private:
	std::shared_ptr<IQuery> provider_; ///< Через него ходят запросы к манагеру

	/**
	 * @brief Метод обертка для вызова provider_->Query, вызывается из методов Query() и XmlQuery()
	 * @param [in] query Запрос к объекту. Например "func=user.edit&name=newname"
	 *   p.yurin@ispsystem.net
	 */
protected:
	mgr_xml::Xml callQuery(const string& query) const;
	bool create; ///< используется для проверки на повторное создание объекта.
	///< Согласно принятым правилам, каждый объект может быть использован
	///< только 1 раз во время работы теста и представляет собой сущности в манагере.
	mutable std::map<std::string, std::string> props;	///< map of object properties, used to build query for Manager
	std::string func; ///< special object function, i.e. "user","reseller","backup"
	std::string key; ///< Имя элемента в котором хранится уникальный ключ объекта
	std::string nukey; ///< not unique object key, i.e name for backup. For user = key
	std::string su; ///< содержит имя пользователя из-под которого будут выполняться операции с объектом.
	bool forceSu; ///< в 4 манагере использовалось для принуждения к проваливанию под указанного пользователя,
	///< т.к. в некоторых случаях su был неэффективен (выполнение действий и-под администратора не root'а, например).

	/**
	 * @brief используется для генерации запроса на основе существующих пропсов в методах Update() и Create()
	 */
	virtual string BuildQuery();

	/**
	 * @brief Метод обертка для вызова provider_->Query
	 * @param [in] query Запрос к объекту. Например "func=user.edit&name=newname"
	 */
	void Query(const string& query) const;

	/**
	 * @brief Метод обертка для вызова provider_->XmlQuery
	 * @param [in] query Запрос к объекту. Например "func=user.edit&name=newname"
	 */
	mgr_xml::Xml XmlQuery(const string& query) const;
	/**
	 * @brief Обработка строки запроса перед отправкой в манагер.
	 * Предполагается, что это центральная точка, через которую проходят все запросы.
	 */
	string QueryHook(const string& query) const;

	/**
	 * @brief Конструктор MgrObject. Установка provider_
	 */
	MgrObject(IQuery* provider);

	/**
	 * @brief Выполнение XPath над результатом запроса
	 * @param [in] query Запрос к объекту. Например "func=user"
	 * @param [in] xpath_expr XPath-запрос. Например "//elem/name"
	 * @return Возвращает объект Xpath
	 */
	mgr_xml::XPath QueryXpath(const string& query, const string& xpath_expr) const;

	/**
	 * @brief Кастомизируемая часть для чтения объекта. Вызывается из Read()
	 * Выполняет считывание свойств объекта и занесение их в props
	 * Абстрактный
	 */
	virtual void ReadBody() = 0;
public:
	/**
	 * @brief Конструктор MgrObject. Создание дефолтного provider_ = LocalQuery
	 */
	MgrObject();

	/**
	 * @brief Устанавливает пользователя из под которого будут выполняться запросы. По-умолчанию это root.
	 */
	void setSU(const string& su, bool forceSu = false);

	/**
	 * @brief Получить текущего пользователя, из под которого выполнять запросы
	 */
	string getSU();

	/**
	 * @brief Деструктор. Удаляет всякое, в частности provider_
	 */
	virtual ~MgrObject();

	/**
	 * @brief Прочитать объект
	 */
	void Read();

	/**
	 * @brief Возвращает ссылку на props[field], что позволяет его поменять.
	 * Если ключа field в props, то эксепшн
	 */
	virtual string& operator[](const string& field);

	/**
	 * @brief Возвращает ссылку на props[field], но менять нельзя.
	 * Если ключа field в props, то эксепшн
	 */
	virtual const string& operator[](const string& field) const;
	/**
	 * @brief HasProp
	 * @param field
	 * @return
	 */
	virtual bool HasProp(const string& field);
	/**
	 * @brief Возвращает ссылку на props[field], что позволяет его поменять.
	 * Если ключа field в props, то эксепшн
	 */
	virtual string& GetProp(const string& field);

	/**
	 * @brief Возвращает ссылку на props[field], но менять нельзя.
	 * Если ключа field в props, то эксепшн
	 */
	virtual const string& GetProp(const string& field) const;

	/**
	 * @brief Выставляет значение для props[field]. Никаких эксепшенов.
	 * @param field
	 * @param value
	 */
	virtual void SetProp(const string& field, const string& value);


	/**
	 * @brief Выставляет значение для create в true.
	 */
	virtual void SetCreated();
	/**
	 * @brief Выставляет значение для create в false.
	 * 
	 */
	virtual void SetNotCreated();

	/**
	 * @brief Получить имя функции (модуля)
	 */
	string getFuncName() const {
		return func;
	}
	friend class RequestByWizard;
	friend class AutoWizard;


	/**
	 * @brief	Дебажный метод. Вывподит значение всех пропов
	 */
	void PrintProps() const {
		std::cout << "--- " << props["name"] << " ---" << std::endl;
		ForEachI(props, prp) std::cout << prp->first << " = " << prp->second << std::endl;
	}

};

class MgrForm : public MgrObject {
public:
	MgrForm(IQuery* provider) : MgrObject(provider) {}

	/**
	 * @brief Обновить объект
	 */
	void Update();

	/**
	 * @brief Возвращает имена полей для формы
	 * @param [in] id Идентификатор элемента
	 * @param [in] lang Язык для которого нужно получить названия полей
	 * @param [out] names Возвращаемый вектор имен полей
	 */
	void getFieldsNames(const string& id, const string& lang, StringVector& names) const;

	virtual void getSelectFieldValues( const string& select_field, StringMap& ValueMap, const string& lang = "en" ) const {
		getSelectValsViaQuery( select_field, "func=" + func + "&lang=" + lang, ValueMap );
	}
protected:
	/**
	 * @brief Вызывается из Read(). Выполняет считывание свойств формы и занесение их в props
	 */
	void ReadBody();
	/**
	 * @brief Кастомизируемая часть для обновления объекта. Вызывается из Update()
	 * выполняет обновление объекта на основании значений мапы props.
	 */
	virtual void UpdateBody();

	/**
	 * @brief Выставляет значения для существующих в мапе props ключей, и не выставляет для указанных в списке на пропуск
	 */
	virtual void FillProps(mgr_xml::Xml ObjXml);

	void getSelectValsViaQuery( const string& select_field, const string& query, StringMap& ValueMap ) const;
};

class MgrElem : public MgrForm {
protected:
	/**
	 * @brief Проверяет был ли объект создан ранее и выставляет флаг create.
	 * Необходимо использовать в классах, где перегружается метод Create() и не используется родительский
	 */
	inline void CheckCreate();

	/**
	 * @brief Cоздание объекта, вызывается из Create.
	 * @param [in] name Строка с именем. Если пустое, то сгенерировать
	 * @param [in] suffix Зарезервировано для будущего
	 */
	virtual mgr_xml::Xml CreateBody(const string& name = "", bool suffix = false);

	/**
	 * @brief Cоздание объекта, вызывается из Create.
	 * @param [in] name Строка с именем. Если пустое, то сгенерировать
	 * @param [in] suffix Зарезервировано для будущего
	 * @param [in] createfunc Имя функции полностью, если функция создания отлична от edit
	 */
	virtual mgr_xml::Xml CreateBody(const string& createfunc, const string& name = "", bool suffix = false);

	/**
	 * @brief Кастомизируемая часть для удаления объекта. Вызывается из Delete()
	 */
	virtual void DeleteBody();

	/**
	 * @brief Добавляется "func=" + func + ".edit"
	 */
	virtual void ReadBody();

	/**
	 * @brief Добавляется "func=" + func + ".edit"
	 */
	virtual void UpdateBody();
public:
	MgrElem(IQuery* provider) : MgrForm(provider) {}

	/**
	  * @brief Сравнить пропсы двух объектов
	  */
	bool operator!= (const MgrElem& toCompare);

	/**
	 * @brief Возвращает строку со значениями свойств объекта вида:
	 * ============ user ============
	 * [name]="blablabla"
	 * ==============================
	 */
	virtual string ToString();

	/**
	 * @brief Cоздание объекта
	 * @param [in] name Строка с именем. Если пусто и gen_name - true, то сгенерировать
	 * @param [in] suffix Если true, то к концу имени сгенерирует суффикс
	 * @param [in] gen_name Если true и name - пусто, то сгенерирует имя
	 */
	mgr_xml::Xml Create(const string& name = "", bool suffix = false, bool gen_name = true, const string& funclist="", const string& createfunc="");

	/**
	 * @brief Прочесть ранее созданный объект
	 * @param [in] key_value - Строка с идентификатором объекта
	 */
	void Open(const string& key_value);
	/**
	 * @brief Удалить объект
	 */
	void Delete();

	/**
	 * @brief Существует ли объект
	 */
	virtual bool Exists();

	/**
	 * @brief Возвращает значение ключевого поля элемента
	 */
	virtual string getName() const;

	/**
	 * @brief	Возвращает значение props["plid"], если последний присуствует в props.
	 * @details	Должна быть перегружена в потомке, если в том классе значение plid получается особым способом.
	 * @return	Строка со значением plid элемента, если есть, иначе пустуая строка.
	 */
	virtual string getPlid() const {
		return (props.count("plid") ? props["plid"] : "");
	}

	virtual string getPlidStr() const {
		auto iter = props.find("plid");
		if (iter != props.end())
			return "&plid=" + iter->second;
		else
			return "";
	}

	virtual void getSelectFieldValues( const string& select_field, StringMap& ValueMap, const string& lang = "en" ) const {
		getSelectValsViaQuery( select_field, "func=" + func + ".edit&lang=" + lang + getPlidStr() + "&elid=" + props[key], ValueMap );
	}

	/**
	 * @brief	Выполняет чтение объекта с вызовом SetValue.
	 * @param [in] value_name Имя поля, для которое выполняется SetValue.
	 */
	void ReadSetValue( const string& value_name );
};

void MgrElem::CheckCreate() {
	if (create)
		throw mgr_err::Error("MgrElem", "The object has already been created [" + func + "] with key=" + props[key]);
//Закомментила Зоя
	//	else
//		create = true;
}

class MgrList : public MgrObject {
public:
	MgrList(IQuery* provider) : MgrObject(provider) {
		props["p_cnt"] = "";
	}

	virtual string getPlidStr() const {
		std::string plid;
		auto iter = props.find("elid");
		if (iter != props.end())
			plid = "&elid=" + iter->second;
		iter = props.find("plid");
		if (iter != props.end())
			plid += "&plid=" + iter->second;
		return plid;
	}

	int getTotalRowCount() const {
		props["out"] = "xml";
		mgr_xml::XPath xp = ListXPath("/doc/elem");
		return xp.size();
	}

	int getVisibleRowCount() const {
		return getXPathFirstElemAsInt("//p_cnt");
	}

	int getRecordLimit() const {
		return getXPathFirstElemAsInt("//p_elems");
	}

	string getButtonView() const {
		return getXPathFirstElemAttr("//toolbar", "view");
	}

	string getLanguage() const {
		return getXPathFirstElemAttr("//doc", "lang");
	}
	/**
	 * @brief Получить уникальные идентификаторы элементов списка "func=" + func
	 * @param [out] dest Возвращаемый контейнер с идентификаторами
	 * @param [in] applyFilter - нужно ли применить фильтр
	 */
	template <typename Container>
	void List(Container& dest, bool applyFilter = false) const  {
		try {
			string query = "func=" + func + (applyFilter ? "&filter=yes" : "&filter=off") + "&out=devel" + getPlidStr();
			mgr_xml::XPath xp = QueryXpath(query, "/doc/elem/" + key);

			for (size_t i = 0; i < xp.size(); i++)
				*std::inserter(dest, dest.end()) = xp[i].Str();
		} catch (const mgr_err::Error& e) {
			throw mgr_err::Error("MgrList", "Can't get objects [" + func + "] list: \"" + e.what() + "\"");
		}
	}

	/**
	 * @brief	Читает все элементы списка в вектор объектов. ElemClass — класс
	 *       	читаемых элементов, например NetElem.
	 * @param	[out] outVecotr вектор, заполняемый читаемыми элементами.
	 * @param	[out] plid - для вложенных списков
	 *  	a.mitroshin@ispsytem.com
	 */
	template <class ElemClass>
	void ReadAll(std::vector<ElemClass>& outVector, const std::string& plid = "") const {
		StringList ids;
		GetIds(ids); // Получим список id всех элементов (надо бы переписать не на id, а на ключевое поле вообще).

		// Зарезервируем нужное количество места для элементов, чтобы при resize()
		// не звался деструктор и, как следствие, Delete()
		outVector.reserve(ids.size());

		ForEachI(ids, idp) {
			outVector.resize(outVector.size() + 1);
			if (!plid.empty()) outVector.back()["plid"] = plid;
			outVector.back().SetProp(key, *idp);
			outVector.back().Read();
		}
	}

	/**
	 * @brief	Открывает все элементы списка в вектор объектов. ElemClass — класс
	 *       	открываемых элементов, например NetElem. От ReadAll() отличается тем,
	 *       	что устанавливает элементам свойство create = true.
	 * @param	[out] outVecotr вектор, заполняемый открываемыми элементами.
	 * @param	[out] plid - для вложенных списков
	 *  	a.mitroshin@ispsytem.com
	 */
	template <class ElemClass>
	void OpenAll(std::vector<ElemClass>& outVector, const std::string& plid = "") const {
		StringList ids;
		GetIds(ids); // Получим список id всех элементов (надо бы переписать не на id, а на ключевое поле вообще).

		// Зарезервируем нужное количество места для элементов, чтобы при resize()
		// не звался деструктор и, как следствие, Delete()
		outVector.reserve(ids.size());

		ForEachI(ids, idp) {
			outVector.resize(outVector.size() + 1);
			if (!plid.empty()) outVector.back()["plid"] = plid;
			outVector.back().Open(*idp);
		}
	}

	/**
	 * @brief	Заполняет выходной контейнер значениями id элементов списка.
	 * @param	[out] outList выходной список.
	 *  	a.mitroshin@ispsytem.com
	 */
	void GetIds(StringList& outList) const {
		StringList fields;
		fields.push_back(key);

		GetList(outList, fields);
	}

	/**
	 * @brief Получить отсортированный столбец списка
	 * @param [in] column_name - имя столбца
	 * @param [in] desс - порядок сортировки. true если убывающий
	 * @param [out] sorted_column - вектор отсортированных значений столбца
         * @param [in] applyFilter - нужно ли применить фильтр
	 */
	void getSortedColumn(const string& column_name, bool desc, StringVector& sorted_column, bool applyFilter = false) const;

	/**
	 * @brief Выполнить XPath запрос для списка "func=" + func
	 * @param [in] xpath строка с xpath запросом. Например "/doc/elem/id"
	 * @param [in] applyFilter применить ли к списку фильтр
	 */
	mgr_xml::XPath ListXPath(const string& xpath, bool applyFilter = false) const;
	/**
	  * @brief посчитать количество элементов (<elemName>) в xml'ке в списке "func=" + func
	  */
	size_t getElementCount(const string& elemName) const;

	/**
	  * @brief получить значение атрибута btnParam для кнопки с именем btnName. Имеются ввиду те кнопки которые над таблицей, в правом углу.
	  */
	string getButtonParam(const string& btnName, const string& btnParam) const;

	/**
	  *
	  */
	void getButtons( std::vector<StringVector>& btnGroups, const string& lang ) const;

	/**
	  * @brief получить значение атрибута param для элемента <metadata>
	  */
	string getMetadataParam(const string& param) const;

	/**
	  * @brief отображается ли столбец в таблице (есть настройка что-б не отображался)
	  */
	bool isColumnVisible(const string& colName) const;

	/**
	  * @brief посчитать количество отображаемых столбцов в таблице
	  */
	size_t getColumnsCount() const;

	/**
	 * @brief Возвращает ID первого элемента в списке "func=" + func
	 * @param	[in] filter фильтр выборки в виде контейнера типа map<string,string>,
	 *       	содержащий пары "параметр-значение". Если map пустой, будут возвращены все элементы.
	 */
	string getFirstID(const StringMap& filter = StringMap()) const;

	/**
	 * @brief Возвращает имена столбцов
	 * @param [in] lang Язык для которого нужно получить названия столбцов
	 * @param [out] names Возвращаемый вектор имен столбцов
	 */
	void getColumnNames(const string& lang, StringVector& names) const;

	/**
	  * @brief Возвращает содержимое элемента elemName. Если их несколько то содержимое первого
	  */
	string getElementValue(const string& elemName) const;

	/**
	  * @brief содержит ли список элемент (<elem>), с подузлом с именем nodeName и содержимым nodeValue
	  */
	bool hasElementWithNode(const string& nodeName, const string& nodeValue);

	/**
	  * @brief возвращает вектор значений в столбце для конкретного элемента в списке
	  * @param [in] elem_key - константная ссылка на строку, содержащую идентификатор элемента
	  * @param [out] elem_key - константная ссылка на строку, содержащую имя столбца
	  * @return вектор значений в столбце
	  */
	StringVector getColumnValuesFor(const string& elem_key, const string& column_name) const;
	/**
	  * @brief возвращает список значений в столбце, разделённых запятой, для конкретного элемента в списке
	  * @details вызывает getColumnValuesFor
	  * @param [in] elem_key - константная ссылка на строку, содержащую идентификатор элемента
	  * @param [out] elem_key - константная ссылка на строку, содержащую имя столбца
	  * @return строка значений в столбце, разделённых запятой
	  */
	string getColumnStringValueFor(const string& elem_key, const string& column_name) const;

	/**
	 * @brief	Возвращает список объектов в виде StringList со значениями указанных свойств.
	 *       	Предполагается, что все указанные во входном параметре поля должны присутсвовать в XML-ответе.
	 *       	Если какое-то поле самозакрывающееся и не содержит значения (aka <enabled/>), для него нужно
	 *       	указать символ / после имени (напр., "enabled/"). В таком случае его значением будет TRUE
	 *       	или FALSE. Список дополнительно может быть отфильтрован (см. параметр filter).
	 * @param	[out] outList выходной список, заполняемый значениями полей (через символ табуляции).
	 * @param	[in] fields массив строк с названиями возвращаемых полей.
	 * @param	[in] filter фильтр выборки в виде контейнера типа map<string,string>,
	 *       	содержащий пары "параметр-значение". Если map пустой, будут возвращены все элементы.
	 *  	s.belous@ispsytem.com
	 */
	void GetList(StringList& outList, const StringList& fields, const StringMap& filter = StringMap()) const;

	/**
	 * @brief	Возвращает количесво записей с указанными значениями полей.
	 * @param	[in] filter контейнер типа map<string,string>, содержащий пары "параметр-значение".
	 * @return	Возвращает количество объектов, попавших в переданную выборку.
	 *  	s.belous@ispsystem.com
	 */
	int ItemsNumber(const StringMap& filter) const;

	/**
	 * @brief	Проверяет, есть ли в списке записи с указанными значениями полей.
	 * @param	[in] fieldsMap контейнер типа map<string,string>, содержащий пары "параметр-значение".
	 * @return	True, если найдена хотя бы одна запись, попадающая в выборку; false в противном случае.
	 *  	s.belous@ispsystem.com
	 */
	bool HasItems(const StringMap& fieldsMap) const;

	/**
	 * @brief Удаляет группу элементов в списке
	 * @param [in] elidList - константная ссылка на лист строк, содержащий идентификаторы элементов
	 * 
	 */
	void DeleteGroup(const StringList& elidList) const;

protected:
	/**
	  * @brief XML-документ, полученный в результате выполнения функции ReadBody()
	  */
	mgr_xml::Xml body_xml;

	/**
	  * @brief Вызывается из Read(). Читает xml представляющую список функции.
	  */
	void ReadBody();

	string getXPathFirstElemAttr(const string& xpath, const string& attr_name) const {
		props["out"] = "devel";
		mgr_xml::XPath xp = ListXPath(xpath);
		if (!xp.empty()) {
			return xp[0].GetProp(attr_name.c_str());
		}
		return "";
	}

	string getXPathFirstElemText(const string& xpath) const {
		props["out"] = "devel";
		mgr_xml::XPath xp = ListXPath(xpath);
		if (!xp.empty()) {
			return xp[0].Str();
		}
		return "";
	}

	int getXPathFirstElemAsInt(const string& xpath) const {
		string rs = getXPathFirstElemText(xpath);
		if (!rs.empty()) {
			return str::Int(rs);
		}
		return 0;
	}

private:

	/**
	 * @brief	Возвращает XPath элементов по указанному фильтру.
	 * @param	[in] filter фильтр выборки в виде контейнера типа map<string,string>, содержащий пары
	 *       	"параметр-значение". Если map пустой, будут возвращены все элементы. Если в фильтр нужно
	 *       	включить логический параметр (aka <enabled/>), необходимо добавить символ / после имени
	 *       	параметра, а в качестве значения указать TRUE или FALSE (например, "enabled/" -> "TRUE").
	 * @returns	Объект XPath с отфильтрованными результатами.
	 *  	s.belous@ispsystem.com
	 */
	mgr_xml::XPath GetFilteredItems(const StringMap& filter) const;

};

/**
 * @brief Класс для прохода по шагам визарда (в одну сторону)
 * @details Для использования необходимо наследоваться.
 *   a.struzhkin@ispsystem.com
 */
class RequestByWizard {
public:
	/**
	 * @brief Конструктор
	 * @param	[in] obj - объект, для которого будет выполнятся визард. Будет помечен, как созданный (актуально для MgrElem)
	 * @param	[in] initial_step - первый шаг (имя визарда)
	 *   a.struzhkin@ispsystem.com
	 */
	RequestByWizard( MgrObject& obj, const string& initial_step );

	/**
	 * @brief Метод выполняет шаг визарда.
	 * @return возвращает true, если ещё есть шаги, а иначе false
	 *   a.struzhkin@ispsystem.com
	 */
	bool NextStep();
	string GetCurStep() const;
	/**
	 * @brief Оператор возвращающий поле объекта
	 * @param [in] field - имя поля
	 *   a.struzhkin@ispsystem.com
	 */
	string& operator[]( const string& field );
	void setSU( const string& admin );

	/**
	 * @brief Возвращает Xml документ текущего шага
	 */
	mgr_xml::Xml& GetStepXml();
protected:
	/**
	 * @brief Возвращает ожидаемое имя следующего шага.
	 * @details Используется в NextStep для проверки релевантности шагов. Необходимо перегрузить.
	 *   a.struzhkin@ispsystem.com
	 */
	virtual string NextStepName();

	MgrObject& Object;
private:
	string CurStep;
	mgr_xml::Xml step_xml;
};

/**
 * @brief Позволяет автоматически пройти по всем шагам визарда.
 * @details В объекте должны быть заполнены поля для заполнения на шагах.
 * @details //для шагов с кнопками на списках можно задать какая кнопка была нажата. ИМЯ_ЛИСТА_button
 * @param	[in] obj - объект, для которого будет выполнятся визард. Будет помечен, как созданный (актуально для MgrElem)
 * @param	[in] wizard_name - имя визарда (можно первый шаг)
 * 
 */
class AutoWizard {
public:
	AutoWizard(test_mgrobject::MgrObject& obj, const string& wizard_name)
		: m_wizard_name						(wizard_name)
		, Object							(obj)
	{

	}
	/**
	 * @brief пройти все шаги и создать объект
	 * @param	[in] need_autofill_props - считывает значение свойств в объект, если они не заполнены в нем
	 */
	mgr_xml::Xml GoAllSteps(const bool need_autofill_props = true);
protected:
	const string m_wizard_name;
	MgrObject& Object;
};

} //namespace test_mgrobject
#endif
