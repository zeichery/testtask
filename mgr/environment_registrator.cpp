#include "environment_registrator.h"

EnvironmentRegistrator::EnvironmentRegistrator() {
	GetEnvRegistrators().push_back( this );
}

std::vector<EnvironmentRegistrator*>& GetEnvRegistrators() {
	static std::vector<EnvironmentRegistrator*> registrators;
	return registrators;
}

