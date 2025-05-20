#pragma once

#define ARRAY_SIZE(array)										(sizeof(array) / sizeof(array[0]))

#include "core_config.h"


// ---------------------------------------------------------------------------------------------------------------------------------------
// implisite casting
// ---------------------------------------------------------------------------------------------------------------------------------------

#define IMPLISITE_CAST(type, field)								operator type& () { return field; };

#define IMPLISITE_CAST_CONST(type, field)						operator const type& () const { return field; };

#define IMPLISITE_CASTS(type, field)							IMPLISITE_CAST(type, field); IMPLISITE_CAST_CONST(type, field);

// ---------------------------------------------------------------------------------------------------------------------------------------
// Constructors
// ---------------------------------------------------------------------------------------------------------------------------------------

#define DELETE_COPY_CONSTRUCTOR(name)							public:																	\
																	name(const name&) = delete;											\
																	name& operator=(const name&) = delete

#define DELETE_MOVE_CONSTRUCTOR(name)							public:																	\
																	name(name&&) = delete;												\
																	name& operator=(name&&) = delete

#define DELETE_COPY_MOVE_CONSTRUCTOR(name)						public:																	\
																	name(const name&) = delete;											\
																	name& operator=(const name&) = delete;								\
																	name(name&&) = delete;												\
																	name& operator=(name&&) = delete

#define DEFAULT_CONSTRUCTORS(name)								public:																	\
																	name() = default;													\
																	name(const name&) = default;											
																	//name(name&&) = default;

#define DEFAULT_COPY_CONSTRUCTOR(name)							public:																	\
																	name(const name& other) = default;

/*
name(const name&) = default;
name& operator=(const name&) = default;
name(name&&) = default;
name& operator=(name&&) = default;
*/

// ---------------------------------------------------------------------------------------------------------------------------------------
// getters && setters
// ---------------------------------------------------------------------------------------------------------------------------------------

// ------------------------------------------- getters -------------------------------------------
#define DEFAULT_GETTER(type, name)								type get_##name() { return m_##name;}
#define DEFAULT_GETTER_REF(type, name)							type& get_##name##_ref() { return m_##name;}
#define DEFAULT_GETTER_C(type, name)							type get_##name() const { return m_##name;}
#define DEFAULT_GETTER_POINTER(type, name)						type* get_##name##_pointer() { return &m_##name;}

#define DEFAULT_GETTERS(type, name)								DEFAULT_GETTER(type, name)												\
																DEFAULT_GETTER_REF(type, name)											\
																DEFAULT_GETTER_POINTER(type, name)

#define DEFAULT_GETTERS_C(type, name)							DEFAULT_GETTER_C(type, name)											\
																DEFAULT_GETTER_POINTER(type, name)

#define GETTER(type, func_name, var_name)						type get_##func_name() { return var_name;}
#define GETTER_C(type, func_name, var_name)						const type get_##func_name() const { return var_name;}


// ------------------------------------------- setters -------------------------------------------
#define DEFAULT_SETTER(type, name)								void set_##name(type name) { m_##name = name;}
#define SETTER(type, func_name, var_name)						void set_##func_name(type value) { var_name = value;}


// ------------------------------------------- both togetter -------------------------------------------
#define DEFAULT_GETTER_SETTER(type, name)						DEFAULT_GETTER(type, name)												\
																DEFAULT_SETTER(type, name)

#define DEFAULT_GETTER_SETTER_C(type, name)						DEFAULT_GETTER_C(type, name)											\
																DEFAULT_SETTER(type, name)

#define DEFAULT_GETTER_SETTER_ALL(type, name)					DEFAULT_SETTER(type, name)												\
																DEFAULT_GETTER(type, name)												\
																DEFAULT_GETTER_POINTER(type, name)

// ------------------------------------------- function for header -------------------------------------------
#define GETTER_FUNC(type, name)									type get_##name();
#define GETTER_REF_FUNC(type, name)								type& get_##name##_ref();
#define GETTER_C_FUNC(type, name)								type get_##name() const;
#define GETTER_POINTER_FUNC(type, name)							type* get_##name##_pointer();

#define SETTER_FUNC(type, name)									void set_##name(type name);

#define GETTER_SETTER_FUNC(type, name)							GETTER_FUNC(type, name)													\
																SETTER_FUNC(type, name)

// ------------------------------------------- function implementation -------------------------------------------
#define GETTER_FUNC_IMPL(type, name)							type name{};																\
																type get_##name() { return name; }

#define GETTER_FUNC_IMPL2(type, name, value)					type name = value;															\
																type get_##name() { return name; }

#define SETTER_FUNC_IMPL(type, name)							void set_##name(const type new_name) { name = new_name; }

// --------- refrence ---------
#define GETTER_REF_FUNC_IMPL(type, name)						type name{};																\
																type& get_##name##_ref() { return name; }

#define GETTER_REF_FUNC_IMPL2(type, name, value)				type name = value;															\
																type& get_##name##_ref() { return name; }
// --------- const ---------
#define GETTER_C_FUNC_IMPL(type, name)							type name{};																\
																type get_##name() const;

// --------- pointer ---------
#define GETTER_POINTER_FUNC_IMPL(type, name)					type name{};																\
																type* get_##name##_pointer() { return &name; }

#define GETTER_POINTER_FUNC_IMPL2(type, name, value)			type name = value;															\
																type* get_##name##_pointer() { return &name; }


// ---------------------------------------------------------------------------------------------------------------------------------------
// bit manipulation
// ---------------------------------------------------------------------------------------------------------------------------------------

#define BIT(x)													(1 << x)

#define BIND_FUNKTION(x)										std::bind(&x, this, std::placeholders::_1)

// ---------------------------------------------------------------------------------------------------------------------------------------
// string
// ---------------------------------------------------------------------------------------------------------------------------------------

#define CONSOLE_LIST_BEGIN										" " << (char)(200) << " "
