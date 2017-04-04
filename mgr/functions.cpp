#include "mgr/mgrproc.h"
#include "functions.h"

using namespace std;
using namespace mgr_proc;
using namespace test_functions;
using namespace str;

namespace test_functions
{
	vector<ParamToCheck> username_validation_list;
	vector<ParamToCheck> alphanum_validation_list;
	vector<ParamToCheck> email_validation_list;
	vector<ParamToCheck> int_validation_list;
	vector<ParamToCheck> url_validation_list;
	vector<ParamToCheck> path_validation_list;
	vector<ParamToCheck> iprange_v4_validation_list;
	vector<ParamToCheck> iprange_v6_validation_list;
	vector<ParamToCheck> net_v4_validation_list;
	vector<ParamToCheck> net_v6_validation_list;
	vector<ParamToCheck> ip_v4_validation_list;
	vector<ParamToCheck> ip_v6_validation_list;
	vector<ParamToCheck> domain_validation_list;
	vector<ParamToCheck> subdomain_validation_list;



}

FileWithSize::FileWithSize( const string& fname, size_t size, bool su ) : tmp( fname ), m_size( size * 1024 ) {
		if(su) mgr_file::Info( mgr_file::Path( tmp.Str() ) ).GetAttrs().SafeSet( tmp.Str() );
		mgr_file::Write( tmp.Str(), str::Random(m_size) );
}

string FileWithSize::Str() const {
		return tmp.Str();
}

//ExecuteAndOutWaiter
ExecuteAndOutWaiter::ExecuteAndOutWaiter( const string& output, const string& exec )
												: m_output( output ), m_exec( exec ) {}
bool ExecuteAndOutWaiter::operator()() {
		return m_output == mgr_proc::Execute( m_exec ).Str();
}

//ExecuteAndCountWaiter
ExecuteAndCountWaiter::ExecuteAndCountWaiter( size_t number_of_strings, const string& exec )
												: m_num( number_of_strings ), m_exec( exec ) {}
bool ExecuteAndCountWaiter::operator()() {
		StringVector num_strings;
	str::Split( mgr_proc::Execute( m_exec ).Str(), "\n", num_strings );
		return m_num == num_strings.size();
}

#ifndef WIN32
string test_functions::GetHostName(const std::string& user, const std::string& password, const std::string& ip) {
		mgr_rpc::SSHpass ssh( ip, user, password );
		ssh.Connect();
		string result = str::Trim( ssh.Execute( "hostname" ).Str() );
		if(ssh.Result() != 0) {
			throw mgr_err::Error( "Error", "failed to get hostname via ssh: ", ssh.Str() );
		}
		return result;
}
#endif
string test_functions::SimpleIpGenerator() {
	return str::Format( "%d.%d.%d.%d", str::Random<unsigned int>() % 256, str::Random<unsigned int>() % 256,
										   str::Random<unsigned int>() % 256, str::Random<unsigned int>() % 256 );
}

std::string test_functions::GenNetAddr(int prefix) {
	return mgr_net::IpRange(SimpleIpGenerator() + "/" + str::Str(prefix)).FirstIp().Str() + "/" + str::Str(prefix);
}

FileChangeWaiter::FileChangeWaiter( const string& file_path, const string& old_md5 ) {
	OldMD5 = old_md5;
	FilePath = file_path;
}

bool FileChangeWaiter::operator ()() {
	return OldMD5 != mgr_proc::Execute( "md5sum " + FilePath ).Str();
}

FileSaver::FileSaver() {
	Saved = false;
}

FileSaver::FileSaver( const string& file_path, bool restore_on_destructor ) {
	Saved = false;
	Save( file_path, RestoreOnDestructor );
}

void FileSaver::Save( const string& file_path, bool restore_on_destructor ) {
	if( Saved )
		throw mgr_err::Exists( MGR "test_file_saver", "File already saved." );
	OriginalPath = file_path;
	RestoreOnDestructor = restore_on_destructor;
	SavedFilePath = file_path + "." + test_functions::GenName();
	mgr_file::Copy( OriginalPath, SavedFilePath );
	Saved = true;
}

void FileSaver::Restore() {
	if( !Saved )
		throw mgr_err::Missed( MGR "test_file_saver", "Nothing saved. Call FileSaver::Save first" );
	mgr_file::Move( SavedFilePath, OriginalPath );
	Saved = false;
}

FileSaver::~FileSaver() {
	if( RestoreOnDestructor )
		try {
			Restore();
		} catch(...) {}
}

MgrConfig::MgrConfig() {
	Saved = false;
}

MgrConfig::MgrConfig(const string& conf_path, bool restore_on_destructor) {
	Saved = false;
	RestoreOnDestructor = restore_on_destructor;
	ConfPath = conf_path;
	file_saver.Save(ConfPath, RestoreOnDestructor);
}

void MgrConfig::ReadParams() {
	string out;
	string param_type;
	StringList ConfList;
	out = mgr_file::Read(ConfPath);
	str::Split(out, ConfList, "\n");
	options.clear();
	paths.clear();
	params.clear();
	ForEachI(ConfList, str_paramp) {
		param_type = str::GetWord(*str_paramp);
		if(param_type == "Option") {
			options.push_back(str::Trim(*str_paramp));
		}
		else if(param_type == "path") {
			string path_name = str::GetWord(*str_paramp);
			paths[path_name] = str::Trim(str::GetWord(*str_paramp));
		}
		else {
			params[param_type] = str::Trim(*str_paramp);
		}

	}
}

void MgrConfig::WriteParams() {
	string out;

	ForEachI(paths, pathp)
			out = out + "path " + pathp->first + " " + pathp->second + "\n";
	ForEachI(params, paramp)
			if(!paramp->second.empty()) out = out + paramp->first + " " + paramp->second + "\n";
	ForEachI(options, optionp)
			out = out + "Option " + *optionp + "\n";
	mgr_file::Write(ConfPath, out);
}

void MgrConfig::Open(const string& conf_path, bool restore_on_destructor) {
	if(Saved)
		throw mgr_err::Exists(MGR "test_mgr_config", "File already saved.");
	RestoreOnDestructor = restore_on_destructor;
	ConfPath = conf_path;
	file_saver.Save(ConfPath);
}

void MgrConfig::Restore() {
	file_saver.Restore();
}

bool MgrConfig::FindOption(const string& name) {
	ReadParams();
	ForEachI(options, optionp) {
		if(*optionp == name)
			return true;
	}
	return false;
}

void MgrConfig::SetOption(const string& name, bool val) {
	test_mgrobject::LocalQuery local;
	if (val){
		local.Query("func=optionlist.edit&elid=" + name + "&value=on&sok=ok");
	} else {
		local.Query("func=optionlist.edit&elid=" + name + "&value=off&sok=ok");
	}
//	if(val && !FindOption(name)) {
//		options.push_back(name);
//		WriteParams();
//	}
//	else if(!val && FindOption(name)) {
//		ForEachI(options, optionp) {
//			if (*optionp == name) {
//				options.erase(optionp);
//				optionp--;
//				WriteParams();
//				continue;
//			}
//		}
//	}

}

std::string MgrConfig::GetParam(const std::string& name) {
	ReadParams();
	return params[name];
}

void MgrConfig::SetParam(const std::string& name, const std::string& val) {
	ReadParams();
	params[name] = val;
	WriteParams();
}

std::string MgrConfig::GetPath(const std::string& name) {
	ReadParams();
	return paths[name];
}

MgrConfig::~MgrConfig() {
	if(RestoreOnDestructor) {
		file_saver.Restore();
		test_functions::mgrRestart<test_mgrobject::LocalQuery>();
	}
}

#ifndef WIN32
// ServerStatusWaiter
ServerRebootWaiter::ServerRebootWaiter( const string& ip, const string& rootpass, bool poweron ) {
	Ip = ip;
	RootPass = rootpass;
	PowerOn = poweron;
	NetcatCmd = "nc";
	if( mgr_env::GetOs().find( "FreeBSD" ) != string::npos && ip.find( ':' ) != string::npos )
		NetcatCmd += "6";
	NetcatCmd += " -w 1 -z " + Ip + " 22";
}

bool ServerRebootWaiter::operator()() {
	bool port22listen = !mgr_proc::Execute( NetcatCmd ).Result();
	if( port22listen ) {
		if( PowerOn )
			try {
				mgr_rpc::SSHpass ssh( Ip, ROOTNAME, RootPass );
				ssh.Connect();
				ssh.Disconnect();
				return true;
			} catch(...) {
				return false;
			}
		else
			return false;
	} else if( PowerOn )
		return false;
	else
		return true;
}

void test_functions::RebootCheck::Init( const string& ip, const string& rootpass ) {
	Ip = ip;
	RootPass = rootpass;
	string filename = ".lock." + test_functions::GenName(); //lock - костыль для Centos
	mgr_rpc::SSHpass ssh( Ip, "root", RootPass );
	ssh.Connect();
	ASSERT_EQ( 0, ssh.Execute( "touch /tmp/" + filename ).Result() );
	ssh.Disconnect();
	FileName = filename;
}

bool test_functions::RebootCheck::IsRebooted() const {
	if( FileName.empty() )
		throw mgr_err::Error( "RebootCheck", "Init should be called first" );
	mgr_rpc::SSHpass ssh( Ip, "root", RootPass );
	ssh.Connect();
	bool rebooted = ssh.Execute( "stat /tmp/" + FileName ).Result();
	ssh.Disconnect();
	return rebooted;
}
#endif

string test_functions::GenCyrillicName(int size) {
		string s;
		string result;
		string alphanum = "абвгдеёжзийклмнопрстуфхцчшщъыьэюя";
		for (int i = 0; i < size; ++i) {
				int j = rand() % (alphanum.length()/2);
				s = alphanum.substr(2*j,2);
				result += s;
		}
		return result;
}

/**
  * @brief Генерирование случайной строки заданной длины, содержащей только символы 'a' - 'z'
  * @param [in] size Требуемая длина строки
  * @return	Возвращает сгенерированную строку
 */
string test_functions::GenName(int size) {
	string res = str::Random(size);
	for (size_t i = 0; i < res.length(); ++i) {
		res[i] = 'a' + (char)((unsigned char)res[i] % ('z' - 'a'));
	}
	return res;
}
/**
  * @brief Генерирование пароля заданной длины, содержащего только разрешенные символы 'a-z', 'A-Z', цифры '0-9'
  * @param [in] size Требуемая длина строки
  * @return	Возвращает сгенерированную строку
 */
string test_functions::GenPass(int size) {
  static const char alphanum[] =
  "0123456789"
  /* манагер по умолчанию не использует такие символы в генерации пароля: "!@#$%^&*" т.ч. и мы не будем. пока. */
  "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
  "abcdefghijklmnopqrstuvwxyz";
  int stringLength = sizeof(alphanum) - 1;
        string res = str::Random(size);
        for (size_t i = 0; i < res.length(); ++i) {
            res[i] = alphanum[rand() % stringLength];
        }
        return res;
}
string test_functions::GenDomainName( bool needRoot ) {
	string result = test_functions::GenName() + "." + test_functions::GenName(3);
	if( needRoot )
		result += ".";
	return result;
}

string test_functions::GenMailBoxName() {
	string domain = test_functions::GenDomainName();
	domain.erase(domain.size() - 1, 1);
	return GenName(5) + "@" + domain;
}

string test_functions::GetPTRRecord(const string& ip, bool classless, const string &server) {
	string s;
	if (classless)
		s = Execute("dig @" + server + " -t PTR " + ip, Execute::efOut).Str();
	else
	{
		s = Execute("dig @" + server + " -x " + ip, Execute::efOut).Str();
	}

	if (ip.find(":") != string::npos)
		str::GetWord(s, "PTR ");
	else
		str::GetWord(s, "PTR\t");

	return str::puny::Encode(str::GetWord(s, "\n"));
}

int test_functions::GetSerial(const string& zoneFilePath) {
	string zoneFile = mgr_file::ReadE(zoneFilePath);

	int i = zoneFile.find("(", zoneFile.find("SOA")) + 1;
	int j = zoneFile.find(" ", i);

	return atoi(zoneFile.substr(i, j - i).c_str());
}

string test_functions::GetManagerUrl(short mask) {
	string ihttpdResp = Execute(mgr_file::GetCurrentDir() + "/sbin/ihttpd", Execute::efOut).Str();

	string cut = ihttpdResp.substr(ihttpdResp.find("listening:") + 11);
	cut.erase( cut.find( '\n' ) );
	string strPort = RGetWord(cut, ':');
	string strIp = cut;

	string result;

	if (mask & URL_HTTP) result += "https://";
	if (mask & URL_IP) result += strIp;
	if (mask & URL_PORT) result += ((result.size()) ? ":" : "") + strPort;
	if (mask & URL_MGR) result += ((result.size()) ? "/" : "") + (string)MGR;

	return result;
}

template <typename Provider>
string test_functions::GetManagerUrlT(short mask) {
	string out;
	string query = mgr_file::GetCurrentDir() + "/sbin/ihttpd";
	Provider().SystemOut(query, out);

	string cut = out.substr(out.find("listening:") + 11);
	cut.erase( cut.find( '\n' ) );
	string strPort = RGetWord(cut, ':');
	string strIp = cut;

	string result;

	if (mask & URL_HTTP) result += "https://";
	if (mask & URL_IP) result += strIp;
	if (mask & URL_PORT) result += ((result.size()) ? ":" : "") + strPort;
	if (mask & URL_MGR) result += ((result.size()) ? "/" : "") + (string)MGR;

	return result;
}

string test_functions::Authenticate(const string& username, const string& password , const string &project) {
	string query = GetManagerUrl() + "?func=auth&username=" + username + "&password=" + password + "&out=xml";
	if(!project.empty())
		query += "&project=" + project;
	string respStr = mgr_rpc::FetchString(query);

	mgr_xml::XmlString respXml(respStr);
	mgr_xml::XPath authNode(respXml, "/doc/auth/@id");

	return ( authNode.size() == 1 ) ? authNode[0].Str() : "";
}

bool test_functions::CheckAuthenticate(const string& username, const string& password, const string& project) {
	return !test_functions::Authenticate(username, password, project).empty();
}

void test_functions::AuthWithUrl(const string& url, const string& username , const string &passwd) {
	mgr_client::Remote remote_query("https://" + url + "/billmgr");
	mgr_xml::Xml xmlDoc;
	xmlDoc = remote_query.Query("func=auth&username=" + username + "&password=" + passwd).xml;
}

bool test_functions::EnterAsUser(const std::string &username, const std::string &parent)
{
	mgr_xml::Xml xml = SendRequestOutXML<test_mgrobject::LocalQuery>("elid="+username+"&func=su&su="+parent+"&out=xml");
	mgr_xml::XPath xp(xml, "/doc/ok");
	return !xp.empty();
}

string test_functions::GetPublicKey() {
	return mgr_file::Read(mgr_file::GetCurrentDir() + "/etc/ssh_id_rsa.pub");
}

#ifndef WIN32
void test_functions::AddPublicKeyToServer(const std::string& ip, const std::string& rootPassword) {
	string publicKey = GetPublicKey(); // Публичный ключ панели
	mgr_rpc::SSHpass ssh(ip, "root", rootPassword);
	ssh.Execute("cat /root/.ssh/authorized_keys | grep \"" + publicKey + "\"");
	if (ssh.Str().empty()) ssh.Execute("echo \"" + publicKey + "\" >> /root/.ssh/authorized_keys");
}
#endif
void test_functions::InterruptTest(const string msg) {
	if (msg != "")
		cout << msg << endl;
	cout << "Test interrupted. To continue type 'y': ";
	cout.flush();
	while (cin.get() != 'y');
	cout << "\nContinue..." << std::endl;
}

void test_functions::InitValidationLists() {
	username_validation_list.push_back(ParamToCheck(true, "username"));
	username_validation_list.push_back(ParamToCheck(true, "user.name"));
	username_validation_list.push_back(ParamToCheck(true, "user..name"));
	username_validation_list.push_back(ParamToCheck(true, "12345polo"));

	username_validation_list.push_back(ParamToCheck(false, "пользователь"));
	username_validation_list.push_back(ParamToCheck(false, "12345"));
	username_validation_list.push_back(ParamToCheck(false, " "));
	username_validation_list.push_back(ParamToCheck(false, ","));
	username_validation_list.push_back(ParamToCheck(false, "\t"));
	username_validation_list.push_back(ParamToCheck(false, ":"));
	username_validation_list.push_back(ParamToCheck(false, "%2B"));
	username_validation_list.push_back(ParamToCheck(false, "%26"));
	username_validation_list.push_back(ParamToCheck(false, "#"));
	username_validation_list.push_back(ParamToCheck(false, "%25"));
	username_validation_list.push_back(ParamToCheck(false, "$"));
	username_validation_list.push_back(ParamToCheck(false, "^"));
	username_validation_list.push_back(ParamToCheck(false, "("));
	username_validation_list.push_back(ParamToCheck(false, ")"));
	username_validation_list.push_back(ParamToCheck(false, "{"));
	username_validation_list.push_back(ParamToCheck(false, "}"));
	username_validation_list.push_back(ParamToCheck(false, "["));
	username_validation_list.push_back(ParamToCheck(false, "]"));
	username_validation_list.push_back(ParamToCheck(false, "!"));
	username_validation_list.push_back(ParamToCheck(false, "@"));
	username_validation_list.push_back(ParamToCheck(false, "~"));
	username_validation_list.push_back(ParamToCheck(false, "*"));
	username_validation_list.push_back(ParamToCheck(false, "%3F"));
	username_validation_list.push_back(ParamToCheck(false, "<"));
	username_validation_list.push_back(ParamToCheck(false, ">"));
	username_validation_list.push_back(ParamToCheck(false, "="));
	username_validation_list.push_back(ParamToCheck(false, "|"));
	username_validation_list.push_back(ParamToCheck(false, "\\"));
	username_validation_list.push_back(ParamToCheck(false, "/"));
	username_validation_list.push_back(ParamToCheck(false, "\""));
	username_validation_list.push_back(ParamToCheck(false, "abcdabcdabcdeabcde"));
	username_validation_list.push_back(ParamToCheck(false, "-name"));
	username_validation_list.push_back(ParamToCheck(false, ".name"));
	username_validation_list.push_back(ParamToCheck(false, "new name"));
	username_validation_list.push_back(ParamToCheck(false, "name."));
	username_validation_list.push_back(ParamToCheck(false, "..."));
	username_validation_list.push_back(ParamToCheck(false, BIG_VALUE));

	alphanum_validation_list.push_back(ParamToCheck(true, "ALPHA"));
	alphanum_validation_list.push_back(ParamToCheck(true, "ALP-HA"));
	alphanum_validation_list.push_back(ParamToCheck(true, "Alpha"));
	alphanum_validation_list.push_back(ParamToCheck(true, "alpha"));
	alphanum_validation_list.push_back(ParamToCheck(true, "al_ph-a"));
	alphanum_validation_list.push_back(ParamToCheck(true, "abc123"));
	alphanum_validation_list.push_back(ParamToCheck(true, "_"));

	alphanum_validation_list.push_back(ParamToCheck(false, "12345"));
	alphanum_validation_list.push_back(ParamToCheck(false, "12345polo"));
	alphanum_validation_list.push_back(ParamToCheck(false, "-"));
	alphanum_validation_list.push_back(ParamToCheck(false, "имя"));
	alphanum_validation_list.push_back(ParamToCheck(false, "new name"));
	alphanum_validation_list.push_back(ParamToCheck(false, " "));
	alphanum_validation_list.push_back(ParamToCheck(false, ","));
	alphanum_validation_list.push_back(ParamToCheck(false, "\t"));
	alphanum_validation_list.push_back(ParamToCheck(false, ":"));
	alphanum_validation_list.push_back(ParamToCheck(false, "%2B"));
	alphanum_validation_list.push_back(ParamToCheck(false, "%26"));
	alphanum_validation_list.push_back(ParamToCheck(false, "#"));
	alphanum_validation_list.push_back(ParamToCheck(false, "%25"));
	alphanum_validation_list.push_back(ParamToCheck(false, "$"));
	alphanum_validation_list.push_back(ParamToCheck(false, "^"));
	alphanum_validation_list.push_back(ParamToCheck(false, "("));
	alphanum_validation_list.push_back(ParamToCheck(false, ")"));
	alphanum_validation_list.push_back(ParamToCheck(false, "{"));
	alphanum_validation_list.push_back(ParamToCheck(false, "}"));
	alphanum_validation_list.push_back(ParamToCheck(false, "["));
	alphanum_validation_list.push_back(ParamToCheck(false, "]"));
	alphanum_validation_list.push_back(ParamToCheck(false, "!"));
	alphanum_validation_list.push_back(ParamToCheck(false, "@"));
	alphanum_validation_list.push_back(ParamToCheck(false, "~"));
	alphanum_validation_list.push_back(ParamToCheck(false, "*"));
	alphanum_validation_list.push_back(ParamToCheck(false, "%3F"));
	alphanum_validation_list.push_back(ParamToCheck(false, "<"));
	alphanum_validation_list.push_back(ParamToCheck(false, ">"));
	alphanum_validation_list.push_back(ParamToCheck(false, "="));
	alphanum_validation_list.push_back(ParamToCheck(false, "|"));
	alphanum_validation_list.push_back(ParamToCheck(false, "\\"));
	alphanum_validation_list.push_back(ParamToCheck(false, "/"));
	alphanum_validation_list.push_back(ParamToCheck(false, "\""));
	alphanum_validation_list.push_back(ParamToCheck(false, "имяname"));

	email_validation_list.push_back(ParamToCheck(true, "123@mail.com"));
	email_validation_list.push_back(ParamToCheck(true, "box@mail.com"));
	email_validation_list.push_back(ParamToCheck(true, "box@раша.ком"));
	email_validation_list.push_back(ParamToCheck(true, "%2Bbox@mail.com"));
	email_validation_list.push_back(ParamToCheck(true, "-box@mail.com"));
	email_validation_list.push_back(ParamToCheck(true, "=box@mail.com"));
	email_validation_list.push_back(ParamToCheck(true, "`box@mail.com"));
	email_validation_list.push_back(ParamToCheck(true, "__box@mail.com"));
	email_validation_list.push_back(ParamToCheck(true, "_box@mail.com"));
	email_validation_list.push_back(ParamToCheck(true, "b.ox@mail.com"));

	email_validation_list.push_back(ParamToCheck(false, "ящик@new@box.com"));
	email_validation_list.push_back(ParamToCheck(false, "box@new@box.com"));
	email_validation_list.push_back(ParamToCheck(false, " "));
	email_validation_list.push_back(ParamToCheck(false, "box..@mail.com"));
	email_validation_list.push_back(ParamToCheck(false, "box@mail..com"));
	email_validation_list.push_back(ParamToCheck(false, ".box@mail.com"));
	email_validation_list.push_back(ParamToCheck(false, "box.@mail.com"));
	email_validation_list.push_back(ParamToCheck(false, ".@mail.com"));
	email_validation_list.push_back(ParamToCheck(false, "12345"));
	email_validation_list.push_back(ParamToCheck(false, "abcd"));
	email_validation_list.push_back(ParamToCheck(false, "абвг"));
	email_validation_list.push_back(ParamToCheck(false, "bo^x@mail.com"));
	email_validation_list.push_back(ParamToCheck(false, "@domain.com"));
	email_validation_list.push_back(ParamToCheck(false, "@doma_in.com"));
	email_validation_list.push_back(ParamToCheck(false, "@com."));
	email_validation_list.push_back(ParamToCheck(false, BIG_VALUE));

	int_validation_list.push_back(ParamToCheck(true, "10"));
	int_validation_list.push_back(ParamToCheck(true, "123456789"));

	int_validation_list.push_back(ParamToCheck(false, "abcd"));
	int_validation_list.push_back(ParamToCheck(false, "~"));
	int_validation_list.push_back(ParamToCheck(false, "-100"));
	int_validation_list.push_back(ParamToCheck(false, "3.14"));

	url_validation_list.push_back(ParamToCheck(true, "http://domain.com"));
	url_validation_list.push_back(ParamToCheck(true, "http://домен.рф"));
	url_validation_list.push_back(ParamToCheck(true, "http://домен.рф/index.html"));
	url_validation_list.push_back(ParamToCheck(true, "http://localhost"));
	url_validation_list.push_back(ParamToCheck(true, "https://localhost:1500/"));
	url_validation_list.push_back(ParamToCheck(true, "http://domain.com:1500/"));
	url_validation_list.push_back(ParamToCheck(true, "http://192.168.0.1:1500/"));
	url_validation_list.push_back(ParamToCheck(true, "http://127.0.0.1/"));
	url_validation_list.push_back(ParamToCheck(true, "https://domain.com/index.html"));
	url_validation_list.push_back(ParamToCheck(true, "http://do--main.com"));

	url_validation_list.push_back(ParamToCheck(false, "domain.com"));
	url_validation_list.push_back(ParamToCheck(false, "12345"));
	url_validation_list.push_back(ParamToCheck(false, "абвг"));
	url_validation_list.push_back(ParamToCheck(false, "%25~^"));
	url_validation_list.push_back(ParamToCheck(false, " "));
	url_validation_list.push_back(ParamToCheck(false, "http:// domain.com"));
	url_validation_list.push_back(ParamToCheck(false, "шттп://domain.com"));
	url_validation_list.push_back(ParamToCheck(false, BIG_VALUE));
	url_validation_list.push_back(ParamToCheck(false, "http://:8080"));
	url_validation_list.push_back(ParamToCheck(false, "http://host.:8080"));
	url_validation_list.push_back(ParamToCheck(false, "http://host."));
	url_validation_list.push_back(ParamToCheck(false, "http://,"));
	url_validation_list.push_back(ParamToCheck(false, "http://\t"));
	url_validation_list.push_back(ParamToCheck(false, "http://:"));
	url_validation_list.push_back(ParamToCheck(false, "http://%2B"));
	url_validation_list.push_back(ParamToCheck(false, "http://%26"));
	url_validation_list.push_back(ParamToCheck(false, "http://#"));
	url_validation_list.push_back(ParamToCheck(false, "http://%25"));
	url_validation_list.push_back(ParamToCheck(false, "http://$"));
	url_validation_list.push_back(ParamToCheck(false, "http://^"));
	url_validation_list.push_back(ParamToCheck(false, "http://("));
	url_validation_list.push_back(ParamToCheck(false, "http://)"));
	url_validation_list.push_back(ParamToCheck(false, "http://{"));
	url_validation_list.push_back(ParamToCheck(false, "http://}"));
	url_validation_list.push_back(ParamToCheck(false, "http://["));
	url_validation_list.push_back(ParamToCheck(false, "http://]"));
	url_validation_list.push_back(ParamToCheck(false, "http://!"));
	url_validation_list.push_back(ParamToCheck(false, "http://@"));
	url_validation_list.push_back(ParamToCheck(false, "http://~"));
	url_validation_list.push_back(ParamToCheck(false, "http://*"));
	url_validation_list.push_back(ParamToCheck(false, "http://%3F"));
	url_validation_list.push_back(ParamToCheck(false, "http://<"));
	url_validation_list.push_back(ParamToCheck(false, "http://>"));
	url_validation_list.push_back(ParamToCheck(false, "http://="));
	url_validation_list.push_back(ParamToCheck(false, "http://|"));
	url_validation_list.push_back(ParamToCheck(false, "http://\\"));
	url_validation_list.push_back(ParamToCheck(false, "http:///"));
	url_validation_list.push_back(ParamToCheck(false, "http://\""));
	url_validation_list.push_back(ParamToCheck(false, "http://domain..com"));
	url_validation_list.push_back(ParamToCheck(false, "http://имя"));
	url_validation_list.push_back(ParamToCheck(false, "http://localhost-"));
	url_validation_list.push_back(ParamToCheck(false, "http://8localhost"));
	url_validation_list.push_back(ParamToCheck(false, "http://local12345678blablabla1234"));
	url_validation_list.push_back(ParamToCheck(false, "http://l"));
	url_validation_list.push_back(ParamToCheck(false, "http://com."));
	url_validation_list.push_back(ParamToCheck(false, "http://com_polo.com"));

	iprange_v4_validation_list.push_back(ParamToCheck(true, "192.168.0.0/24"));
	iprange_v4_validation_list.push_back(ParamToCheck(true, "192.168.0.0/0"));
	iprange_v4_validation_list.push_back(ParamToCheck(true, "192.168.0.0/32"));
	iprange_v4_validation_list.push_back(ParamToCheck(true, "192.168.0.0/27"));
	iprange_v4_validation_list.push_back(ParamToCheck(true, "192.168.0.0/22"));
	iprange_v4_validation_list.push_back(ParamToCheck(true, "192.168.0.254"));
	iprange_v4_validation_list.push_back(ParamToCheck(true, "192.168.0.10-192.168.0.20"));

	iprange_v4_validation_list.push_back(ParamToCheck(false, " "));
	iprange_v4_validation_list.push_back(ParamToCheck(false, "\t"));
	iprange_v4_validation_list.push_back(ParamToCheck(false, "\n"));
	iprange_v4_validation_list.push_back(ParamToCheck(false, "\r"));
	iprange_v4_validation_list.push_back(ParamToCheck(false, "192.168.0.256"));
	iprange_v4_validation_list.push_back(ParamToCheck(false, "192.168.0"));
	iprange_v4_validation_list.push_back(ParamToCheck(false, "192.168.0.-1"));
	iprange_v4_validation_list.push_back(ParamToCheck(false, "абвг"));
	iprange_v4_validation_list.push_back(ParamToCheck(false, BIG_VALUE));
	iprange_v4_validation_list.push_back(ParamToCheck(false, "~"));
	iprange_v4_validation_list.push_back(ParamToCheck(false, "192.168.0.10 - 192.168.0.20"));
	iprange_v4_validation_list.push_back(ParamToCheck(false, "192.168.0.20-192.168.0.10"));
	iprange_v4_validation_list.push_back(ParamToCheck(false, "192.168.0.20-abcd::123"));
	iprange_v4_validation_list.push_back(ParamToCheck(false, "192.168.0.0/33"));
	iprange_v4_validation_list.push_back(ParamToCheck(false, "192.168.0.0/-1"));

	iprange_v6_validation_list.push_back(ParamToCheck(true, "3011:db8::/64"));
	iprange_v6_validation_list.push_back(ParamToCheck(true, "3011:db8::/0"));
	iprange_v6_validation_list.push_back(ParamToCheck(true, "3011:db8::/128"));
	iprange_v6_validation_list.push_back(ParamToCheck(true, "3011:db8::/30"));
	iprange_v6_validation_list.push_back(ParamToCheck(true, "3011:db8::1"));
	iprange_v6_validation_list.push_back(ParamToCheck(true, "3011:db8::1-3011:db8::10"));

	iprange_v6_validation_list.push_back(ParamToCheck(false, " "));
	iprange_v6_validation_list.push_back(ParamToCheck(false, "\t"));
	iprange_v6_validation_list.push_back(ParamToCheck(false, "\n"));
	iprange_v6_validation_list.push_back(ParamToCheck(false, "\r"));
	iprange_v6_validation_list.push_back(ParamToCheck(false, "3011:db8::1x"));
	iprange_v6_validation_list.push_back(ParamToCheck(false, "3011::db8::"));
	iprange_v6_validation_list.push_back(ParamToCheck(false, "абвг"));
	iprange_v6_validation_list.push_back(ParamToCheck(false, BIG_VALUE));
	iprange_v6_validation_list.push_back(ParamToCheck(false, "~"));
	iprange_v6_validation_list.push_back(ParamToCheck(false, "3011:db8::1 - 3011:db8::10"));
	iprange_v6_validation_list.push_back(ParamToCheck(false, "3011:db8::10-3011:db8::1"));
	iprange_v6_validation_list.push_back(ParamToCheck(false, "192.168.0.20-abcd::123"));
	iprange_v6_validation_list.push_back(ParamToCheck(false, "3011:db8::/129"));
	iprange_v6_validation_list.push_back(ParamToCheck(false, "3011:db8::/-1"));
	iprange_v6_validation_list.push_back(ParamToCheck(false, "3011:db8:"));
	iprange_v6_validation_list.push_back(ParamToCheck(false, "3011:db8::xdc"));
	iprange_v6_validation_list.push_back(ParamToCheck(false, "3011:db8::12345"));
	iprange_v6_validation_list.push_back(ParamToCheck(false, "3011::db8::"));

	net_v4_validation_list.push_back(ParamToCheck(true, "192.168.0.0/24"));
	net_v4_validation_list.push_back(ParamToCheck(true, "192.168.0.0/0"));
	net_v4_validation_list.push_back(ParamToCheck(true, "192.168.0.0/32"));
	net_v4_validation_list.push_back(ParamToCheck(true, "192.168.0.0/27"));
	net_v4_validation_list.push_back(ParamToCheck(true, "192.168.0.0/22"));

	net_v4_validation_list.push_back(ParamToCheck(false, "192.168.0.0"));
	net_v4_validation_list.push_back(ParamToCheck(false, "192.168.0.0/33"));
	net_v4_validation_list.push_back(ParamToCheck(false, "192.168.0.0/-1"));
	net_v4_validation_list.push_back(ParamToCheck(false, "абвг"));
	net_v4_validation_list.push_back(ParamToCheck(false, "abcd"));
	net_v4_validation_list.push_back(ParamToCheck(false, " "));
	net_v4_validation_list.push_back(ParamToCheck(false, ","));
	net_v4_validation_list.push_back(ParamToCheck(false, "\t"));
	net_v4_validation_list.push_back(ParamToCheck(false, ":"));
	net_v4_validation_list.push_back(ParamToCheck(false, "%2B"));
	net_v4_validation_list.push_back(ParamToCheck(false, "%26"));
	net_v4_validation_list.push_back(ParamToCheck(false, "#"));
	net_v4_validation_list.push_back(ParamToCheck(false, "%25"));
	net_v4_validation_list.push_back(ParamToCheck(false, "$"));
	net_v4_validation_list.push_back(ParamToCheck(false, "^"));
	net_v4_validation_list.push_back(ParamToCheck(false, "("));
	net_v4_validation_list.push_back(ParamToCheck(false, ")"));
	net_v4_validation_list.push_back(ParamToCheck(false, "{"));
	net_v4_validation_list.push_back(ParamToCheck(false, "}"));
	net_v4_validation_list.push_back(ParamToCheck(false, "["));
	net_v4_validation_list.push_back(ParamToCheck(false, "]"));
	net_v4_validation_list.push_back(ParamToCheck(false, "!"));
	net_v4_validation_list.push_back(ParamToCheck(false, "@"));
	net_v4_validation_list.push_back(ParamToCheck(false, "~"));
	net_v4_validation_list.push_back(ParamToCheck(false, "*"));
	net_v4_validation_list.push_back(ParamToCheck(false, "%3F"));
	net_v4_validation_list.push_back(ParamToCheck(false, "<"));
	net_v4_validation_list.push_back(ParamToCheck(false, ">"));
	net_v4_validation_list.push_back(ParamToCheck(false, "="));
	net_v4_validation_list.push_back(ParamToCheck(false, "|"));
	net_v4_validation_list.push_back(ParamToCheck(false, "\\"));
	net_v4_validation_list.push_back(ParamToCheck(false, "/"));
	net_v4_validation_list.push_back(ParamToCheck(false, "\""));
	net_v4_validation_list.push_back(ParamToCheck(false, "1"));
	net_v4_validation_list.push_back(ParamToCheck(false, "1.01"));

	net_v6_validation_list.push_back(ParamToCheck(true, "3011:db8::/64"));
	net_v6_validation_list.push_back(ParamToCheck(true, "3011:db8::/0"));
	net_v6_validation_list.push_back(ParamToCheck(true, "3011:db8::/128"));
	net_v6_validation_list.push_back(ParamToCheck(true, "3011:db8::/30"));

	net_v6_validation_list.push_back(ParamToCheck(false, "3011:db8::"));
	net_v6_validation_list.push_back(ParamToCheck(false, "3011:db8::/129"));
	net_v6_validation_list.push_back(ParamToCheck(false, "3011:db8::/-1"));
	net_v6_validation_list.push_back(ParamToCheck(false, "абвг"));
	net_v6_validation_list.push_back(ParamToCheck(false, "abcd"));
	net_v6_validation_list.push_back(ParamToCheck(false, " "));
	net_v6_validation_list.push_back(ParamToCheck(false, ","));
	net_v6_validation_list.push_back(ParamToCheck(false, "\t"));
	net_v6_validation_list.push_back(ParamToCheck(false, ":"));
	net_v6_validation_list.push_back(ParamToCheck(false, "%2B"));
	net_v6_validation_list.push_back(ParamToCheck(false, "%26"));
	net_v6_validation_list.push_back(ParamToCheck(false, "#"));
	net_v6_validation_list.push_back(ParamToCheck(false, "%25"));
	net_v6_validation_list.push_back(ParamToCheck(false, "$"));
	net_v6_validation_list.push_back(ParamToCheck(false, "^"));
	net_v6_validation_list.push_back(ParamToCheck(false, "("));
	net_v6_validation_list.push_back(ParamToCheck(false, ")"));
	net_v6_validation_list.push_back(ParamToCheck(false, "{"));
	net_v6_validation_list.push_back(ParamToCheck(false, "}"));
	net_v6_validation_list.push_back(ParamToCheck(false, "["));
	net_v6_validation_list.push_back(ParamToCheck(false, "]"));
	net_v6_validation_list.push_back(ParamToCheck(false, "!"));
	net_v6_validation_list.push_back(ParamToCheck(false, "@"));
	net_v6_validation_list.push_back(ParamToCheck(false, "~"));
	net_v6_validation_list.push_back(ParamToCheck(false, "*"));
	net_v6_validation_list.push_back(ParamToCheck(false, "%3F"));
	net_v6_validation_list.push_back(ParamToCheck(false, "<"));
	net_v6_validation_list.push_back(ParamToCheck(false, ">"));
	net_v6_validation_list.push_back(ParamToCheck(false, "="));
	net_v6_validation_list.push_back(ParamToCheck(false, "|"));
	net_v6_validation_list.push_back(ParamToCheck(false, "\\"));
	net_v6_validation_list.push_back(ParamToCheck(false, "/"));
	net_v6_validation_list.push_back(ParamToCheck(false, "\""));
	net_v6_validation_list.push_back(ParamToCheck(false, "1"));
	net_v6_validation_list.push_back(ParamToCheck(false, "1.01"));

	ip_v4_validation_list.push_back(ParamToCheck(true, "192.168.0.1"));
	ip_v4_validation_list.push_back(ParamToCheck(true, "192.168.0.0"));
	ip_v4_validation_list.push_back(ParamToCheck(true, "192.168.0.255"));

	ip_v4_validation_list.push_back(ParamToCheck(false, "абвг"));
	ip_v4_validation_list.push_back(ParamToCheck(false, "192.168.0.256"));
	ip_v4_validation_list.push_back(ParamToCheck(false, "192.168.0.-1"));
	ip_v4_validation_list.push_back(ParamToCheck(false, "192.168.0"));
	ip_v4_validation_list.push_back(ParamToCheck(false, " "));
	ip_v4_validation_list.push_back(ParamToCheck(false, ","));
	ip_v4_validation_list.push_back(ParamToCheck(false, "\t"));
	ip_v4_validation_list.push_back(ParamToCheck(false, ":"));
	ip_v4_validation_list.push_back(ParamToCheck(false, "%2B"));
	ip_v4_validation_list.push_back(ParamToCheck(false, "%26"));
	ip_v4_validation_list.push_back(ParamToCheck(false, "#"));
	ip_v4_validation_list.push_back(ParamToCheck(false, "%25"));
	ip_v4_validation_list.push_back(ParamToCheck(false, "$"));
	ip_v4_validation_list.push_back(ParamToCheck(false, "^"));
	ip_v4_validation_list.push_back(ParamToCheck(false, "("));
	ip_v4_validation_list.push_back(ParamToCheck(false, ")"));
	ip_v4_validation_list.push_back(ParamToCheck(false, "{"));
	ip_v4_validation_list.push_back(ParamToCheck(false, "}"));
	ip_v4_validation_list.push_back(ParamToCheck(false, "["));
	ip_v4_validation_list.push_back(ParamToCheck(false, "]"));
	ip_v4_validation_list.push_back(ParamToCheck(false, "!"));
	ip_v4_validation_list.push_back(ParamToCheck(false, "@"));
	ip_v4_validation_list.push_back(ParamToCheck(false, "~"));
	ip_v4_validation_list.push_back(ParamToCheck(false, "*"));
	ip_v4_validation_list.push_back(ParamToCheck(false, "%3F"));
	ip_v4_validation_list.push_back(ParamToCheck(false, "<"));
	ip_v4_validation_list.push_back(ParamToCheck(false, ">"));
	ip_v4_validation_list.push_back(ParamToCheck(false, "="));
	ip_v4_validation_list.push_back(ParamToCheck(false, "|"));
	ip_v4_validation_list.push_back(ParamToCheck(false, "\\"));
	ip_v4_validation_list.push_back(ParamToCheck(false, "/"));
	ip_v4_validation_list.push_back(ParamToCheck(false, "\""));
	ip_v4_validation_list.push_back(ParamToCheck(false, "1"));
	ip_v4_validation_list.push_back(ParamToCheck(false, "1.01"));

	ip_v6_validation_list.push_back(ParamToCheck(true, "3011:db8::"));
	ip_v6_validation_list.push_back(ParamToCheck(true, "3011:db8::ffff"));
	ip_v6_validation_list.push_back(ParamToCheck(true, "3011:db8::1"));
	ip_v6_validation_list.push_back(ParamToCheck(true, "3011:db8::abcd"));
	ip_v6_validation_list.push_back(ParamToCheck(true, "3011:db8:0000:0000:0000:0000:0000:abcd"));

	ip_v6_validation_list.push_back(ParamToCheck(false, "3011:db8:"));
	ip_v6_validation_list.push_back(ParamToCheck(false, "3011:db8::xdc"));
	ip_v6_validation_list.push_back(ParamToCheck(false, "абвг"));
	ip_v6_validation_list.push_back(ParamToCheck(false, "3011:db8::12345"));
	ip_v6_validation_list.push_back(ParamToCheck(false, "3011::db8::"));
	ip_v6_validation_list.push_back(ParamToCheck(false, " "));
	ip_v6_validation_list.push_back(ParamToCheck(false, ","));
	ip_v6_validation_list.push_back(ParamToCheck(false, "\t"));
	ip_v6_validation_list.push_back(ParamToCheck(false, ":"));
	ip_v6_validation_list.push_back(ParamToCheck(false, "%2B"));
	ip_v6_validation_list.push_back(ParamToCheck(false, "%26"));
	ip_v6_validation_list.push_back(ParamToCheck(false, "#"));
	ip_v6_validation_list.push_back(ParamToCheck(false, "%25"));
	ip_v6_validation_list.push_back(ParamToCheck(false, "$"));
	ip_v6_validation_list.push_back(ParamToCheck(false, "^"));
	ip_v6_validation_list.push_back(ParamToCheck(false, "("));
	ip_v6_validation_list.push_back(ParamToCheck(false, ")"));
	ip_v6_validation_list.push_back(ParamToCheck(false, "{"));
	ip_v6_validation_list.push_back(ParamToCheck(false, "}"));
	ip_v6_validation_list.push_back(ParamToCheck(false, "["));
	ip_v6_validation_list.push_back(ParamToCheck(false, "]"));
	ip_v6_validation_list.push_back(ParamToCheck(false, "!"));
	ip_v6_validation_list.push_back(ParamToCheck(false, "@"));
	ip_v6_validation_list.push_back(ParamToCheck(false, "~"));
	ip_v6_validation_list.push_back(ParamToCheck(false, "*"));
	ip_v6_validation_list.push_back(ParamToCheck(false, "%3F"));
	ip_v6_validation_list.push_back(ParamToCheck(false, "<"));
	ip_v6_validation_list.push_back(ParamToCheck(false, ">"));
	ip_v6_validation_list.push_back(ParamToCheck(false, "="));
	ip_v6_validation_list.push_back(ParamToCheck(false, "|"));
	ip_v6_validation_list.push_back(ParamToCheck(false, "\\"));
	ip_v6_validation_list.push_back(ParamToCheck(false, "/"));
	ip_v6_validation_list.push_back(ParamToCheck(false, "\""));
	ip_v6_validation_list.push_back(ParamToCheck(false, "1"));
	ip_v6_validation_list.push_back(ParamToCheck(false, "1.01"));

	domain_validation_list.push_back(ParamToCheck(true, "domain.com"));
	domain_validation_list.push_back(ParamToCheck(true, "domain1.com"));
	domain_validation_list.push_back(ParamToCheck(true, "new-domain.com"));
	domain_validation_list.push_back(ParamToCheck(true, "доменное.name"));
	domain_validation_list.push_back(ParamToCheck(true, "dom--ain.com"));
	domain_validation_list.push_back(ParamToCheck(true, "1.01"));

	domain_validation_list.push_back(ParamToCheck(false, "domain."));
	domain_validation_list.push_back(ParamToCheck(false, ".com"));
	domain_validation_list.push_back(ParamToCheck(false, " "));
	domain_validation_list.push_back(ParamToCheck(false, "domain. com"));
	domain_validation_list.push_back(ParamToCheck(false, "domain-.com"));
	domain_validation_list.push_back(ParamToCheck(false, "-domain.com"));
	domain_validation_list.push_back(ParamToCheck(false, "domain..com"));
	domain_validation_list.push_back(ParamToCheck(false, "new_domain.com"));
	domain_validation_list.push_back(ParamToCheck(false, "123456789012345678901234567890123456789012345678901234567890abcdef.com"));
	domain_validation_list.push_back(ParamToCheck(false, BIG_VALUE ".com"));
	domain_validation_list.push_back(ParamToCheck(false, " "));
	domain_validation_list.push_back(ParamToCheck(false, ","));
	domain_validation_list.push_back(ParamToCheck(false, "\t"));
	domain_validation_list.push_back(ParamToCheck(false, ":"));
	domain_validation_list.push_back(ParamToCheck(false, "%2B"));
	domain_validation_list.push_back(ParamToCheck(false, "%26"));
	domain_validation_list.push_back(ParamToCheck(false, "#"));
	domain_validation_list.push_back(ParamToCheck(false, "%25"));
	domain_validation_list.push_back(ParamToCheck(false, "$"));
	domain_validation_list.push_back(ParamToCheck(false, "^"));
	domain_validation_list.push_back(ParamToCheck(false, "("));
	domain_validation_list.push_back(ParamToCheck(false, ")"));
	domain_validation_list.push_back(ParamToCheck(false, "{"));
	domain_validation_list.push_back(ParamToCheck(false, "}"));
	domain_validation_list.push_back(ParamToCheck(false, "["));
	domain_validation_list.push_back(ParamToCheck(false, "]"));
	domain_validation_list.push_back(ParamToCheck(false, "!"));
	domain_validation_list.push_back(ParamToCheck(false, "@"));
	domain_validation_list.push_back(ParamToCheck(false, "~"));
	domain_validation_list.push_back(ParamToCheck(false, "*"));
	domain_validation_list.push_back(ParamToCheck(false, "%3F"));
	domain_validation_list.push_back(ParamToCheck(false, "<"));
	domain_validation_list.push_back(ParamToCheck(false, ">"));
	domain_validation_list.push_back(ParamToCheck(false, "="));
	domain_validation_list.push_back(ParamToCheck(false, "|"));
	domain_validation_list.push_back(ParamToCheck(false, "\\"));
	domain_validation_list.push_back(ParamToCheck(false, "/"));
	domain_validation_list.push_back(ParamToCheck(false, "\""));
	domain_validation_list.push_back(ParamToCheck(false, "1"));


	subdomain_validation_list.push_back(ParamToCheck(true, "domain.com"));
	subdomain_validation_list.push_back(ParamToCheck(true, "domain1.com"));
	subdomain_validation_list.push_back(ParamToCheck(true, "new-domain.com"));
	subdomain_validation_list.push_back(ParamToCheck(true, "доменное.name"));
	subdomain_validation_list.push_back(ParamToCheck(true, "dom--ain.com"));
	subdomain_validation_list.push_back(ParamToCheck(true, "1.01"));
	subdomain_validation_list.push_back(ParamToCheck(true, "1"));
	subdomain_validation_list.push_back(ParamToCheck(true, "www"));
	subdomain_validation_list.push_back(ParamToCheck(true, "@"));

	domain_validation_list.push_back(ParamToCheck(false, ".com"));
	domain_validation_list.push_back(ParamToCheck(false, "domain. com"));
	domain_validation_list.push_back(ParamToCheck(false, "domain-.com"));
	domain_validation_list.push_back(ParamToCheck(false, "-domain.com"));
	domain_validation_list.push_back(ParamToCheck(false, "domain..com"));
	domain_validation_list.push_back(ParamToCheck(false, "123456789012345678901234567890123456789012345678901234567890abcdef.com"));
	domain_validation_list.push_back(ParamToCheck(false, BIG_VALUE ".com"));
	domain_validation_list.push_back(ParamToCheck(false, ","));
	domain_validation_list.push_back(ParamToCheck(false, "\t"));
	domain_validation_list.push_back(ParamToCheck(false, ":"));
	domain_validation_list.push_back(ParamToCheck(false, "%2B"));
	domain_validation_list.push_back(ParamToCheck(false, "%26"));
	domain_validation_list.push_back(ParamToCheck(false, "#"));
	domain_validation_list.push_back(ParamToCheck(false, "%25"));
	domain_validation_list.push_back(ParamToCheck(false, "$"));
	domain_validation_list.push_back(ParamToCheck(false, "^"));
	domain_validation_list.push_back(ParamToCheck(false, "("));
	domain_validation_list.push_back(ParamToCheck(false, ")"));
	domain_validation_list.push_back(ParamToCheck(false, "{"));
	domain_validation_list.push_back(ParamToCheck(false, "}"));
	domain_validation_list.push_back(ParamToCheck(false, "["));
	domain_validation_list.push_back(ParamToCheck(false, "]"));
	domain_validation_list.push_back(ParamToCheck(false, "!"));
	domain_validation_list.push_back(ParamToCheck(false, "@"));
	domain_validation_list.push_back(ParamToCheck(false, "~"));
	domain_validation_list.push_back(ParamToCheck(false, "*"));
	domain_validation_list.push_back(ParamToCheck(false, "%3F"));
	domain_validation_list.push_back(ParamToCheck(false, "<"));
	domain_validation_list.push_back(ParamToCheck(false, ">"));
	domain_validation_list.push_back(ParamToCheck(false, "="));
	domain_validation_list.push_back(ParamToCheck(false, "|"));
	domain_validation_list.push_back(ParamToCheck(false, "\\"));
	domain_validation_list.push_back(ParamToCheck(false, "/"));
	domain_validation_list.push_back(ParamToCheck(false, "\""));

	path_validation_list.push_back(ParamToCheck(true, "././...home/"));
	path_validation_list.push_back(ParamToCheck(true, "/etc/home_%"));
	path_validation_list.push_back(ParamToCheck(true, "//home/etc/"));
#ifndef WIN32
	path_validation_list.push_back(ParamToCheck(true, "./root"));
#endif
	path_validation_list.push_back(ParamToCheck(true, "."));
	path_validation_list.push_back(ParamToCheck(false, ";<>|&\\/'\t`\0"));
#ifdef WIN32
	//Максимальная длинна пути для винды - 260 (см. define MAX_PATH)
	path_validation_list.push_back(ParamToCheck(true, GenName(248)+"/root_true/"));
	path_validation_list.push_back(ParamToCheck(false, GenName(250)+"/root_false/"));
	path_validation_list.push_back(ParamToCheck(false, "C:\\windows\temp\\"));
#else
	path_validation_list.push_back(ParamToCheck(true, GenName(256)+"/root_true/"));
	path_validation_list.push_back(ParamToCheck(false, GenName(257)+"/root_false/"));
	path_validation_list.push_back(ParamToCheck(false, "./root\\/"));
	path_validation_list.push_back(ParamToCheck(false, "./root||//\0"));
	path_validation_list.push_back(ParamToCheck(false, "\\gome\\etc"));
#endif
	path_validation_list.push_back(ParamToCheck(false, ";"));
	path_validation_list.push_back(ParamToCheck(false, "<"));
	path_validation_list.push_back(ParamToCheck(false, ">"));
	path_validation_list.push_back(ParamToCheck(false, "|"));
	path_validation_list.push_back(ParamToCheck(false, "|&"));
#ifndef WIN32
	path_validation_list.push_back(ParamToCheck(false, "\\"));
	path_validation_list.push_back(ParamToCheck(false, "'"));
	path_validation_list.push_back(ParamToCheck(false, "\t"));
	path_validation_list.push_back(ParamToCheck(false, "`"));
	path_validation_list.push_back(ParamToCheck(false, "\0"));
#endif
}

PingWaiter::PingWaiter( const string& ip, bool expect_no_fail ) {
	Cmd = "ping";
	if( ip.find( ':' ) != string::npos )
		Cmd += "6";
	Cmd += " -c 1 " + ip;
	NoFail = expect_no_fail;
}

bool PingWaiter::operator()() {
	return NoFail ? !mgr_proc::Execute( Cmd ).Result() : mgr_proc::Execute( Cmd ).Result();
}

WindowsWaiter::WindowsWaiter( const string& ip, bool power_on ) {
	Cmd = "nc -z " + ip + " 3389";
	PowerOn = power_on;
}

bool WindowsWaiter::operator()() {
	int result = mgr_proc::Execute( Cmd ).Result();
	return PowerOn ? !result : result;
}

void test_functions::CheckErrorMessage( const mgr_err::Error& e, const string& expected_message ) {
	const string actual_msg = e.xml().GetNode( "/doc/error/msg" ).Str();
	if( expected_message != actual_msg )
		ADD_FAILURE() << "Incorrect error message.\nExpected: \"" + expected_message
				+ "\"\nActual: \"" + actual_msg + "\"";
}

string test_functions::GetManagerVersion( const string& manager_name, bool full_size ) {
	string format = full_size ? " -V" : " -v";
	string cmd;
#ifndef WIN32
	cmd = mgr_file::GetCurrentDir() + "/bin/core " + manager_name + format;
#else
	cmd = mgr_file::GetCurrentDir() + "\core.exe " + manager_name + format;
#endif
	mgr_proc::Execute command(cmd, mgr_proc::Execute::efOut);
	std::stringstream output;
	output << command.Out().rdbuf();
	return output.str();
}

bool test_functions::CheckFileCountInDir( const string& path, size_t file_count ) {
	std::vector<string> files;
	mgr_file::Dir dir(path);
	while (dir.Read()) {
		files.push_back(dir.FullName());
	}
	return files.size() == file_count;
}

int test_functions::FindElemInDir( const string& path, const string& name, SearchType search ) {
		int counter = 0;
		mgr_file::Dir dir(path);
		while (dir.Read()) {
			if( (search & test_functions::stDir) && dir.Info(true).IsDir() ) {
						if( dir.name().find(name) != string::npos ) ++counter;
			}
			if( (search & test_functions::stFile) && dir.Info(true).IsFile() ) {
						if( dir.name().find(name) != string::npos ) ++counter;
			}
			if( (search & test_functions::stSymlink) && dir.Info(true).IsLink() ) {
						if( dir.name().find(name) != string::npos ) ++counter;
			}
		}
		return counter;
}

string test_functions::error_what( const mgr_err::Error& e ) {
	return "Type: '" + e.type() + "' Object: '" + e.object() + "' Value: '" + e.value() + "'";
}

std::string test_functions::xml2json(const string& source) {
	mgr_client::Local l("testmgr", "test");
	auto res = l.Query("func=xmltojson&sok=ok&xml=" + str::url::Encode(source));
	return res.xml.GetNode("/doc/json").Str();
}

std::string test_functions::ExecSSH(const std::string& ipaddr, const std::string& cmd) {
	std::string result;

	if (ipaddr != GetManagerUrl(test_functions::URL_IP)) {
		mgr_rpc::SSHkey ssh(ipaddr, "root", mgr_file::ConcatPath(mgr_file::GetCurrentDir(), "etc/.ssh/master_id"));
		ssh.Execute(cmd);
		result = ssh.Str();
	} else {
		test_mgrobject::LocalQuery local;
		local.SystemOut(cmd, result);
	}

	return result;
}

void test_functions::GenStat(StringList& paramList, const string param, const mgr_date::DateTime& begin, const mgr_date::DateTime& end,
								   const int period, const string& path, const long int& min, const long int& max, bool add_hour) {
	mgr_date::DateTime periodBegin((begin / period) * period);
	mgr_date::DateTime periodEnd((end / period) * period);
	string statFile = "";
	string oldHour = "";
	std::string header;

	header = str::Append("#begin", str::Join<StringList>(paramList, "\t"), "\t") + "\n";

	mgr_file::Attrs attrs(0644);
	mgr_file::MkDir(path, &attrs);

	while(periodBegin < periodEnd) {
		StringList statList;
		std::string day = str::Replace(periodBegin.AsDate(), "-", "");
		std::string hour = str::Str(periodBegin.hour());
		while(hour.size() < 2)
			hour = "0" + hour;

		if(add_hour && oldHour != day + hour)
			statFile =  mgr_file::ConcatPath(path, day + hour + ".stat");
		else
			statFile =  mgr_file::ConcatPath(path, day + ".stat");
		if(!mgr_file::Exists(statFile))
			mgr_file::Create(statFile, header, &attrs);

		ForEachI(paramList, paramp) {
		  if(*paramp == param)
			  statList.push_back(str::Str(rand() % max + min));
		  else
			  statList.push_back("0");
		}

		mgr_file::Append(statFile, str::Str(periodBegin) + '\t' + str::Join<StringList>(statList, "\t") + "\n");

		periodBegin.AddSeconds(period);
	}

}

void test_functions::MakeStatDay(const string& hourPath, const string& dayPath) {
	mgr_file::Dir dir(hourPath);
	StringList hourFileList;
	mgr_file::Attrs attrs(0644);
	while (dir.Read()) {
		const mgr_file::Info& info = dir.Info();
		if (info.IsFile()) {
			hourFileList.push_back(dir.FullName());
		}
	}
	mgr_file::MkDir(dayPath, &attrs);

	ForEachI(hourFileList, hour_file) {
		string statDayFile = "";
		string period;
		string out = mgr_file::ReadE(*hour_file);
		string header = GetWord(out, "\n");
		StringVector statVector;
		str::Split(out, statVector, "\n");
		ForEachI(statVector, stat) {
			period = GetWord(*stat, "\t");
			mgr_date::DateTime periodBegin(str::Int(period));
			statDayFile = mgr_file::ConcatPath(dayPath,  str::Replace(periodBegin.AsDate(), "-", "") + ".stat");
			if(!mgr_file::Exists(statDayFile)) {
				mgr_file::Create(statDayFile, header + "\n", &attrs);
			}
			mgr_date::DateTime dayBegin(periodBegin.AsDate());
			mgr_file::Append(statDayFile, str::Str(dayBegin) + "\t" + *stat + "\n");

		}

	}
}

void test_functions::GetRawStat(StringList& outList, const std::string& param, const std::string& path,  const mgr_date::DateTime& begin, const mgr_date::DateTime& end, bool addDate) {
	std::string out, header;
	StringList statFileList;
	mgr_file::Dir dir(path);

	int uTimeBegin = begin;
	int uTimeEnd = end;

	while (dir.Read()) {
		const mgr_file::Info& info = dir.Info();
		if (info.IsFile()) {
			if(dir.name().find(".stat") != string::npos)
				statFileList.push_back(dir.FullName());
		}
	}

	ForEachI(statFileList, stat_file) {
		uint col = 0;
		out = mgr_file::ReadE(*stat_file);
		header = str::GetWord(out, "\n");
		while(!header.empty()) {
			if(str::GetWord(header, "\t") == param)
				break;
			col++;
		}
		while(!out.empty()) {
			StringVector data;
			str::Split(str::GetWord(out, "\n"), data, "\t");
			if(col < data.size()) {
				if(str::Int(data[0]) >= uTimeBegin && str::Int(data[0]) <= uTimeEnd) {
					if(addDate)
						outList.push_back(str::Append(data[0], data[col], "\t"));
					else
						outList.push_back(data[col]);
				}
			}
			else {
				ADD_FAILURE() << "Parameter \"" + param + "\" is not found in the file " + *stat_file;
				break;
			}
		}
	}
	outList.sort();
}

void test_functions::GetRawStatInt64(std::list<int64_t>& outList, const std::string& param, const std::string& path,  const mgr_date::DateTime& begin, const mgr_date::DateTime& end) {
	StringList statList;
	GetRawStat(statList, param, path, begin, end, false);
	ForEachI(statList, stat) {
			outList.push_back(str::Int64(*stat));
	}
	outList.sort();
}

void test_functions::RawStatExport(const string& ipaddr, const string& localPath, const string& remotePath, const string& key) {
	test_mgrobject::LocalQuery local;
	string statFiles = mgr_file::ConcatPath(localPath, "*.stat");
	string cmd = "scp -vCq -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null -i " + key + " " + statFiles + " " + ROOTNAME + "@" + ipaddr + ":" + remotePath;
	int result;
	string out;
	mgr_rpc::SSHkey ssh(ipaddr, ROOTNAME, key);
	ssh.Execute("mkdir -p " + remotePath);
	ssh.Result();
	result = local.SystemOut(cmd, out);
	if(result > 0) {
		ADD_FAILURE() << "An error occurred during the statistics export: " + out;
		throw mgr_err::System("RawStatExport", result);
	}
}

void test_functions::RawStatImport(const string& ipaddr, const string& localPath, const string& remotePath, const string& key) {
	test_mgrobject::LocalQuery local;
	string statFiles = mgr_file::ConcatPath(remotePath, "*.stat");
	string cmd = "scp -vCq -i " + key + " " + ROOTNAME + "@" + ipaddr + ":" + statFiles + " " + localPath;
	int result;
	mgr_file::Attrs attrs(0644);
	mgr_file::MkDir(localPath, &attrs);
	result = local.System(cmd);
	if(result > 0) {
		ADD_FAILURE() << "An error occurred during the statistics import";
		throw mgr_err::System("RawStatImport", result);
	}
}

std::string test_functions::GetDefaultAdmin(const std::string name)
{
	static string admin_name = name;
	if (admin_name.empty())
		throw mgr_err::Error("Call std::string GetDefaultAdmin(const std::string name) before Environment::SetUp");
	return admin_name;
}

string test_functions::DecryptFieldValue(string value, const string pkey_path) {
	static mgr_crypto::RsaPrivateKey pkey = mgr_crypto::pem_private::Decode(mgr_file::ReadE(pkey_path));
	string res;
	while (!value.empty()) {
		string block = str::GetWord(value, CRYPTED_FIELD_DELIMETER);
		if (!block.empty()) {
			res += mgr_crypto::crypt::Decrypt(pkey, str::base64::Decode(block));
		}
	}
	return res;
}
