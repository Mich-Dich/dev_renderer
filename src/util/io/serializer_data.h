#pragma once

#include "util/data_structures/string_manipulation.h"

#define TEST_NAME_CONVERTION(variable)		util::extract_vaiable_name(#variable)
#define SERIALIZE_KEY_VALUE(variable)		serialize_key_value(util::extract_vaiable_name(#variable), variable);

// @brief used with [serializer] to shorten the serializer::yaml::entry call
#define KEY_VALUE(var)						util::extract_vaiable_name(#var), var

namespace GLT::serializer {

	enum class option {

		save_to_file,
		load_from_file,
	};
	
}
