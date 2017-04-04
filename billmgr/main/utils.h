#ifndef BILLMGR_FUNCTIONS_H
#define BILLMGR_FUNCTIONS_H
#include "../../mgr/functions.h"

namespace billmgr {
namespace functions {
/**
 * @brief Проверяет запущен ли процесс
 * @return
 */
bool IsProcessActive(const string& process_name);

/**
* @brief Функтор, ожидающий появления процесса
*
*/
class CheckProcessActiveWaiter {
public:
	CheckProcessActiveWaiter(const string& process_name) : processname(process_name){
	}
	bool operator()() {
		return billmgr::functions::IsProcessActive(processname);
	}
private:
	string processname;
};

/**
 * @brief Возвращает Количество дней в месяце.
 * @param [date] Дата от которой хотим получить количество дней в месяце
 * @return
 */
int DaysInMonth(mgr_date::Date date);

/**
 * @brief Возвращает логин админа по умолчанию. Он создается в Environment::SetUp. Раньше этого звать нельзя.
 * @param name
 * @return
 */
string GetDefaultAdmin(const string name = "");

/**
 * @brief Возвращает логин админа по умолчанию с продающего сервера. Он создается в Environment::SetUp.
 * @param name
 * @return
 */
string GetRemoteDefaultAdmin(const string remote_name = "");
/**
* @brief	Генерирует случайную строку
* @param	[len] Длинна строки
* @param	[allowed_symbols] разрешенные символы
* @returns	[out] Случайная строка
* 
*/
string RandomString(const size_t len, const string& allowed_symbols = "123456789");

/**
* @brief	Генерирует номер телефона
* @returns	[out] Валидный телефонный номер
* 
*/
string GenPhoneNumber();
/**
 * @brief	Возвращает код основной локализации
 * @param	from_cache Разрешает возвращать кэшированное значение
 * @return
 * 
 */
string GetMainLocale(const bool from_cache = true);
/**
 * @brief Добавляет к имени поля постфикс локализации по умолчанию
 * @param field_name
 * @return
 * 
 */
string AddLocalePostfix(const string& field_name);
/**
 * @brief GetLocaleList Список локализаций в биллинге
 * @param from_cache Разрешает возвращать кэшированное значение
 * @return
 */
const StringList& GetLocaleList(const bool from_cache = true);
/**
 * @brief Очистить кэш для указанной таблицы
 * @param table_name имя таблицы
 */
void ClearCache(const string& table_name);

/**
* @brief Используется при заказе домена, заменяет тире и точки на подчерки,
* чтобы можно было использовать в качестве параметров
*/
string EscapeDomain(const string& input_domain);

/**
* @brief Используется при заказе домена, заменяет подчерки на тире и точки,
* чтобы можно было использовать в качестве параметров
*/
string UnEscapeDomain(const string& input_domain);

/**
* @brief Считает цену со скидкой
*/
double PriceWithDiscount(const double price, const double discount);

/**
* @brief получает имя пакета по файлу
*/
string GetPackageName(const string& path_to_lib);

/**
* @brief возвращает список запускаемых файлов
* принимает строку со списком файлов
*/
void FilesToRun(const string& files_string, StringList& files_to_run);

template <typename Provider = test_mgrobject::LocalQuery>
class QueryThread {
public:
	QueryThread(const string& query, const int& retry_count = 1)
		: m_query		(query)
		, m_retry_count	(retry_count)
		, m_result		(-1)
	{

	}

	void operator()() {
		try {
			for (int i = 0; i < m_retry_count; ++i) {
				m_last_result_xml = m_client.Query(m_query);
			}
			m_result = EXIT_SUCCESS;
		} catch (...) {
			m_result = EXIT_FAILURE;
		}
	}
	/**
	 * @brief Статус выполнения задачи
	 * @return EXIT_SUCCESS - успешно, EXIT_FAILURE - ошибка, -1 не завершено или не запускалось.
	 */
	int Result() {
		return m_result;
	}
	/**
	 * @brief xml ответ последнего запроса
	 * @return
	 */
	mgr_xml::Xml LastResultXML() {
		return m_last_result_xml;
	}

private:
	string m_query;
	int m_retry_count;
	Provider m_client;
	int m_result;
	mgr_xml::Xml m_last_result_xml;
};
}
}
#endif // BILLMGR_FUNCTIONS_H
