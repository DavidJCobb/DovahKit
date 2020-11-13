#pragma once
#include <functional>
#include <QString>

extern QString get_default_object_name(uint32_t signature);
extern QString get_default_object_description(uint32_t signature);

extern bool for_each_default_object(std::function<bool(uint32_t s, const QString& n, const QString& d)>);