#include "ispeventlistener.h"
#include <ispbin.h>
#include <mgr/mgrlog.h>
#include <mgr/mgrargs.h>
MODULE("mgrtest");
#include <mgr/mgrxml.h>
#include <algorithm>
#include "functions.h"
#include <gtest/gtest.h>
#ifdef WIN32
	#include <winlibcntl.h>

	USECORELIB("libmgr");
	USELIB("gtest");
#endif

#include "environment_registrator.h"

::testing::Environment* environment;

using namespace test_mgrobject;
using namespace std;

mgr_args::Args args;

void fillAllActions() {
	LocalQuery query;
	if(mgr_file::Exists(mgr_file::ConcatPath(mgr_file::GetCurrentDir(), "lib/" MGR ".so"))) {
		auto act_xml = query.Query("func=actionlist&out=xml");
		mgr_xml::XPath act_xpath(act_xml, "//elem/action");

		ForEachI(act_xpath, act) {
			mgr_thread::SafeSection lock(MgrObject::m_all_functions_lock);
			MgrObject::AllFunctions().insert(act->Str());
		}
	}
}

void fillSkipedActions() {
	mgr_thread::SafeSection lock(MgrObject::m_skip_functions_lock);
	MgrObject::SkipFunctions().insert("*");
}

void printCoverage() {
	StringSet result;
	mgr_thread::SafeSection lock1(MgrObject::m_all_functions_lock);
	mgr_thread::SafeSection lock2(MgrObject::m_skip_functions_lock);
	mgr_thread::SafeSection lock3(MgrObject::m_called_functions_lock);

	std::set_difference(
	    MgrObject::AllFunctions().begin(), MgrObject::AllFunctions().end(),
	    MgrObject::SkipFunctions().begin(), MgrObject::SkipFunctions().end(),
	    std::inserter(result, result.end()));
	size_t not_skiped_size = result.size();

	result.clear();

	std::set_intersection(
	    MgrObject::AllFunctions().begin(), MgrObject::AllFunctions().end(),
	    MgrObject::CalledFunctions().begin(), MgrObject::CalledFunctions().end(),
	    std::inserter(result, result.end()));
	size_t called_size = result.size();

	cout << "Total functions: " << MgrObject::AllFunctions().size() << endl;
	cout << "Covered functions: " << called_size << endl;
	cout << "Excluded: " << MgrObject::SkipFunctions().size() << endl;
	cout.setf(ios::fixed, ios::floatfield);
	cout << "Covering: " << setprecision(2) << static_cast<double>(called_size) / not_skiped_size * 100 << endl;
}

bool CheckMgrConnection() {
	bool res(true);
	EXPECT_NO_THROW(
				try {
					test_mgrobject::LocalQuery().Query("func=whoami");
				} catch (...) {
					res = false;
					throw;
				}
				) << "No connection to " MGR;
	return res;
}

#ifndef WIN32
int ISP_MAIN(int argc, char** argv) {
#else
int main(int argc, char** argv) {
#endif
	mgr_log::Init("mgrtest", mgr_log::L_DEBUG);
	testing::InitGoogleTest(&argc, argv);

	testing::TestEventListeners& listeners = testing::UnitTest::GetInstance()->listeners();

	testing::TestEventListener* remListener = listeners.Release(listeners.default_result_printer());
	listeners.Append(new IspEventListener(remListener));

	//Регистрируем параметры командной строки
	args.AddOption("vmmgr", 'v', "VMmanager option")
		.AddSuboption("free_node_ip", 'f', "Free node IP").SetParam();

	//Разбираем параметры оставшиеся после GoogleTest
	if (args.Parse(argc, (const char**)argv))
		args.Usage();

	if (!CheckMgrConnection())
		return EXIT_FAILURE;
	fillAllActions();
	fillSkipedActions();
#ifndef WIN32	
	// Инициализируем среды
	for( size_t i = 0; i < GetEnvRegistrators().size(); ++i )
		environment = GetEnvRegistrators()[i]->Register();
#endif
	int result = RUN_ALL_TESTS();

	printCoverage();

	return result;
}
