#include "utils.h"
#include "provider.h"


namespace billmgr {
mgr_xml::Xml DefaultAdminQuery::Query(const string& query) {
	string query_with_su = query;
	if (query_with_su.find("&su=") == string::npos) {
		query_with_su += "&su=" + str::url::Encode(functions::GetDefaultAdmin());
	}

	return test_mgrobject::LocalQuery::Query(query_with_su);
}



}

