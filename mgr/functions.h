#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <string>
#include <vector>
#include <list>
#include <memory>
#include <classes.h>

#include "mgr/mgrerr.h"
#include "mgr/mgrstr.h"
#include "mgr/mgrxml.h"
#include "mgr/mgrrpc.h"
#include "mgr/mgrssh.h"
#include "mgr/mgrfile.h"
#include "mgr/mgrdb.h"

#include "mgrobject.h"
#include "isptest.h"
#include <api/output.h>
#include <mgr/mgrcrypto.h>

#define CYR_VALUE "русский"
#define ENG_VALUE "english"
#define NUM_VALUE "12345"
#define SPEC_VALUE "?"
#define BIG_VALUE "errorerrorerrorerrorerrorerrorerrorerrorerrorerrorerrorerrorerrorerrorerror"\
	"errorerrorerrorerrorerrorerrorerrorerrorerrorerrorerrorerrorerrorerrorerror"\
	"errorerrorerrorerrorerrorerrorerrorerrorerrorerrorerrorerrorerrorerrorerror"\
	"errorerrorerrorerrorerrorerrorerrorerrorerrorerrorerrorerrorerrorerrorerror"\
	"errorerrorerrorerrorerrorerrorerrorerrorerrorerrorerrorerrorerrorerrorerror"\
	"errorerrorerrorerrorerrorerrorerrorerrorerrorerrorerrorerrorerrorerrorerror"\
	"errorerrorerrorerrorerrorerrorerrorerrorerrorerrorerrorerrorerrorerrorerror"
//При найтбилдах определяет домен который резолвится для тестовой машины.
//Использовать его в случае необходимости работы с доменом, который должен отвечать.
//Например для отправки почты.
#ifndef PUB_DOMAIN
	#define PUB_DOMAIN test_functions::GenName() + ".com"
#endif
#define PTR_WAIT 40
#ifdef WIN32
	#define ROOTNAME "Administrator"
#else
	#define ROOTNAME "root"
#endif

//! Все, что требуется для автоматизации тестирования.
namespace test_functions {
typedef std::vector<std::string>::iterator vsPtr;
typedef std::list<std::string>::iterator lsPtr;
// ----------------------------- 1. Различные утилиты -----------------------------

/**
  * @brief Класс для создания уникального временного файла c заданным размером
  * @details Файл создается на основе шаблона, передаваемого в конструкторе, и удаляется в деструкторе подобъекта tmp
  * @param [in] fname -имя файла, будет сгенерированно на основе fname.XXXXXX
  * @param [in] size - размер создаваемого файла в кибибайтах
  * @param [in] su - true: назначить на создаваемый файл права доступа, владельца и группу родительского каталога. false: файл будет принадлежать root/Administrator
  *   n.kinash@ispsystem.com
  */
class FileWithSize {
public:
		FileWithSize( const string& fname, size_t size, bool su = false );
		/**
		  * @brief Возвращает полное имя сгенерированного файла(напр. /var/name.1Jfgds)
		  */
		string Str() const;
private:
		mgr_file::Tmp tmp;
		size_t m_size;
};

/**
 * @brief	Функтор ожидающий выполнения указанной команды с определенным выводом
 * @param [in]  output - ожидаемый вывод команды
 * @param [in]  exec - команда, напр. netstat -nap | grep nfacctd | head -n1 | awk '{print $4}'
 *  	n.kinash@ispsystem.com
 */
class ExecuteAndOutWaiter {
public:
		ExecuteAndOutWaiter( const string& output, const string& exec );
		bool operator()();
private:
		const string m_output, m_exec;
};

/**
 * @brief	Функтор ожидающий выполнения указанной команды с определенным числом строк ее вывода
 * @param [in]  number_of_strings - число строк, которые должны быть в выводе команды
 * @param [in]  exec - команда, напр. "pgrep nfacctd"
 *  	n.kinash@ispsystem.com
 */
class ExecuteAndCountWaiter {
public:
		ExecuteAndCountWaiter( size_t number_of_strings, const string& exec );
		bool operator()();
private:
		size_t m_num;
		const string m_exec;
};

/**
  * @brief Узнать хостнейм сервера.
  * @param [in] user - пользователь под которым подключаемся по SSH
  * @param [in] password - пароль от пользователя user для этого сервера
  * @param [in] ip ip-адрес сервера, хостнейм которого нужно получить.
  * @return хостнейм
  *   n.kinash@ispsystem.com
  */
string GetHostName(const std::string& user, const std::string& password, const std::string& ip);


string GenCyrillicName( int size = 10 );

/**
  * @brief Генерация случайного ipv4 адреса.
  * @return Строковое значение ip-адреса.
  */
std::string SimpleIpGenerator();

/**
  * @brief Сгенерировать адрес сети с заданным префиксом
  * @param [in] prefix префикс сети
  * @return строка с адресом сети и префиксом(напр. 192.168.56.0/24)
  *   n.kinash@ispsystem.com
  */
std::string GenNetAddr(int prefix);
/**
  * @brief Генерация случайной последовательности латинских букв.
  * @param [in] size длина генерируемой последовательности.
  * @return Строка, состоящая из латинских букв в нижнем регистре.
  */
std::string GenName(int size = 12);
/**
  * @brief Генерирование пароля заданной длины, содержащего только разрешенные символы 'a-z', цифры '0-9', символы '!'
  * @param [in] size Требуемая длина строки
  * @return	Возвращает сгенерированную строку
 */
std::string GenPass(int size = 12);
/**
  * @brief Генерация доменного имени. Возвращает FQDN.
  */
std::string GenDomainName( bool needRoot = true );

std::string GenMailBoxName();

/**
  * @brief Выполняет попытку получить PTR-запись для IP-адреса.
  * @param [in] ptr_name ARPA-запись айпишника.
  * @return PTR-запись. Если записи нет, то вернет пустую строку.
  */
std::string GetPTRRecord(const std::string& ip, bool classless = false, const std::string& server = "localhost");

/**
  * @brief Выполняет попытку получить SOA-запись для IP-адреса.
  * @param [in] ip ip-адрес.
  * @param [in] server сервер, на котором будет производиться поиск записей
  * @return SOA-запись. Если записи нет, то вернет пустую строку.
  *   p.yurin.ispsystem.com
  */
template<class Provider>
std::string GetSOARecord(const std::string& ip, const std::string& server = "localhost") {
	Provider provider;
	std::string s = "";
	provider.SystemOut("dig @" + server + " -x " + ip + " soa", s);

	if (ip.find(":") != string::npos) {
		str::GetWord(s, "SOA ");
	} else {
		str::GetWord(s, "SOA\t");
	}

	return str::GetWord(s, "\n");
}

/**
  * @brief Выполняет попытку получить NS-записи для IP-адреса.
  * @param [in] ip ip-адрес.
  * @param [out] ns_list список NS-записей.
  * @param [in] server сервер, на котором будет производиться поиск записей
  *   p.yurin.ispsystem.com
  */
template<class Provider>
void GetNSRecords(const std::string& ip, StringVector& ns_list, const std::string& server = "localhost") {
	Provider provider;
	std::string s = "";
	provider.SystemOut("dig @" + server + " -x " + ip, s);

	std::string divider;
	if (ip.find(":") != std::string::npos) {
		divider = "NS ";
	} else {
		divider = "NS\t";
	}

	while (s.find(divider) != std::string::npos) {
		str::GetWord(s, divider);
		ns_list.push_back(str::GetWord(s, "\n"));
	}
}

/**
 * @brief	Возвращает serial сети из указанного файла зоны.
 * @return	Serial сети в целочисленном виде.
 *  	s.belous@ispsystem.com
 */
int GetSerial(const std::string& zoneFilePath);

/**
  * @brief	 Выполняет ожидание выполнения длительного процесса в течение заданного времени.
  * @details Выполняет ожидание выполнения длительного процесса в течение заданного времени.
  * @details Проверка успешного завершения осуществляется при помощи complete_func 1 раз в секунду.
  * @details Если complete_func возвращает true, работа функции может завершиться до истечения заданного количества попыток.
  * @param [in] NumberOfAttempts Максимальное количество проверок с помощью CompleteFunctor.
  * @param [in] complete_func Функтор, определяющий признак завершения процесса
  * @param (должен возвращать bool и не иметь параметров)
  * @return Возвращает true, если в течение заданного времени complete_func вернет значение true,
  * @return в противном случае - возвращает false
  *   p.yurin.ispsystem.com
  */
template <class CompleteFunctor>
bool WaitForComplete(int NumberOfAttempts, CompleteFunctor complete_func) {
	NumberOfAttempts *= 5;
	while (NumberOfAttempts) {
		if (complete_func())
			return true;
		mgr_proc::Sleep(250);
		--NumberOfAttempts;
	}
	return false;
}

template <class CompleteFunctor>
bool WaitForResult(int NumberOfAttempts, CompleteFunctor complete_func) {
	NumberOfAttempts *= 2;
	while (NumberOfAttempts > 0) {
		if (complete_func())
			return true;
		mgr_proc::Sleep(500);
		--NumberOfAttempts;
	}
	complete_func.OnBad();
	return false;
}

class RebootCheck {
public:
	void Init( const string& ip, const string& rootpass );
	bool IsRebooted() const;
private:
	std::string Ip, RootPass, FileName;
};

/**
  * @brief Функтор для проверки результата возвращаемого GetPTRRecord. Для использования с WaitForComplete
  *   a.struzhkin@ispsystem.com
  */
class PTRWaiter {
public:
	/**
	  * @brief Конструктор функтора. Входные параметры такие же как у @see GetPTRRecord
	  */
	PTRWaiter(const std::string& expect, const std::string& ip_addr, bool classless = false, const std::string& server = "localhost")
		: m_expect(expect), m_ip_addr(ip_addr), m_server(server), m_classless(classless) {}
	bool operator()() {
		return (GetPTRRecord(m_ip_addr, m_classless, m_server) == m_expect);
	}
	/**
	  * @brief Деструктор
	  * @details В деструкторе происходит финальная проверка с использованием EXPECT_EQ
	  */
	~PTRWaiter() {
		try {
			EXPECT_EQ(m_expect, GetPTRRecord(m_ip_addr, m_classless, m_server)) << "Incorrect PTR record";
		} catch (...) {}
	}
private:
	const string m_expect, m_ip_addr, m_server;
	const bool m_classless;
};

class FileChangeWaiter {
public:
	FileChangeWaiter( const string& file_path, const string& old_md5 );
	bool operator ()();
private:
	string OldMD5, FilePath;
};

class FileSaver {
public:
	FileSaver();

	FileSaver( const string& file_path, bool restore_on_destructor = true );

	void Save( const string& file_path, bool restore_on_destructor = false );

	void Restore();

	~FileSaver();
private:
	string OriginalPath, SavedFilePath;
	bool RestoreOnDestructor, Saved;
};

/**
  * @brief  Класс для работы с конфигурационным файлом
  * @param  [in] conf_path Путь к конфигурационному файлу
  * @param  [in] restore_on_destructor если tru, то в деструкторе будет вызван Restore и конфиг будет восстановлен к первоначальному виду.
  *   a.mitroshin@ispsystem.com
  */

class MgrConfig {
public:
	MgrConfig();
	MgrConfig(const std::string& conf_path, bool restore_on_destructor = true);
	void Open(const std::string& conf_path, bool restore_on_destructor = false);
	void Restore();
	std::string GetParam(const std::string& name);
	std::string GetPath(const std::string& name);
	void SetParam(const std::string& name, const std::string& val = "");
	bool FindOption(const std::string &name);
	void SetOption(const std::string &name, bool val = true);
	~MgrConfig();

private:
	FileSaver file_saver;
	std::string OriginalPath, SavedFilePath, ConfPath;
	bool RestoreOnDestructor, Saved;
	StringMap params, paths;
	StringList options;

	void ReadParams();
	void WriteParams();

};

/**
* @brief    Функор перезагружает сервер
*  	a.mitroshin@ispsystem.com
*/
#ifndef WIN32
template <typename Provider>
bool Reboot() {
	Provider provider;
	return (provider.System("reboot") == 0);
}

/**
 * @brief	Функтор проверяющий состояние сервера
 * @details	Проверят доступность по SSH.
 *  	a.mitroshin@ispsystem.com
 */
class ServerRebootWaiter {
public:
		ServerRebootWaiter(const string& ip, const string& rootpass, bool poweron);
		bool operator()();
private:
		string Ip, RootPass, NetcatCmd;
		bool PowerOn;
};
#endif

/**
  * @brief Получение параметра функции pathlist (фактически - параметра файла конфигурации менеджера)
  * @param [in] param Имя параметра
  * @return Возвращает значение параметра либо пустую строку, если параметр не найден
  */
template<class Provider>
std::string GetPathListParam(const std::string& param) {
	Provider prov;
	std::string query = "func=pathlist&out=xml";
	auto xml = prov.Query(query);
	mgr_xml::XPath xp(xml, "//" + param + "/text()");
	if (xp.empty()) {
		return "";
	} else {
		return xp[0].Str();
	}
}

/**
  * @brief Выполняет очистку журнала операций
  *   p.yurin.ispsystem.com
  */
template <class Provider>
void ClearJournal() {
	Provider prov;
	prov.System("echo \"\" > ./var/core.journal");
}

/**
  * @brief Возвращает значение параметра функции paramlist
  * @param [in] ParamName Имя параметра
  * @return string Строка, содержащая значение параметра, либо пустое значение, если параметра нет.
  *   p.yurin.ispsystem.com
  */
template <class Provider>
std::string GetParamlistValue(const std::string& ParamName, bool use_cache = false) {
	Provider prov;
	static mgr_xml::Xml cache;
	auto xml = use_cache && cache.GetRoot().FirstChild() ? cache : prov.Query("func=paramlist&out=xml");
	cache.LoadFromString(xml.Str(false));
	mgr_xml::XPath xp(xml, "//elem/" + ParamName + "/text()");
	if (xp.empty()) {
		return "";
	} else {
		return xp[0].Str();
	}
}

/**
  * @brief Выполняет запрос к манагеру.
  * @param Строка запроса, который будет выполнен.
  */
template <typename Provider>
void SendRequest(const std::string& request) {
	Provider provider;
	try {
		provider.Query(request);
	} catch (const mgr_err::Error& e) {
		throw mgr_err::Error("SendRequest", "An error appeared while sending a request: " + string(e.what()));
	}
}

/**
  * @brief Выполняет запрос к манагеру и возвращает результат в виде out=xml.
  * @param Строка запроса, который будет выполнен.
  *  	a.ermakov@ispsystem.com
  * @return xmlDoc Результат выполненного запроса
  */
template <class Provider>
mgr_xml::Xml SendRequestOutXML(const std::string& request) {
	mgr_xml::Xml xmlDoc;
	Provider provider;
	try {
		xmlDoc = provider.Query(request);
	} catch (const mgr_err::Error& e) {
		throw mgr_err::Error("SendRequestOutXML", "An error appeared while sending a request: " + string(e.what()));
	}
	return xmlDoc;
}

/**
 * @brief	Константы для сбора маски URL панели.
 *  	s.belous@ispsystem.com
 */
const short URL_HTTP 	= 1; ///< https://
const short URL_IP 		= 2; ///< IP-адрес
const short URL_PORT 	= 4; ///< Порт
const short URL_MGR 	= 8; ///< Имя панели (например, vmmgr)

/**
 * @brief	Возвращает URL панели вида https://ip-addr:port/mgr или отдельные части.
 * @param	[in] mask маска требуемых частей URL. Например, URL_IP + URL_PORT вернёт
 *       	строку вида "192.168.0.1:1500", а URL_PORT - только "1500".
 * @returns	Строка с URL панели.
 *  	s.belous@ispsystem.com
 */
std::string GetManagerUrl(short mask = 255);

/**
 * @brief	Возвращает URL панели вида https://ip-addr:port/mgr или отдельные части.
 * @param	[in] mask маска требуемых частей URL. Например, URL_IP + URL_PORT вернёт
 *       	строку вида "192.168.0.1:1500", а URL_PORT - только "1500".
 * @returns	Строка с URL панели.
 *  	a.mitroshin@ispsystem.com
 */
template <typename Provider>
std::string GetManagerUrlT(short mask = 255);

/**
 * @brief	Авторизация с указанными именем пользователя и паролем.
 * @details	Пробует произвести авторизацию извне (методами curl) с указанными
 *         	именем пользователя и паролем. В случае удачи возвращает id сессии.
 *         	Иначе возвращаемая строка будет пустой.
 * @param	[in] username имя пользователя.
 * @param	[in] password пароль пользователя.
 * @return	id сессии.
 *  	a.struzhkin@ispsystem.com
 */
std::string Authenticate(const std::string& username, const std::string& password, const std::string& project="" );
/**
 * @brief	Проверяет возможность авторизации с указанными логином и паролем.
 * @details	Пробует произвести авторизацию извне (методами curl) с указанными
 *         	именем пользователя и паролем. Определяет успешность попытки методом
 *         	парсинга HTTP-ответа.
 * @param	[in] username имя пользователя.
 * @param	[in] password пароль пользователя.
 * @returns	True, если авторизовация успешна; false в противном случае.
 *  	s.belous@ispsystem.com
 */
bool CheckAuthenticate(const std::string& username, const std::string& password, const std::string& project = "");
/**
 * @brief	Проверяет возможность авторизации с указанными именем пользователя и паролем в определенном URL.
 * @param	[in] url URL-адрес ресурса.
 * @param	[in] username имя пользователя.
 * @param	[in] passwd пароль пользователя.
 * @return	id сессии.
 *  	i.meshkov@ispsystem.com
 */
void AuthWithUrl(const std::string& url, const std::string& username, const std::string& passwd);
/**
 * @brief	Проверяет func=su
 * @param	[in] user_go_from - пользователь из-под которого пытаются войти
 * @param	[in] user_go_to - пользователь под которого пытаются войти
 * @param	[in] user_func -
 */
template <typename Provider>
bool CheckSu(const std::string& user_go_from, const std::string& user_go_to, const std::string& user_func = "user" ) {
	std::string query = "func=" + user_func + ".su&elid=" + user_go_to + "&su=" + user_go_from;
	mgr_xml::Xml respXml;
	try {
		respXml = Provider().Query(query);
	} catch (const mgr_err::Error& e) {
		return false;
	}

	mgr_xml::XPath authNode(respXml, "/doc/auth/@id");
	return authNode.size();
}

/**
 * @brief	Вход в Панель с указанным пользователем (эмитация нажатия кнопки "войти")
 * @details	Пробует произвести авторизацию извне (методами curl) с указанными
 *         	именем пользователя и паролем. Определяет успешность попытки методом
 *         	парсинга HTTP-ответа.
 * @param	[in] username имя пользователя.
 * @param	[in] parent имя пользователя из подкоторого осуществляется вход Если не заданно владелец root.
 * @returns	True, если авторизовация успешна; false в противном случае.
 *  	a.ermakov@ispsystem.com
 */
bool EnterAsUser(const std::string& username, const std::string& parent = "");

/**
 * @brief	Возвращает публичный RSA-ключ, используемый для авторизации панели на удалённых серверах.
 * @return	Строка с ключём.
 *  	s.belous@ispsystem.com
 */
std::string GetPublicKey();

/**
 * @brief	Добавляет публичный ключ панели на указанный сервер в /root/.ssh/authorized_keys.
 *       	Авторизуется с паролем root'a.
 * @param	[in] ip IP-адрес сервера
 * @param	[in] rootPassword пароль root'а
 *  	s.belous@ispsystem.com
 */
void AddPublicKeyToServer(const std::string& ip, const std::string& rootPassword);

/**
 * @brief	Класс-предикат для поиска элемента-потомка MgrObject
 *       	по заданным значениям параметров в контейнере.
 *  	s.belous@ispsystem.com
 */
class ElemPropsPredicate {
private:
	StringMap props;

public:
	ElemPropsPredicate(const StringMap& props) {
		this->props = props;
	}

	inline bool operator()(const test_mgrobject::MgrObject& elemObj) const {
		bool result = true;
		ForEachI(props, prp)
			if (elemObj[prp->first] != prp->second)
				return false;
		return result;
	}
};

/**
 * @brief	Ищет во входном контейнере элемент со значениями свойств, переданных
 *       	во входном контейнере типа map<string,string>, и возвращает итератор на этот объект.
 * @param	[in] elems входной контейнер, в котором производится поиск.
 * @param	[in] props контейнер типа map<string,string> со именами и значениями параметров.
 * @return	Итератор, указывающий на найденный объект; или elems.end(), если объект не найден.
 *  	s.belous@ispsystem.com
 */
template <class ContainerClass>
typename ContainerClass::iterator FindElem(ContainerClass& elems, const StringMap& props) {
	return std::find_if(elems.begin(), elems.end(), ElemPropsPredicate(props));
}

/**
 * @brief	Ищет во входном контейнере элемент со свойством key = val и возвращает итератор на этот объект.
 * @param	[in] elems входной контейнер, в котором производится поиск.
 * @param	[in] key имя искомого параметра.
 * @param	[in] val значение искомого параметра.
 * @return	Итератор, указывающий на найденный объект; или elems.end(), если объект не найден.
 *  	s.belous@ispsystem.com
 */
template <class ContainerClass>
typename ContainerClass::iterator FindElem(ContainerClass& elems, const std::string& key, const std::string& val) {
	StringMap props;
	props[key] = val;

	return std::find_if(elems.begin(), elems.end(), ElemPropsPredicate(props));
}

/**
 * @brief	Выполняет запрос к базе. Параметры подключения берет из конфига.
 * @param	[in] db_type тип базв данных.
 *  	a.mitroshin@ispsystem.com
 */

template <typename Provider>
class dbConnect {
private:
	mgr_db::ConnectionPtr mgrdb;
	mgr_db::ConnectionParams params;
public:
	dbConnect (const std::string& db_type = "mysql",
				   const std::string& db_host = test_functions::GetParamlistValue<Provider>("DBHost"),
				   const std::string& db_user = test_functions::GetParamlistValue<Provider>("DBUser"),
				   const std::string& db_password =  test_functions::GetParamlistValue<Provider>("DBPassword"),
				   const std::string& db = test_functions::GetParamlistValue<Provider>("DBName")) {
		params.client = MGR;
		params.type = db_type;
		params.host = db_host;
		params.user = db_user;
		params.password = db_password;
		params.db = db;
		mgrdb = mgr_db::Connect(params);
	}
	//Возвращает первое значение как строку
	std::string GetStr(const std::string& query) {
		std::string result = "";
		result = mgrdb->Query(query)->Str();
		return result;
	}
	mgr_db::ConnectionPtr GetConnection() {
		return mgrdb;
	}
};

/**
 * @brief	Возвращает true, если порт слушается на удаленном ресурсе
 *  	a.mitroshin@ispsystem.com
 */
class ScanPort {
private:
	test_mgrobject::LocalQuery provider;
	std::string query;
public:
	void Init(const std::string& ip, const std::string& port){
		query = "nc -z " + ip + " " + port;
	}
	bool IsListen() const {
		int result;
		try {
			result = provider.System(query);
		} catch(...) {
			throw mgr_err::Error("ScanPort", "nc return error");
		}
		return (result == 0);
	}
};

/**
* @brief	Функтор oжидеат, когда поднимется удаленный сервис на указанном порту
* @param	[in] ip - IP-адрес удаленного ресурса
* @param	[in] port - порт
* @return	Возвращает true, если процесс завершился; false в противном случае.
*  	a.mitroshin@ispsystem.com
*/
class PortWaiter {
private:
	const test_functions::ScanPort& checker;
public:
	PortWaiter(const test_functions::ScanPort& rebootCheck) : checker(rebootCheck) {}
	bool operator()() {
		return checker.IsListen();
	}
};

/**
* @brief	Функтор сохраняет параметры формы, при удалении объекта, параметры восстанавливаются
* @param	[in] FormType - класс, описывающий сохраняемую форму
*/
template <typename FormType>
class FormSaver {
public:
	FormSaver(const std::string& suser = ROOTNAME) {
		SavedForm.setSU( suser );
		Save();
	}
	void Save() {
		SavedForm.Read();
	}
	void Restore() {
		SavedForm.Update();
	}
	~FormSaver() {
		EXPECT_NO_THROW(Restore());
	}
private:
	FormType SavedForm;
};

/**
* @brief	Проверяет доступность прокси-сервера по url
* @return	Возвращает true, если процесс запросы через прокси проходят; false в противном случае.
*  	a.mitroshin@ispsystem.com
*/
#ifndef WIN32
template <typename Provider>
   bool TestProxy(const std::string& url) {
	   Provider provider;
	   return (provider.System("wget \"" + url + "@google.ru\"") == 0);
};
#endif

// ----------------------------- 2. Валидация параметров -----------------------------

/**
  * @brief Класс представляет значение, которое необходимо проверить в каком-либо валидаторе.
  */
class ParamToCheck {

public:
	bool flag; ///< если true, то значение должно пропускаться валидатором, в противном случае, нет.
	std::string parToSet; ///< Значение, которое используется при создании объекта
	std::string parToDel; ///< Значение, которое используется при удалении объекта (как пример, используется при создании IP-адресов диапазоном)

	ParamToCheck(const bool &_flag, const std::string &_parToSet, const std::string &_parToDel = "") {
		flag = _flag;
		parToSet = _parToSet;
		parToDel = _parToDel;
		if (parToDel.empty())
			parToDel = parToSet;
	}

	std::string getCreateName() const {
		return parToSet;
	}

	std::string getDelName() const {
		return parToDel;
	}

	bool getFlag() const {
		return flag;
	}
};

typedef std::vector<ParamToCheck>::iterator vparamPtr;

/**
 *   nikita@ispsystem.com
 *
 * @brief Завернул проверку валидации какого-либо параметра в класс.
 * Один объект может быть использован для тестирования различных параметров одной функции. Для этого, после тестирования
 * параметра, выполняем ClearSettings() и добавляем новый параметр с помощью setParam().
 *
 */
template <typename Provider>
class ParamValidation {
	std::string elid_;
	std::string func_;
	std::string user_;
	std::string param_name_;
	bool escape_;
	std::vector<std::string> additional_params;
	std::vector<ParamToCheck> param_list_;

	std::string GetAdditionalParams() {
		std::string result;

		for (vsPtr it = additional_params.begin(); it != additional_params.end(); it++) {
			result += "&" + (*it);
		}

		return result;
	}

public:

	/**
	  * @brief Задает список значений, которые будут подставляться на место проверяемого параметра.
	 */
	void SetParamList(const std::vector<ParamToCheck>& params_list) {
		param_list_ = params_list;
	}

	/**
	  * @brief Конструктор, в который передается имя проверяемой функции и список значений на проверку параметра.
	  * @param [in] func Имя функции.
	  * @param [in] params_list Вектор значений, которые идут на проверку.
	 */
	ParamValidation(const std::string& func, const std::vector<ParamToCheck>& params_list) {
		if (func.empty())
			throw mgr_err::Error("ParamValidation", "The function's name can't be empty");

		if (params_list.empty())
			throw mgr_err::Error("ParamValidation", "The list of parameters can't be empty");

		SetParamList(params_list);

		func_ = func;
		escape_ = false;
	}

	/**
	  * @brief Если установить в true, то при выполнении запроса на создание и удаление,
	  * elid или проверяемый параметр будет искейпиться. В каком это случае нужно
	  * не представляю, но может где-то пригодиться.
	  */
	void setEscape(const bool& escape) {
		escape_ = escape;
	}

	/**
	  * @brief Задает имя параметра, который будет проверяться.
	  */
	void setParam(const std::string& param) {
		if (param.empty())
			throw mgr_err::Error("ParamValidation", "The parameter can't be empty");

		param_name_ = param;
	}


	/**
	  * @brief Устанавливает необходимый нам elid.
	  *
	  * Он нам нужен, если проверяемый параметр является не ключевым, т. е.
	  * удаление происходит по другому параметру. Например, при проверке поля
	  * "name", которое является ключевым, elid указывать не нужно, так как удаление
	  * cобъекта происходит по его имени. Но если проверяемым параметром является лимит
	  * доменов, то для нормального удаления нужно поставить elid'ом поле "name".
	  * Таким образом будет создаваться пользователь с разными лимитами но одним и тем
	  * же именем и удаление будет проходить по одному и тому же имени, заданному в
	  * AddAdditionalParam.
	  */
	void setElid(const std::string& elid) {
		if (elid.empty())
			throw mgr_err::Error("ParamValidation", "Can't set the empty elid");

		elid_ = elid;
	}

	void setSU(const std::string& user) {
		user_ = user;
	}

	/**
	  * @brief Используется для установки дополнительных параметров, которые
	  * используются для корректного создания объекта.
	  */
	void AddAdditionalParam(const std::string& param) {
		if (param.empty())
			throw mgr_err::Error("ParamValidation", "It isn't possible to add an empty parameter");

		additional_params.push_back(param);
	}

	/**
	 * @brief Передавать элементы, которые не нужно тестировать, можно строкой "elem_1 elem_2 ..."
	 */
	void ExcludeElements(const std::string& elements) {
		if (elements.empty())
			throw mgr_err::Error("ParamValidation", "It isn't possible to exclude the empty list");

		std::list<std::string> elements_list;
		str::Split(elements, elements_list, " ");

		for (lsPtr i = elements_list.begin(); i != elements_list.end(); i++) {
			for (vparamPtr it = param_list_.begin(); it != param_list_.end(); it++) {
				if (it->getCreateName() == (*i)) {
					printf("Elem to erase = '%s'\n", (*i).c_str());
					param_list_.erase(it);
					break;
				}
			}
		}
	}

	/**
	  * @brief Иногда требуется изменить флаг создания объекта.
	  * @param [in] elem_name Имя изменяемого элемента.
	  * @param [in] flag_value Новое значение флага.
	  */
	void ChangeElementFlag(const std::string& elem_name, const bool& flag_value) {
		if (elem_name.empty())
			throw mgr_err::Error("ParamValidation", "It isn't possible to change the element with the empty value");

		for (vparamPtr it = param_list_.begin(); it != param_list_.end(); it++) {
			if (it->getCreateName() == elem_name) {
				it->flag = flag_value;
				return;
			}
		}
	}

	/**
	  * @brief Используется для того, чтобы не плодить кучу экземпляров класса
	  * для тестирования отдельных параметров объекта.
	  */
	void ClearSettings() {
		user_.clear();
		param_name_.clear();
		additional_params.clear();
		param_list_.clear();
		escape_ = false;
	}

	/**
	  * @brief Запуск проверки.
	 */
	void Execute() {
		std::string query = "func=" + func_  + ".edit";
		test_mgrobject::IQuery *provider;
		provider = new Provider;
		bool is_created;

		for (vparamPtr it = param_list_.begin(); it != param_list_.end(); it++) {
			is_created = false;

			try {
				provider->Query(query + "&" + param_name_ + "=" + (escape_ ? str::url::Encode(it->getCreateName()) : it->getCreateName()) + GetAdditionalParams() + (user_.empty() ? "" : ("&su=" + user_)));

				if (!it->getFlag())
					ADD_FAILURE() << "It's possible to create an object '" + func_ + "' with the wrong value = '" + it->getCreateName() + "'. Param = '" + param_name_ + "'";

				is_created = true;
			} catch (const mgr_err::Error& e) {
				if (it->getFlag())
					ADD_FAILURE() << "Can't create an object '" + func_ + "' with the value = '" + it->getCreateName() + "'. Param = '" + param_name_ + "' : " + e.what();
			}

			if (is_created) {
				if (elid_.empty()) {
					try {
						provider->Query("func=" + func_ + ".delete" + "&elid=" + (escape_ ? str::url::Encode(it->getDelName()) : it->getDelName()) + (user_.empty() ? "" : ("&su=" + user_)));
					} catch (const mgr_err::Error& e) {
						ADD_FAILURE() << "Can't remove an object '" + func_ + "' with the value = '" + it->getDelName() + "'. Param '" + param_name_ + "' : " + e.what();
					}
				}

				if (!elid_.empty()) {
					try {
						provider->Query("func=" + func_ + ".delete" + "&elid=" + (escape_ ? str::url::Encode(elid_) : elid_) + (user_.empty() ? "" : ("&su=" + user_)));
					} catch (const mgr_err::Error& e) {
						ADD_FAILURE() << "Can't remove an object '" + func_ + "' with the value = '" + it->getDelName() + "'. Param '" + param_name_ + "' : " + e.what();
					}
				}
			}
		}

		delete provider;
	}

};

// ----------------------------- Списки проверки валидаторов -----------------------------

/**
  * @brief Инициализация списков проверки
 */
void InitValidationLists();

/**
  * @brief Список для проверки валидатора "username"
 */
extern std::vector<ParamToCheck> username_validation_list;
/**
  * @brief Список для проверки валидатора "alphanum"
 */
extern std::vector<ParamToCheck> alphanum_validation_list;
/**
  * @brief Список для проверки валидатора "email"
 */
extern std::vector<ParamToCheck> email_validation_list;
/**
  * @brief Список для проверки валидатора "int"
 */
extern std::vector<ParamToCheck> int_validation_list;
/**
  * @brief Список для проверки валидатора "url"
 */
extern std::vector<ParamToCheck> url_validation_list;

/**
  * @brief Список для проверки валидатора "path"
 */
extern std::vector<ParamToCheck> path_validation_list;

/**
  * @brief Список для проверки валидатора "iprange". Для v4 диапазонов.
 */
extern std::vector<ParamToCheck> iprange_v4_validation_list;
/**
  * @brief Список для проверки валидатора "iprange". Для v6 диапазонов.
 */
extern std::vector<ParamToCheck> iprange_v6_validation_list;
/**
  * @brief Список для проверки валидатора "net" для v4 сетей.
 */
extern std::vector<ParamToCheck> net_v4_validation_list;
/**
  * @brief Список для проверки валидатора "net" для v6 сетей.
 */
extern std::vector<ParamToCheck> net_v6_validation_list;
/**
  * @brief Список для проверки валидатора "ip" для v4 адресов.
 */
extern std::vector<ParamToCheck> ip_v4_validation_list;
/**
  * @brief Список для проверки валидатора "ip" для v6 адресов.
 */
extern std::vector<ParamToCheck> ip_v6_validation_list;
/**
  * @brief Список для проверки валидатора "domain".
 */
extern std::vector<ParamToCheck> domain_validation_list;
/**
  * @brief Список для проверки валидатора "subdomain".
 */
extern std::vector<ParamToCheck> subdomain_validation_list;

// ----------------------------- 3. Работа со списками (поиск в списке, получение параметров и т. д.) -----------------------------

/**
  * @brief Используется для поиска элемента в списке. Поиск может быть осуществлен по 2 параметрам.
  * Искомый элемент может иметь более чем одно вхождение в список.
  * @return true - если элемент существует, в противном случае, false.
  */
template <typename Provider>
bool CheckListFor(const std::string& func, const std::string& param_1, const std::string& value_1, const std::string& user = "", const std::string& param_2 = "", const std::string& value_2 = "") {
	std::string query = "func=" + func + (user.empty() ? "" : "&su=" + user);
	test_mgrobject::IQuery *provider;
	provider = new Provider;
	mgr_xml::Xml xml_response;

	try {
		xml_response = provider->Query(query);
	} catch (const mgr_err::Error& e) {
		ADD_FAILURE() << "Can't execute the query: " + query + ". Error: " + e.what();
		delete provider;
		return false;
	}

	delete provider;
	std::string xpath_query = (param_2.empty()) ? ("/doc/elem[" + param_1 + "=\"" + value_1 + "\"]/" + param_1) :
						 ("/doc/elem[" + param_1 + "=\"" + value_1 + "\" and " + param_2 +
						  + "=\"" + value_2 + "\"]/" + param_1);
	mgr_xml::XPath xp(xml_response, xpath_query);

	if (xp.size() > 0)
		return true;

	return false;
}

/**
 * @brief Шаблонный класс для проверки списков.
 * @details Проверка осуществляется через метод ExecuteCheck. Все ожидаемые значения заполняются через публичные контейнеры.
 * Параметром шаблона является класс проверяемого списка.
 */
enum ListCheckerFlags {
	ckContent = 1,
	ckToolbar = 2,
	ckColumns = 4,
	ckAll = 7
};

template <typename MgrListType>
class ListChecker {
public:
	/**
	 * @brief Конструктор класса
	 * @param [in] user - имя пользователя, из-под которого будет проверяться список.
	 * @param [in] elid - идентификатор родителя в случае проверки дочернего списка.
	 * @param [in] msg_lang - язык мессаг, для которого осущестляется проверка.
	 */
	ListChecker( const string& user = ROOTNAME, const string& elid = "", const string& msg_lang = "ru" ) : User( user ), MsgLang( msg_lang ) {
		User = user;
		MsgLang = msg_lang;
		if( !elid.empty() )
			List["elid"] = elid;
		List.setSU( User );
	}

	/**
	 * @brief Метод запускающий проверку
	 * @param [in] check_content - указывает нужно ли проверять содержимое списока
	 * @param [in] check_toolbar - указывает нужно ли проверять содержимое панели инструментов
	 * @param [in] check_columns - указывает нужно ли проверять колонки
	 */
	void ExecuteCheck( int mask = ckAll ) {
		if( mask & ckColumns ) {
			EXPECT_EQ( ExpectedColumns.size(), List.getColumnsCount() ) << "Incorrect number of columns.";
			StringVector exist_columns;
			List.getColumnNames( MsgLang, exist_columns );
			for( size_t i = 0; i < ExpectedColumns.size() && i < exist_columns.size(); ++i )
				EXPECT_TRUE( ExpectedColumns[i] == exist_columns[i] ) << "Incorrect column #" + str::Str( i )
							+ "\nExpected: \"" + ExpectedColumns[i] + "\""
							+ "\nActual: \"" + exist_columns[i] + "\"";
		}
		if( mask & ckToolbar ) {
			std::vector<StringVector> actual_toolbar;
			List.getButtons( actual_toolbar, MsgLang );
			EXPECT_EQ( ExpectedToolBar.size(), actual_toolbar.size() ) << "Incorrect nubmer of groups in toolbar";
			for( size_t i = 0; i < ExpectedToolBar.size() && i < actual_toolbar.size(); ++i ) {
				EXPECT_EQ( ExpectedToolBar[i].size(), actual_toolbar[i].size() )
					<< "Incorrect number of buttons in group #" + str::Str( i );
					for( size_t j = 0; j < ExpectedToolBar[i].size() && j < actual_toolbar[i].size(); ++j )
						EXPECT_TRUE( ExpectedToolBar[i][j] == actual_toolbar[i][j] )
							<< "Incorrect button #" + str::Str( j ) + " in group #" + str::Str( i )
							+ "\nExpected: \"" + ExpectedToolBar[i][j] + "\""
							+ "\nActual: \"" + actual_toolbar[i][j] + "\"";
			}
		}
		if( mask & ckContent ) {
			StringList current_list;
			List.GetList( current_list, ContentFields );
			bool incorrect_list = false;
			if( ExpectedContent.size() != current_list.size() ) {
				incorrect_list = true;
				ADD_FAILURE() << "Incorrect number of items in the list. "
						"Current list size is " + str::Str( current_list.size() )
						+ ". Expected list size is " + str::Str( ExpectedContent.size() ) + ".";
			}

			string print_expected;
			ForEachI( ExpectedContent, ec ) {
				print_expected += *ec + "\n";
				if( std::find( current_list.begin(), current_list.end(), *ec ) == current_list.end() ) {
					incorrect_list = true;
					ADD_FAILURE() << "Incorrect content of the list. Not found:\n" + *ec;
				}
			}
			if( incorrect_list ) {
				string print_current;
				ForEachI( current_list, cl )
					print_current += *cl + "\n";
				ADD_FAILURE() << "List content check failed. Current list was:\n" + print_current
						<< "\nExpected list was:\n" + print_expected;
			}
		}
	}

	std::vector<StringVector> ExpectedToolBar;
	StringVector ExpectedColumns;
	StringList ExpectedContent, ContentFields;
	string User, MsgLang;
private:
	MgrListType List;
};

/**
 * @brief Шаблонный класс для проверки форм.
 * @details Проверка осуществляется через метод ExecuteCheck. Все ожидаемые значения заполняются через публичные контейнеры.
 * Параметром шаблона является класс проверяемой формы.
 *   e.zhigunova@ispsystem.com
 */
enum FormCheckerFlags {
	fmExistField = 1,
	fmMissedField = 2,
	fmAll = 3
};
template<typename MgrFormType>
class FormChecker {
public:
	/**
	 * @brief Конструктор класса
	 * @param [in] user - имя пользователя, из-под которого будет проверяться форма.
	 * @param [in] elid - идентификатор родителя в случае проверки дочерней формы.
	 * @param [in] msg_lang - язык мессаг, для которого осуществляется проверка.
	 */
	FormChecker(const string& user = ROOTNAME, const string& elid = "", const string& msg_lang = "ru") : User(user), MsgLang(msg_lang) {
		User = user;
		MsgLang = msg_lang;
		if (!elid.empty())
			Form["elid"] = elid;
		Form.setSU(User);
	}

	/**
	 * @brief Метод запускающий проверку
	 * param[in] mask - условие проверки fmExistField - проверять только видимые поля,
	 * fmMissedField - проверить отсутствие полей на форме, fmAll - проверить всё.
	 */
	void ExecuteCheck(int mask = fmAll) const{
		if (mask & fmExistField) {
			StringVector exist_fields;
			Form.getFieldsNames(Form["elid"], MsgLang, exist_fields);
			EXPECT_EQ( ExpectedFields.size(), exist_fields.size()) << "Incorrect number of fields";
			for (size_t i = 0; i < ExpectedFields.size() && i < exist_fields.size(); ++i)
				EXPECT_EQ(ExpectedFields[i], exist_fields[i]) << "Missing required field with name = \"" + ExpectedFields[i];
		}
		if (mask & fmMissedField) {
				StringVector exist_fields;
				Form.getFieldsNames(Form["elid"], MsgLang, exist_fields);
				ForEachI( ExpectedMissedFields, missField ) {
					EXPECT_TRUE(find(exist_fields.begin(), exist_fields.end(), *missField) == exist_fields.end())
								 << "Required field '" + *missField + "' is present in form";
				}
			}
	}

	StringVector ExpectedFields;
	StringVector ExpectedMissedFields;
	string User, MsgLang;
private:
	MgrFormType Form;
};

/**
  * @brief шаблон функции выполняющей перезапуск панели
  */
template<typename Provider>
void mgrQuit() {
	Provider quitmgr;
	try {
		quitmgr.Query("func=exit");
	} catch (const mgr_err::Error& e) {
		ADD_FAILURE() << "Exception in mgrQuit() call. " << e.what();
	}
}

/**
  * @brief шаблон функции выполняющей перезапуск панели
  *   e.zhigunova@ispsystem.com
  */
template<typename Provider>
void mgrRestart() {
	Provider restart_mgr;
	try {
		restart_mgr.Restart();
	} catch (const mgr_err::Error& e) {
		ADD_FAILURE() << "Exception in mgrRestart() call. " << e.what();
	}
}

template<typename Provider>
class mgrWaitRestart {
private:
	Provider mgr;
	bool m_cancel;
	string m_pid;
public:
	mgrWaitRestart()
		: m_cancel(false)
	{
		m_pid = mgr.Query("func=whoami").GetNode("//pid").Str();
	}

	~mgrWaitRestart() {
		if (m_cancel)
			return;

		if (m_pid.empty()) {
			mgrRestart<Provider>();
		} else {
			for (size_t i = 0; i < 100; ++i) {
				try {
					if (mgr.Query("func=whoami").GetNode("//pid").Str() != m_pid) {
						mgr.Query("func=whoami");
						return;
					}
				} catch (...) {
				}
				mgr_proc::Sleep(100 * i);
			}
//			throw mgr_err::Error("restart_fail");
		}
	}

	void Cancel() {
		m_cancel = true;
	}
};

class Scheduler {
private:
	string m_service_name;
public:
	Scheduler() {
		m_service_name =  OSFAMILY_REDHAT ? "crond" : "cron";
	}
	~Scheduler() {
	}
	void Stop()
	{
		mgr_proc::Execute( "service " + m_service_name + " stop" ).Result();
	}
	void Start()
	{
		mgr_proc::Execute( "service " + m_service_name + " start" ).Result();
	}
};

/**
  * @brief функция прерывающая выполнение теста и ожидающая ввода (для дебага)
  */
void InterruptTest(const string msg = "");


/**
 * @brief	Фикстура, пингующая указанный сервер.
 * @param	[in] ip - ip адрес сервера, который нужно попинговать
 *  	a.struzhkin@ispsystem.com
 */
class PingWaiter {
public:
	PingWaiter( const string& ip, bool expect_no_fail );
	bool operator()();
private:
	string Cmd;
	bool NoFail;
};

/**
 * @brief	Фикстура, проверяющая слушает ли Windows-сервер по rdp-порту.
 * @param	[in] ip - ip адрес сервера, который нужно слушать
 *  	a.struzhkin@ispsystem.com
 */
class WindowsWaiter {
public:
	WindowsWaiter( const string& ip, bool power_on );
	bool operator()();
private:
	string Cmd;
	bool PowerOn;
};

/**
  * @brief функция для проверки сообщения об ошибке в сгенерированном исключении.
  *   a.struzhkin@ispsystem.com
  */
void CheckErrorMessage( const mgr_err::Error& e, const string& expected_message );

/**
 * @brief	Функция, возвращающая версию указанного менеджера.
 * @param	[in] manager_name - имя менеджера, например ispmgr
 * @param	[in] full_size - формат вывода команды, true - полный, false - сокращенный
 * @param	[out] версия указанного менеджера в заданном формате
 *  	v.burov@ispsystem.com
 */
string GetManagerVersion( const string& manager_name, bool full_size = true );

/**
 * @brief	Функция проверяет, что указанная директория содержит определенное количество элементов
 * @param	[in] path - путь к директории
 * @param	[in] file_count ожидаемое количество файлов
 * @param	[out] true - если количество соответствует, false - не соответствует
 *  	v.burov@ispsystem.com
 */
bool CheckFileCountInDir( const string& path, size_t file_count );

/**
 * @brief	Какие элементы искать и считать(директории, файлы, симлинки)
 *  	n.kinash@ispsystem.com
 */
enum SearchType {
  stFile = 1, ///< Искать файлы
  stDir = 2, ///< Искать директории
  stSymlink = 4 ///< Искать симлинки
};

/**
 * @brief	Функция возвращает число элементов(файлов и(или) дир и(или) симлинок) в заданной директории,
 * содержащих в своем имени указанную последовательность символов
 * @param	[in] path - путь к директории
 * @param	[in] name - последовательность символов, которые должны содержатся в имени элемента
 * @param	[in] search - какой тип элемента искать
 * @param	[out] количество найденных элементов
 *  	n.kinash@ispsystem.com
 */
int FindElemInDir( const string& path, const string& name, SearchType search = stFile);

/**
 * @brief	Найти строки содержащие искомую последовательность символов, результат положить в любой стандартный контейнер
 * @param	[in] data - мета-строка(содержащая реальные строки с \\n на конце). Мета-строка будет разбита на строки,
 * в каждой из которых будет производится поиск
 * @param	[in] pattern - последовательность символов, которые должны содержатся в выходных строках.
 * @param	[out] стандартный контейнер заполненный строками содержащими pattern, если ничего не было найдено - возвращается пустой контейнер
 *  	n.kinash@ispsystem.com
 */
template<typename Container>
Container Grep(const string& data, const string& pattern) {
	Container result, lines;
	str::Split(data, lines, "\n");
	ForEachI(lines, line) {
		if (line->find(pattern) != std::string::npos) {
			result.push_back(*line);
		}
	}
	return result;
}

/**
  * @brief функция для получения информации об ошибке в старом стиле(напр. Type: 'exists').
  * [out] строка сообщения об ошибке в формате: Type: '' Object: '' Value: ''
  *   n.kinash@ispsystem.com
  */
string error_what(const mgr_err::Error& e);

/**
  * @brief  Функция для конвертации xml в json
  * @param  [in]  Строка в формате xml
  * @param  [out] Строка в формате json
  *   s.butakov@ispsystem.com
  */
std::string xml2json(const std::string& source);

/**
  * @brief  Функция выполняет команду на удаленной ноде (ISPManager PRO)
  * @param  [in]  Адрес удаленной ноды
  * @param  [in]  Команда
  * @param  [out] Результат команды
  *   s.butakov@ispsystem.com
  */
std::string ExecSSH(const std::string& ipaddr, const std::string& cmd = "");

/**
  * @brief	Генерирует файлы статистики за указанный период
  * @brief	Статистика генерируется по указанному параметру случайным образом. Остальные параметры записываются нулями
  * @param	[in]  Список параметров файла статистики
  * @param	[in]  Параметр, по которому будет сгенерирована статистика
  * @param	[in]  Начало периода
  * @param  [in]  Окончание периода
  * @param  [in]  Период квантования в секундах
  * @param  [in]  Путь к директории со статистикой. Если не существует, то будет создана
  * @param  [in]  Минимальное ограничение параметра
  * @param  [in]  Максимальное ограничение параметра
  *  	a.mitroshin@ispsystem.com
  */
void GenStat(StringList& paramList, const string param, const mgr_date::DateTime& begin, const mgr_date::DateTime& end,
				   const int period, const string& path, const long int& min, const long int& max, bool add_hour = true);

/**
 * @brief	Собирает файлы суточной статистики из файлов часовой статистики
 *  	a.mitroshin@ispsystem.com
 */
void MakeStatDay(const string& hourPath, const string& dayPath);

/**
 * @brief	Читает сырую статистику из файла
 *  	a.mitroshin@ispsystem.com
 */
void GetRawStat(StringList& outList, const std::string& param, const std::string& path, const mgr_date::DateTime &begin, const mgr_date::DateTime &end, bool addDate = true);
void GetRawStatInt64(std::list<int64_t>& outList, const std::string& param, const std::string& path, const mgr_date::DateTime &begin, const mgr_date::DateTime &end);

/**
 * @brief	Экспортирует файлы статистики на удаленный узел
 *  	a.mitroshin@ispsystem.com
 */
void RawStatExport(const string& ipaddr, const string& localPath, const string& remotePath, const string& key);

/**
 * @brief	Импортирует файлы статистики с удаленного узла
 *  	a.mitroshin@ispsystem.com
 */
void RawStatImport(const string& ipaddr, const string& localPath, const string& remotePath, const string& key);


/**
* @brief Возвращает логин админа по умолчанию. Он создается в Environment::SetUp. Раньше этого звать нельзя.
* @param name
* @return
*/
std::string GetDefaultAdmin(const string name = "");


/**
* @brief Расшифровывает зашифрованный пароль
* @param value - зашифрованный пароль
* @param pkey_path - путь к ключу
* @return возвращает расшифрованный пароль
*/
#define CRYPTED_FIELD_DELIMETER '|'
string DecryptFieldValue(string value, const std::string pkey_path);

}; // test_functions

#endif // FUNCTIONS_H
