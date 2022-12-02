/**
*  yrpp-spawner
*
*  Copyright(C) 2022-present CnCNet
*
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once
#include <Helpers/Macro.h>
#include <ASMMacros.h>
#include "Patch.h"

template<typename T>
__forceinline T& Make_Global(const uintptr_t address)
{
	return *reinterpret_cast<T*>(address);
}

template<typename T>
__forceinline T* Make_Pointer(const uintptr_t address)
{
	return reinterpret_cast<T*>(address);
}

#define NAKED __declspec(naked)

#pragma region Patch Macros

#pragma region Patch Structs
#pragma pack(push, 1)
#pragma warning(push)
#pragma warning( disable : 4324)

#define LJMP_LETTER 0xE9
#define CALL_LETTER 0xE8
#define NOP_LETTER  0x90

typedef void JumpType;

typedef JumpType LJMP;
struct _LJMP
{
	byte command;
	DWORD pointer;

	constexpr
		_LJMP(DWORD offset, DWORD pointer) :
		command(LJMP_LETTER),
		pointer(pointer - offset - 5)
	{ };
};

typedef JumpType CALL;
struct _CALL
{
	byte command;
	DWORD pointer;

	constexpr
		_CALL(DWORD offset, DWORD pointer) :
		command(CALL_LETTER),
		pointer(pointer - offset - 5)
	{ };
};

typedef JumpType CALL6;
struct _CALL6
{
	byte command;
	DWORD pointer;
	byte nop;

	constexpr
		_CALL6(DWORD offset, DWORD pointer) :
		command(CALL_LETTER),
		pointer(pointer - offset - 5),
		nop(NOP_LETTER)
	{ };
};

typedef JumpType VTABLE;
struct _VTABLE
{
	DWORD pointer;

	constexpr
		_VTABLE(DWORD offset, DWORD pointer) :
		pointer(pointer)
	{ };
};

#pragma warning(pop)
#pragma pack(pop)
#pragma endregion Patch Structs

#pragma region Macros
#define GET_OFFSET(pointer) reinterpret_cast<DWORD>(pointer)

#pragma region Static Patch
#define _ALLOCATE_STATIC_PATCH(offset, size, data)                \
	namespace STATIC_PATCH##offset                                \
	{                                                             \
		__declspec(allocate(PATCH_SECTION_NAME))                  \
		Patch patch = {offset, size, (byte*)data};                \
	}

#define DEFINE_PATCH(offset, ...)                                 \
	namespace STATIC_PATCH##offset                                \
	{                                                             \
		const byte data[] = {__VA_ARGS__};                        \
	}                                                             \
	_ALLOCATE_STATIC_PATCH(offset, sizeof(data), data);

#define DEFINE_JUMP(jumpType, offset, pointer)                    \
	namespace STATIC_PATCH##offset                                \
	{                                                             \
		const _##jumpType data (offset, pointer);                 \
	}                                                             \
	_ALLOCATE_STATIC_PATCH(offset, sizeof(data), &data);
#pragma endregion Static Patch

#pragma region Dynamic Patch
#define _ALLOCATE_DYNAMIC_PATCH(name, offset, size, data)         \
	namespace DYNAMIC_PATCH_##name                                \
	{                                                             \
		Patch patch = {offset, size, (byte*)data};                \
	}                                                             \
	Patch* const name = &DYNAMIC_PATCH_##name::patch;

#define DEFINE_DYNAMIC_PATCH(name, offset, ...)                   \
	namespace DYNAMIC_PATCH_##name                                \
	{                                                             \
		const byte data[] = {__VA_ARGS__};                        \
	}                                                             \
	_ALLOCATE_DYNAMIC_PATCH(name, offset, sizeof(data), data);

#define DEFINE_DYNAMIC_JUMP(jumpType, name, offset, pointer)      \
	namespace DYNAMIC_PATCH_##name                                \
	{                                                             \
		const _##jumpType data (offset, pointer);                 \
	}                                                             \
	_ALLOCATE_DYNAMIC_PATCH(name, offset, sizeof(data), &data);
#pragma endregion Dynamic Patch

#pragma endregion Macros
#pragma endregion Patch Macros
