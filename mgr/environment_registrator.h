#ifndef TEST_ENV_REG_H
#define TEST_ENV_REG_H

#include <gtest/gtest.h>
#include <vector>


class EnvironmentRegistrator {
public:
	EnvironmentRegistrator();
	virtual ::testing::Environment* Register() = 0;
};

std::vector<EnvironmentRegistrator*>& GetEnvRegistrators();
#endif
