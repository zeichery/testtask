#include "utils.h"
#include "provider.h"
#include <mgr/mgrlog.h>
MODULE("test");

namespace billmgr {
namespace functions {

bool IsProcessActive(const string& process_name){
	return mgr_proc::Exists(process_name);
}

string RandomString(const size_t len, const string& allowed_symbols)
{
	const string random_bin(str::Random(len));
	string random_str(len, '0');
	for(size_t i = 0; i < len; ++i)
		random_str[i] = allowed_symbols[random_bin[i] % allowed_symbols.size()];
	return random_str;
}

string GenPhoneNumber() {
	return "+7 " + RandomString(3) + RandomString(7);
}


int DaysInMonth(mgr_date::Date date)
{
date.AddDays(-date.day() + 1);
mgr_date::Date nextmonth(date);
nextmonth.AddMonth();
return nextmonth - date;
}

std::string GetDefaultAdmin(const std::string name)
{
	static mgr_thread::SafeLock lock;
	mgr_thread::SafeSection ss(lock);
	static string admin_name = name;
	if (admin_name.empty())
		throw mgr_err::Error("Call std::string GetDefaultAdmin(const std::string name) before Environment::SetUp");
	return admin_name;
}

std::string GetRemoteDefaultAdmin(const std::string remote_name)
{
	static mgr_thread::SafeLock lock;
	mgr_thread::SafeSection ss(lock);
	static string remote_admin_name = remote_name;
	return remote_admin_name;
}

std::string GetMainLocale(const bool from_cache)
{
	static string main_locale;
	if (!from_cache || main_locale.empty()) {
		test_mgrobject::LocalQuery conn;
		main_locale = conn.Query("func=locale").GetNode("/doc/elem[default_lang='on' and active='on']/langcode").Str();
		Debug("main locale = '%s'", main_locale.c_str());
	}
	return main_locale;
}

std::string AddLocalePostfix(const std::string &field_name) {
	string main_locale = GetMainLocale();
	if (main_locale == "en")
		return field_name;
	else
		return field_name + "_" + main_locale;
}

const StringList &GetLocaleList(const bool from_cache)
{
	static StringList locales;
	if (!from_cache || locales.empty()) {
		locales.clear();
		test_mgrobject::LocalQuery conn;
		auto locale_xpath = conn.Query("func=locale").GetNodes("/doc/elem[embedded='on' and active='on']/langcode");
		ForEachI(locale_xpath, e)
			locales.push_back(e->Str());
	}
	return locales;
}

void ClearCache(const std::string &table_name)
{
	test_mgrobject::LocalQuery().Query("func=tool.clearcache&out=xml&table=" + str::url::Encode(table_name));
}

std::string EscapeDomain(const std::string& input_domain) {
	string domain = str::Lower(str::puny::Encode(UnEscapeDomain(input_domain)));
	str::inpl::Replace(domain, "-", "_");
	str::inpl::Replace(domain, ".", "____________");
	return domain;
}

std::string UnEscapeDomain(const std::string& input_domain) {
	string domain = input_domain;
	str::inpl::Replace(domain, "____________", ".");
	str::inpl::Replace(domain, "_", "-");
	return domain;
}

double PriceWithDiscount(double price, double discount){
	return (price * (100 - discount)/100);
}

string GetPackageName(const string& path_to_lib){
	string package_name;
	if (mgr_env::GetOsType() == mgr_env::osCentOS) {
		test_mgrobject::LocalQuery().SystemOut("rpm --queryformat '%{NAME}'  -qf " + path_to_lib, package_name);
	}
	if (mgr_env::GetOsType() == mgr_env::osDebian) {
		test_mgrobject::LocalQuery().SystemOut("dpkg -S " + path_to_lib, package_name);
		str::RGetWord(package_name, ":");
	}
	return package_name;
}


void FilesToRun(const string& files_string, StringList& files_to_run){
	string file_list = files_string;
	while (!file_list.empty()) {
		string file = str::GetWord(file_list);
		string filename = mgr_file::SplitPath(file).second;
		mgr_file::Info fileInfo(file);
		if (fileInfo.IsExecutable() && !fileInfo.IsDir() && (filename.find('.') == string::npos)) {
			files_to_run.push_back(file);
		}
	}
}

}
}
