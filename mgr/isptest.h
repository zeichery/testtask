#ifndef ISPTEST_H
#define ISPTEST_H

#include <string>
#include <algorithm>
#include <mgr/mgrclient.h>
#include <mgr/mgrerr.h>
#include <gtest/gtest.h>
#include "mgrobject.h"

#define IS_AVAILABLE(expression, why)\
{\
	if (false == (expression)) { RecordProperty("skipped", why); std::cout << "\tTest was skipped: " << (why) << std::endl; return; }\
}

/* GLOBAL */
extern ::testing::Environment* environment;
	// Указатель на объект класса Environment, представляющего собой глобальную среду выполнения тестов

#endif //ISPTEST_H
