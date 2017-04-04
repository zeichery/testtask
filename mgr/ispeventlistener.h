#ifndef ISPEVENTLISTENER_H
#define ISPEVENTLISTENER_H

#include <gtest/gtest.h>
#include <mgr/mgrerr.h>
#include <iostream>

#include "mgrobject.h"
#include "isptest.h"

class IspEventListener : public testing::TestEventListener {
public:
	IspEventListener(TestEventListener* listener) : m_listener(listener) {}
	~IspEventListener() {
		if (nullptr != m_listener) delete m_listener;
	}

	void OnTestProgramStart(const testing::UnitTest& unit_test) {
		m_listener->OnTestProgramStart(unit_test);
	}
	void OnEnvironmentsSetUpStart(const testing::UnitTest& unit_test) {
		m_listener->OnEnvironmentsSetUpStart(unit_test);
	}
	void OnEnvironmentsSetUpEnd(const testing::UnitTest& unit_test) {
		m_listener->OnEnvironmentsSetUpEnd(unit_test);
	}
	void OnTestCaseStart(const testing::TestCase& test_case) {
		m_listener->OnTestCaseStart(test_case);
	}
	void OnTestPartResult(const testing::TestPartResult& test_part_result) {
		if (test_part_result.failed())
			std::cout<<"Time of crashing = "<<mgr_date::DateTime().AsDateTime()<<std::endl;
		m_listener->OnTestPartResult(test_part_result);
	}
	void OnTestCaseEnd(const testing::TestCase& test_case) {
		m_listener->OnTestCaseEnd(test_case);
	}
	void OnEnvironmentsTearDownStart(const testing::UnitTest& unit_test) {
		m_listener->OnEnvironmentsTearDownStart(unit_test);
	}
	void OnEnvironmentsTearDownEnd(const testing::UnitTest& unit_test) {
		m_listener->OnEnvironmentsTearDownEnd(unit_test);
	}
	void OnTestProgramEnd(const testing::UnitTest& unit_test) {
		m_listener->OnTestProgramEnd(unit_test);
	}
	void OnTestIterationStart(const testing::UnitTest& unit_test, int iteration) {
		m_listener->OnTestIterationStart(unit_test, iteration);
	}
	void OnTestIterationEnd(const testing ::UnitTest& unit_test, int iteration) {
		m_listener->OnTestIterationEnd(unit_test, iteration);
	}

	void OnTestStart(const testing::TestInfo& test_info) {
		m_listener->OnTestStart(test_info);
	}
	void OnTestEnd(const testing::TestInfo& test_info) {
		m_listener->OnTestEnd(test_info);
	}
private:
	TestEventListener* m_listener;
};

#endif
