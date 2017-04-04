#ifndef REMOTEQUERY_SERVERS_H
#define REMOTEQUERY_SERVERS_H

#include "mgrobject.h"
#include "functions.h"
#include <mgr/mgrnet.h>

class IPmgrLocal : public test_mgrobject::Server {
public:
	static std::string username;
	static std::string password;

	IPmgrLocal() : Server(username, password, "ipmgr"){}
	static void PrepareAdmin() {
		try {
			mgr_client::Local("ipmgr", "coretest").Query("func=user.edit&name=" + username
														 + "&passwd=" + password + "&sok=ok&level=29");
		} catch (const mgr_err::Error& e) {
			mgr_err::Error("Unable to create admin for integration. Trace: " + std::string(e.what()));
		}
	}
	static void CleanUp() {
		try {
			mgr_client::Local("ipmgr", "coretest").Query("func=user.delete&elid=" + username);
		} catch (const mgr_err::Error& e) {
			mgr_err::Error("Unable to delete admin created for integration. Trace: " + std::string(e.what()));
		}
	}
};

std::string IPmgrLocal::username = test_functions::GenName(5) + "_integr";
std::string IPmgrLocal::password = test_functions::GenName(10);

/**
 * @brief	Класс запросов к локальному VMmanager'у как к удалённому серверу (по HTTP,
 *          а не через mgrctl). Используется исключительно для аутентификации пользователя.
 *  	s.belous@ispsystem.com
 */
class VMMgrLocalAuth : public test_mgrobject::Server {
public:
	
	std::string GetURL() const;
};

std::string VMMgrLocalAuth::GetURL() const {
	return test_functions::GetManagerUrl();
}

#endif

