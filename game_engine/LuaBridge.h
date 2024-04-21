// https://github.com/vinniefalco/LuaBridge
// Copyright 2020, Dmitry Tarakanov
// Copyright 2012, Vinnie Falco <vinnie.falco@gmail.com>
// Copyright 2007, Nathan Reed
// SPDX-License-Identifier: MIT

#pragma once

// All #include dependencies are listed here
// instead of in the individual header files.
//

#define LUABRIDGE_MAJOR_VERSION 2
#define LUABRIDGE_MINOR_VERSION 7
#define LUABRIDGE_VERSION 207

#ifndef LUA_VERSION_NUM
#error "Lua headers must be included prior to LuaBridge ones"
#endif

#include "CFunctions.h"
#include "ClassInfo.h"
#include "Constructor.h"
#include "FuncTraits.h"
#include "Iterator.h"
#include "LuaException.h"
#include "LuaHelpers.h"
#include "LuaRef.h"
#include "Namespace.h"
#include "Security.h"
#include "Stack.h"
#include "TypeList.h"
#include "TypeTraits.h"
#include "Userdata.h"

