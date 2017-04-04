#include "mgrobject.h"
#include "functions.h"
#include <mgr/mgrstr.h>
#include <mgr/mgrlog.h>
#include <mgr/mgrtest.h>
MODULE("mgrobject");

using namespace test_mgrobject;
using namespace mgr_err;
using namespace mgr_xml;
using namespace std;
using namespace str;

//Server::Server(const string& username, const string& password, const string& mgr) {
Server::Server(const string& username, const string& password, const string& mgr, const string& host, const string& ip, const string& root_passwd, const mgr_env::OsTypeEnum& os_type) {
	m_Username = username;
	m_Password = password;
	m_Manager = mgr;
	m_Host = host;
	m_IP = ip;
	m_RootPasswd = root_passwd;
	m_os_type = os_type;

}

string Server::GetIP() const {
	string ip = m_IP;
	if (m_IP.empty()) {
		StringList ip_list;
		mgr_net::GetIpList(ip_list);
		ForEachI(ip_list, ip_item) {
			mgr_net::Ip ip_addr(*ip_item);
			if (!ip_addr.IsSpecial()) {
				ip = *ip_item;
				break;
			}
		}
	}
	return ip;
}

string Server::GetURL() const {
	string ip = GetIP();
	if (test::Ip6(ip))
		ip = "[" + ip + "]";
	return "https://" + ip + ":1500/" + m_Manager;
}

string Server::GetUsername() const {
	return m_Username;
}

void Server::SetUsrName(const string & name) {
	m_Username = name;
}

string Server::GetPassword() const {
	return m_Password;
}

void Server::SetPassWord(const string & pass) {
	m_Password = pass;
}

string Server::GetRootPasswd() const {
	return m_RootPasswd;
}

/**
 * @brief	Добавляет публичный ключ панели на сервер в /root/.ssh/authorized_keys
 *       	Авторизуется с паролем root'a
 *  	a.mitroshin@ispsystem.com
 */
void Server::AddAuthorizedKey(const std::string& key) const {
#ifndef WIN32
	string publicKey;
	if(key.empty())
		publicKey = test_functions::GetPublicKey(); // Публичный ключ панели
	else
		publicKey = key;
	mgr_rpc::SSHpass ssh(GetIP(), "root", m_RootPasswd);

	ssh.Execute("cat /root/.ssh/authorized_keys | grep \"" + str::Trim(publicKey) + "\"");
	if (ssh.Str().empty()) {
		ssh.Execute("mkdir -p /root/.ssh");
		ssh.Execute("echo \"" + publicKey + "\" >> /root/.ssh/authorized_keys");
	}
#endif
}

/**
 * @brief	Удаляет публичный ключ панели с сервера из /root/.ssh/authorized_keys. Авторизуется с паролем root'a.
 *  	a.mitroshin@ispsystem.com
 */
void Server::RemoveAuthorizedKey(const std::string& key) const {
#ifndef WIN32
	string publicKey;
	if(key.empty())
		publicKey = test_functions::GetPublicKey(); // Публичный ключ панели
	else
		publicKey = key;
	mgr_rpc::SSHpass ssh(m_IP, "root", m_RootPasswd);
	ssh.Execute("sed -i -e '\\@" + publicKey + "@d' /root/.ssh/authorized_keys");
#endif
}

/**
 * @brief	Получает публичный ключ сервера из /root/.ssh/authorized_keys. Авторизуется с паролем root'a.
 *  	a.mitroshin@ispsystem.com
 */
string Server::GetPublicKey(const std::string& path) const {
#ifndef WIN32
	string keyPath;
	if(path.empty())
		keyPath = mgr_file::GetCurrentDir() + "/etc/ssh_id_rsa.pub"; // Публичный ключ панели
	else
		keyPath = path;
	mgr_rpc::SSHpass ssh(GetIP(), "root", m_RootPasswd);

	ssh.Execute("cat " + keyPath);
	return ssh.Str();
#endif
}

mgr_env::OsTypeEnum Server::GetOSType() const {
	return m_os_type;
}

StringSet MgrObject::m_called_functions;
StringSet MgrObject::m_all_functions;
StringSet MgrObject::m_skip_functions;

mgr_thread::SafeLock MgrObject::m_called_functions_lock;
mgr_thread::SafeLock MgrObject::m_all_functions_lock;
mgr_thread::SafeLock MgrObject::m_skip_functions_lock;

StringSet& MgrObject::CalledFunctions() {
	return m_called_functions;
}

StringSet& MgrObject::AllFunctions() {
	return m_all_functions;
}

StringSet& MgrObject::SkipFunctions() {
	return m_skip_functions;
}

MgrObject::MgrObject(IQuery* provider) :  provider_(provider), create(false), su("") {
	props["out"] = "xml";
	props["lang"] = "ru";
}

MgrObject::MgrObject(): create(false), su("") {
	provider_ = shared_ptr<IQuery>(new LocalQuery);
	props["out"] = "xml";
}

string MgrObject::QueryHook(const string& query) const {
	string result = query;

	StringVector query_args;
	str::Split(query, query_args, "&");

	ForEachI(query_args, arg) {
		StringVector key_val;
		str::Split(*arg, key_val, "=");
		if (key_val[0] == "func") {
			mgr_thread::SafeSection lock(MgrObject::m_called_functions_lock);
			CalledFunctions().insert(key_val[1]);
		}
	}

	if (query.find("&out=") == string::npos && query.find("out=") != 0) {
		result += "&out=" + props["out"];
	}

	if (query.find("&lang=") == string::npos && query.find("lang=") != 0) {
		result += "&lang=" + props["lang"];
	}

	return result + (su.empty() ? "" : "&su=" + su) + (forceSu ? "&forcesu=on" : "");
}

mgr_xml::Xml MgrObject::callQuery(const string& query) const {
	//1. Определяем является ли запрос зпросом на редактирование:
	bool is_editing_object =
		(query.find("sok=ok") != string::npos) &&
		(query.find(".edit") != string::npos);
	//2. Определяем является ли запрос запросом на создание
	bool is_creating_object = props[key].empty() && is_editing_object;

	//3. Выполняем запрос и запоминаем ответ:
	auto xml = provider_->Query(QueryHook(query));

	if (is_creating_object && mgr_xml::XPath(xml, "//id").empty()) {
		//если при создании объекта ответ не содержит тэг <id>, бросаем исключение:
		throw mgr_err::Error("MgrObject", "can't create object of type \"" + getFuncName() + "\": returned empty id");
	} else if (is_editing_object && mgr_xml::XPath(xml, "//ok").empty()) {
		//если при редактировании ответ не содержит тэг <ok>, выводим сообщение:
		printf("[LocalQuery]: %s (Seems, that manager died during processing)\n", query.c_str());
		fflush(stdout);
	}
	return xml;
}

void MgrObject::Query(const string& query) const {
	callQuery(query);
}

mgr_xml::Xml MgrObject::XmlQuery(const string& query) const {
	return callQuery(query);
}

XPath MgrObject::QueryXpath(const string& query, const string& xpath_expr) const {
	mgr_xml::Xml xmlDoc = XmlQuery(query);
	mgr_xml::XPath xp(xmlDoc, xpath_expr);
	return xp;
}

void MgrObject::setSU(const string& su, bool forceSu) {
	this->su = su;
	this->forceSu = forceSu;
}

string MgrObject::getSU() {
	return this->su;
}

void MgrObject::Read() {
	ReadBody();
}

MgrObject::~MgrObject() {
	//delete provider_;
}

string MgrObject::BuildQuery() {
	string query;

	for (mapIterator i = props.begin(); i != props.end(); ++i)
		if (!i->first.empty())
			query += "&" + i->first + "=" + i->second;
	return query;
}

string& MgrObject::operator[](const string& field) {
	return GetProp( field );
}

const string& MgrObject::operator[](const string& field) const {
	return GetProp( field );
}

bool MgrObject::HasProp(const string& field) {
	return props.find(field) != props.end();
}

string& MgrObject::GetProp(const string& field) {
	if (props.find(field) == props.end()) throw Error("MgrObject", "No such field [" + field + "] in object [" + func + "]");

	return props[field];
}

const string& MgrObject::GetProp(const string& field) const {
	if (props.find(field) == props.end()) throw Error("MgrObject", "No such field [" + field + "] in object [" + func + "]");

	return props[field];
}

void MgrObject::SetProp(const string &field, const string &value)
{
	props[field] = value;
}

void MgrObject::SetCreated() {
	create = true;
}

void MgrObject::SetNotCreated() {
	create = false;
}

////////////////
// MgrElem
////////////////

Xml MgrElem::Create(const string& name, bool suffix, bool gen_name, const string& funclist, const string& createfunc) {
	CheckCreate();
	string funcl = funclist.empty() ? func: funclist;
	string tmpName = ((props["name"].empty()) ? name : props["name"]);

	if (tmpName.empty() && gen_name)
		props["name"] = test_functions::GenName(6);
	else if (suffix)
		props["name"] = tmpName + test_functions::GenName(6);
	else
		props["name"] = tmpName;

	string pth = "/doc/elem/" + key;
	string plid = getPlid();

	string elid = ((!plid.empty()) ? "&elid=" + plid : "");
#ifdef BILLMGR
if (plid.find("/")!= string::npos){
	elid = ((!plid.empty()) ? "&elid=" + str::RGetWord(plid, '/') + "&plid=" + str::GetWord(plid, '/') : "");
}
#endif

	Xml xmlDoc;

	// Снимем слепок списка объектов до создания нового
	xmlDoc = XmlQuery("func=" + funcl + elid);


	XPath itemsBefore(xmlDoc, pth);

	// Создаём собственно объект в манагере
	Xml xmlElem = createfunc.empty() ? CreateBody("", false) : CreateBody(createfunc, "", false);

	// Снимем слепок списка объектов после создания
	xmlDoc = XmlQuery("func=" + funcl + elid);
	XPath itemsAfter(xmlDoc, pth);

	// Сравним списки в цикле, найдём добавенный элемент и запомним значение его ключевого свойства
	string keyVal = "";
	ForEachR(itemsAfter, itA) {
		bool fnd = false;
		ForEachR(itemsBefore, itB) {
			fnd = (fnd || (itB->Str() == itA->Str()));
			if (fnd) break;
		}
		if (!fnd) {
			keyVal = itA->Str();
			break;
		}
	}

	// Ругаемся, если значение уникального поля пустое (в т.ч. если элемент не найден).
	if (keyVal.empty())
		throw Error("MgrElem", "Empty key value (" + key + ") returned after create [" + func + "] -- element not found?");

	props[key] = keyVal;
	create = true;
	return xmlElem;
}

void MgrElem::Open(const string& key_value) {
	create = true;
	props[key] = key_value;
	Read();
}

/*
 * s.belous:
 * Возврат списка объектов выпилен, ибо оно должно быть в Create().
 * Теперь метод void, и должен перегружаться соответсвующим образом.
 */
Xml MgrElem::CreateBody(const string& name, bool suffix) {
	string query = "func=" + func + ".edit";
	query = query + BuildQuery();
	return callQuery(query);
}

/*
 * s.belous:
 * Возврат списка объектов выпилен, ибо оно должно быть в Create().
 * Теперь метод void, и должен перегружаться соответсвующим образом.
 */
Xml MgrElem::CreateBody(const string& createfunc, const string& name, bool suffix) {
	//Если создание идет через визард и функция создания не .edit, пишем название функции полностью
	string query = "func=" + createfunc;
	query = query + BuildQuery();
	return callQuery(query);
}


string MgrElem::ToString() {
	string query = "\n============" + func + "=============\n";

	for (mapIterator i = props.begin(); i != props.end(); ++i)
		query += "\t[" + i->first + "]=\"" + i->second + "\"\n";

	query += "==============================\n";

	return query;
}

void MgrElem::ReadBody() {
	mgr_xml::Xml ReadedXml;
	string query = "func=" + func + ".edit&elid=" + props[key];
	if (props.count("plid")) query += "&plid=" + props["plid"];
	query += "&out=devel";

	ReadedXml = XmlQuery(query);
	FillProps(ReadedXml);
}

void MgrElem::UpdateBody() {
	string query = "func=" + func + ".edit" + "&elid=" + props[key] + BuildQuery();
	Query(query);
}

void MgrElem::Delete() {
	if (!create)
		return;

	DeleteBody();

	if (Exists())
		throw Error("MgrElem", "Object [" + func + "] with key=[" + props[key] + "] wasn't removed");

	create = false;
}

void MgrElem::DeleteBody() {
	string query = "func=" + func + ".delete&elid=" + props[key];
	if (props.count("plid")) query += "&plid=" + props["plid"];

	Query(query);
}

bool MgrElem::operator!= (const MgrElem& toComapare) {
	return this->props != toComapare.props;
}

bool MgrElem::Exists() {
	//That kind of check should work on all "normal" objects. But in child class better add some system check.
	if( props[key].empty() )
		return false;
	try {
		this->Read();
		return true;
	} catch (...) {
		return false;
	}
}

string MgrElem::getName() const {
	return props[key];
}


void MgrElem::ReadSetValue( const string& value_name ) {
	const string old_out = props["out"];
	props["out"] = "xml";
	string query = "func=" + func + ".edit&elid=" + props[key] + "&sv_field=" + value_name + "&" + value_name + "=" + props[value_name];

	if (props.count("plid")) query += "&plid=" + props["plid"];

	Xml result;
	try {
		result = XmlQuery( query );
	} catch( const Error& e ) {
		props["out"] = old_out;
		throw e;
	}
	XPath xp( result, "/doc/*[name() != 'doc' and name() != 'tparams']" );
	for( XPath::iterator iter = xp.begin(); iter != xp.end(); ++iter ) {
		StringMap::iterator prop = props.find( iter->Name() );
		if( prop == props.end() )
			throw Error( "UGLY TODO. Request contains nodes not from props map. This behaviour was expected. REFACTOR!!!" );

		prop->second = iter->Str();
	}
}

////////////////
// MgrForm
////////////////
void MgrForm::ReadBody() {
	mgr_xml::Xml ReadedXml;
	string query = "func=" + func;
	if( !key.empty() )
		query += "&" + key + "=" + props[key];
	query += "&out=devel";
	ReadedXml = XmlQuery( query );
	FillProps(ReadedXml);
}

void MgrForm::FillProps(mgr_xml::Xml ObjXml) {
	ForEachI(ObjXml.GetNodes("/doc/metadata/form//field/*"), e) {
		const string field_name = e->GetProp("name");
		if (!field_name.empty()) {
			auto value_node = ObjXml.GetNode("/doc/" + field_name);
			if (value_node) {
					props[field_name] = value_node.Str();
			}
		}
	}
	XPath objxpath(ObjXml, "/doc/*");
	XmlNode tmp_node;
	for (size_t i = 0; i < objxpath.size(); i++) {
		tmp_node = objxpath[i];
		mapIterator prop = props.find(tmp_node.Name());
		if (prop != props.end())
			prop->second = tmp_node.Str();
	}
}

void MgrForm::Update() {
	UpdateBody();
}

void MgrForm::UpdateBody() {
	string query = "func=" + func + BuildQuery();
	Query(query);
}

void MgrForm::getFieldsNames(const string& id, const string& lang, StringVector& names) const {
	mgr_xml::Xml xmlDoc = id.empty() ? XmlQuery("func=" + func + "&out=devel&lang=" + lang) :
						  XmlQuery("func=" + func + ".edit" + "&elid=" + id + "&out=devel&lang=" + lang);
	mgr_xml::XPath messages(xmlDoc, "//messages/msg");

	mgr_xml::XPath fields_names_xpath = mgr_xml::XPath(xmlDoc, "//field[count(desc)=0]/@name | //field/desc/@name | //field/textdata/@name");

	StringVector fields_names(fields_names_xpath.begin(), fields_names_xpath.end());

	ForEachI(fields_names, field) {
		ForEachI(messages, msg) {
			if (*field == msg->GetProp("name"))
				names.push_back(msg->Str());
		}
	}
}

void MgrForm::getSelectValsViaQuery( const string& select_field, const string& query, StringMap& ValueMap ) const {
	mgr_xml::XPath xp = QueryXpath( query, "/doc/slist[@name='" + select_field + "']/val" );
	for( size_t i = 0; i < xp.size(); ++i )
		ValueMap[xp[i].GetProp( "key" )] = xp[i].Str();
}

////////////////
// MgrList
////////////////

void MgrList::ReadBody() {
	body_xml = XmlQuery("func=" + func + BuildQuery());
}

void MgrList::getSortedColumn(const string& column_name, bool desc, StringVector& sorted_column, bool applyFilter) const {
	string query = "func=" + func + "&p_order=" + (desc ? "desc" : "asc") + "&p_sort=" + column_name + "&out=devel" + getPlidStr();
	query += (applyFilter ? "&filter=yes" : "&filter=off");
		XPath xp = QueryXpath(query, "/doc/elem/" + column_name);

	for (size_t i = 0; i < xp.size(); ++i)
		sorted_column.push_back(xp[i].Str());
}

mgr_xml::XPath MgrList::ListXPath(const string& xpath, bool applyFilter) const {
	string query = "func=" + func + (applyFilter ? "&filter=yes" : "&filter=off") + "&out=" + props["out"] + getPlidStr();
	return QueryXpath(query, xpath);
}

bool MgrList::hasElementWithNode(const string& nodeName, const string& nodeValue) {
	string query = "func=" + func + "&out=" + props["out"] + getPlidStr();
	string xpath_expr = "/doc/elem[" + nodeName + "=\"" + nodeValue + "\"]";
	return QueryXpath(query, xpath_expr).size() > 0;
}

StringVector MgrList::getColumnValuesFor(const string& elem_key, const string& column_name) const {
	StringVector result;
	const string plid_string = props.find("elid") != props.end() ? "&elid=" + props["elid"] : "";
	string query = "func=" + func + "&out=devel" + plid_string;
	string xpath_expr_main = "/doc/metadata/coldata/col[@name='" + column_name + "']";
	string xpath_expr_props = xpath_expr_main + "/prop/@name";

	mgr_xml::XPath xp = QueryXpath(query, xpath_expr_props);
	query = "func=" + func + "&out=xml" + plid_string;
	if (xp.size()) {
		for (size_t i = 0; i < xp.size(); ++i) {
			string prop_xpath = "/doc/elem[" + key + "='" + elem_key + "']/" + xp[i].Str();
			if (QueryXpath(query, prop_xpath).size())
				result.push_back(xp[i].Str());
		}
	} else {
		string column_xpath = "/doc/elem[" + key + "='" + elem_key + "']/" + column_name;
		mgr_xml::XPath xp_column = QueryXpath(query, column_xpath);
		if (xp_column.size())
			result.push_back(xp_column[0].Str());
	}
	return result;
}

string MgrList::getColumnStringValueFor(const string& elem_key, const string& column_name) const {
	return Join(getColumnValuesFor(elem_key, column_name), ", ");
}

size_t MgrList::getElementCount(const string& elemName) const {
	return QueryXpath("func=" + func + "&out=" + props["out"] + getPlidStr(), "//" + elemName).size();
}

string MgrList::getButtonParam(const string& btnName, const string& btnParam) const {
	string query = "func=" + func + "&out=devel";
	mgr_xml::XPath xp = QueryXpath(query, "//toolbtn[@name='" + btnName + "']/@" + btnParam);
	if (xp.size()) {
		return xp[0];
	}
	return "";
}

void MgrList::getButtons( vector<StringVector>& btnGroups, const string& lang ) const {
	string query = "func=" + func + "&out=devel&lang=" + lang + getPlidStr();
	mgr_xml::Xml xml = XmlQuery( query );
	mgr_xml::XPath xp( xml, "//toolbar/toolgrp" );
	if( !xp.size() )
		return;
	ForEachI( xp, node ) {
		btnGroups.push_back( StringVector() );
		mgr_xml::XPath butxp(xml, "//toolgrp[@name='" + node->GetProp("name") + "']/toolbtn");
		ForEachI(butxp, it) {
			string btnName;
			mgr_xml::XPath hintxp( xml, "//messages/msg[@name='short_" + it->GetProp( "name" ) + "']" );
			if( hintxp.size() == 1 )
				btnName = hintxp[0].Str();
				btnGroups.back().push_back( btnName );
		}
	}
}

string MgrList::getMetadataParam(const string& param) const {
	string query = "func=" + func + "&out=devel";
	mgr_xml::XPath xp = QueryXpath(query, "//metadata/@" + param);
	if (xp.size()) {
		return xp[0];
	}
	return "";
}

bool MgrList::isColumnVisible(const string& colName) const {
	return QueryXpath("func=" + func + "&out=devel", "//coldata/col[@name='" + colName + "']").size() > 0;
}

size_t MgrList::getColumnsCount() const {
	return QueryXpath("func=" + func + "&out=devel" + getPlidStr(), "//coldata/col" ).size();
}

string MgrList::getElementValue(const string& elemName) const {
	string query = "func=" + func + (props["lang"].empty() ? "" : ("&lang=" + props["lang"]));
	mgr_xml::XPath xp = QueryXpath(query, "//" + elemName);
	if (xp.size()) {
		return xp[0];
	} else {
		return "";
	}
}

void MgrList::getColumnNames(const string& lang, StringVector& names) const {
	mgr_xml::Xml xmlDoc = XmlQuery("func=" + func + "&out=devel&lang=" + lang + getPlidStr() );

	mgr_xml::XPath messages(xmlDoc, "//messages/msg");

	mgr_xml::XPath coll_names_xpath(xmlDoc, "//coldata/col/@name");
	StringVector coll_names(coll_names_xpath.begin(), coll_names_xpath.end());

	ForEachI(coll_names, col) {
		ForEachI(messages, msg) {
			if (*col == msg->GetProp("name"))
				names.push_back(msg->Str());
		}
	}
}

void MgrList::GetList(StringList& outList, const StringList& fields, const StringMap& filter) const {
	std::string st, fld;
	bool logical;
	mgr_xml::XPath items = GetFilteredItems(filter);
	mgr_xml::XmlNode node;
	ForEachI(items, item) {
		st = "";
		ForEachI(fields, field) {
			fld = *field;
			logical = (*fld.rbegin() == '/');
			if (logical) fld.erase(fld.size() - 1);
			node = item->FindNode(fld);
			st += ((logical) ? ((node) ? "TRUE" : "FALSE") : node.Str()) + "\t";
		}
		st.erase(st.size() - 1);
		outList.push_back(st);
	}
}

string MgrList::getFirstID(const StringMap& filter) const {
	StringList fields, outList;
	fields.push_back(key);
	GetList(outList, fields, filter);
	if (outList.size())
		return outList.front();
	return "";
}

int MgrList::ItemsNumber(const StringMap& filter) const {
	mgr_xml::XPath items = GetFilteredItems(filter);
	return items.size();
}

bool MgrList::HasItems(const StringMap& fieldsMap) const {
	return (ItemsNumber(fieldsMap) > 0);
}

void MgrList::DeleteGroup(const StringList &elidList) const {
	string elids;
	ForEachI(elidList, iter) {
		elids+= *iter + ", ";
	}
	if (!elids.empty()) {
		elids.resize(elids.size()-2);
	}
	string query = "func=" + func + ".delete" + "&elid=" + elids;
	Query(query);
}

mgr_xml::XPath MgrList::GetFilteredItems(const StringMap& filter) const {
	std::string xp = "/doc/elem";

	if (filter.size() > 0) {
		string fstr, key, val;
		ForEachI(filter, fld) {
			key = fld->first;
			val = fld->second;
			if (*key.rbegin() == '/')
				fstr += "(" + string((val != "TRUE") ? "not" : "") + "(" + key.erase(key.size() - 1) + "))and";
			else
				fstr += "(" + key + "=\"" + val + "\")and";
		}
		fstr.erase(fstr.size() - 3);
		xp += "[" + fstr + "]";
	}
	MODULE("test");
	LogInfo("xpath: %s", xp.c_str());
	return ListXPath(xp, false);
}

//RequestByWizard
RequestByWizard::RequestByWizard( MgrObject& obj, const string& initial_step ) : Object( obj ), CurStep( initial_step ) {}

bool RequestByWizard::NextStep() {
	string next_step = NextStepName();
	string query = "func=" + CurStep + Object.BuildQuery();
	if( !next_step.empty() )
		query += "&snext=ok";
	step_xml = Object.XmlQuery( query );
	CurStep = step_xml.GetNode( "/doc" ).GetProp( "func" );
	bool is_end = next_step.empty();
	if( is_end ) {
				//Предполагаем, что у MgrForm key будет пустым. Таким образом, предотвратим выставление
				//для объекта MgrForm: флага create и параметра Object[key]. Выставление этих параметров
				//для объекта MgrForm может создать ошибки при Update() и Read()
				if( !Object.key.empty() ) {
						Object.create = true;
						Object[Object.key] = step_xml.GetNode( "/doc/id" ).Str();
				}
	} else if( next_step != CurStep ) {
		throw mgr_err::Error( "Unexpected next step was returned by panel." );
	}
	return !is_end;
}

string& RequestByWizard::operator[]( const string& field ) {
	return Object[field];
}

void RequestByWizard::setSU( const string& admin ) {
	Object.setSU( admin );
}

string RequestByWizard::NextStepName() {
	return "";
}

string RequestByWizard::GetCurStep() const {
	return CurStep;
}

mgr_xml::Xml& RequestByWizard::GetStepXml() {
	return step_xml;
}


mgr_xml::Xml AutoWizard::GoAllSteps(const bool need_autofill_props) {
	//запрашиваем содержимое шага, смотрим какие есть поля и кнопки.
	StringMap passed_fields;
	string elid;
	try {
		elid = Object.HasProp("elid") ? Object.GetProp("elid") : "";
	} catch (...)
	{
	}
	string current_step = Object.XmlQuery("func=" + m_wizard_name + "&elid=" + elid + "&out=devel").GetRoot().GetProp("func");
	int step_count(0);
	mgr_xml::Xml result_xml;
	while (!current_step.empty()) {
		if (step_count > 100)
			throw mgr_err::Error("too many steps in wizard " + m_wizard_name);
		string query = "func=" + current_step + "&elid=" + elid;
		ForEachI(passed_fields, e)
				query += "&" + e->first + "=" + str::url::Encode(e->second);
		mgr_xml::Xml devel_xml = Object.XmlQuery(query + "&out=devel");
		auto finish_button =  devel_xml.GetNode("/doc/metadata/form/buttons/button[@type='ok']");
		if (finish_button) {
			//есть кнопка финиш - это последний шаг
			query += "&clicked_button=" + finish_button.GetProp("name");
		}
		else {
			//смотрим какие есть кнопки. Если нет кнопки next - значит нужно выбрать кнопку из списка
			auto next_button = devel_xml.GetNode("/doc/metadata/form/buttons/button[@type='next']");
			if (!next_button) {
				//пока рассчитываем только на один список
				auto list =  devel_xml.GetNode("/doc/metadata/form//list");
				if (!list)
					throw mgr_err::Error("Not find button in wizard step " + current_step);
				string list_key = list.GetProp("key");
				if (list_key.empty())
					throw mgr_err::Error("Not find key in list " + list.GetProp("name"));
				//берем значение для ключа у объекта
				string key_value = Object.GetProp(list_key);
				//можно задать какая кнопка была нажата. ИМЯ_ЛИСТА_button
				auto clicked_button = Object.props.find(list.GetProp("name") + "_button");
				if (clicked_button != Object.props.end()) {
					query += "&clicked_button=" + clicked_button->second;
				}
				passed_fields[list_key] = key_value;
				query += "&" + list_key + "=" + key_value;
			}
			query += "&snext=ok";
		}
		//Заполняем поля с этого шага
		ForEachI(devel_xml.GetNodes("/doc/metadata/form//field/*"), e) {
			string field_name = e->GetProp("name");
			if (passed_fields.find(field_name) != passed_fields.end())
				continue;
			auto field_value = Object.props.find(field_name);
			//получение значения поля с текущего шага
			if (need_autofill_props){
				if (field_value->second.compare("")==0){
					string prop_value = devel_xml.GetNode("/doc/" + field_name).Str();
					if (prop_value.compare("") != 0){
						Object.props[field_name] = prop_value;
						field_value = Object.props.find(field_name);
					}
				}
			}
			if (field_value != Object.props.end()) {
				query += "&" + field_name + "=" + str::url::Encode(field_value->second);
				passed_fields[field_name] = field_value->second;
			}
		}
		query += "&sok=ok";
		result_xml = Object.XmlQuery(query + "&out=devel");

		if (finish_button) {
			if( !Object.key.empty() ) {
					Object.create = true;
					if (!result_xml.GetNode( "/doc/id" ).Str().empty())
						Object[Object.key] = result_xml.GetNode( "/doc/id" ).Str();
			}
			current_step.clear();
		} else {
			current_step = result_xml.GetRoot().GetProp("func");
		}
		++step_count;
	}
	return result_xml;
}
