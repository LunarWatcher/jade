#pragma once

#include <functional> // NOLINT

#define JADE_CALLBACK_BINDING(funcName) std::bind(funcName, server, std::placeholders::_1, std::placeholders::_2)
