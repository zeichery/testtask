#ifndef PROVIDER_H
#define PROVIDER_H
#include "../../mgr/mgrobject.h"
#include "utils.h"
namespace billmgr {

class DefaultAdminQuery : public test_mgrobject::LocalQuery {
public:
	virtual mgr_xml::Xml Query(const string& query);
};

template<typename Server>
class DefaultRemoteQuery : public test_mgrobject::RemoteQuery <Server>
{
public:
	virtual mgr_xml::Xml Query(const string& query) {
	   string query_with_su = query;
	   if (query_with_su.find("&su=") == string::npos) {
		   query_with_su += "&su=" + str::url::Encode(functions::GetRemoteDefaultAdmin());
	   }
	   return test_mgrobject::RemoteQuery<Server>::Query(query_with_su);
	}
};


}
#endif // PROVIDER_H

