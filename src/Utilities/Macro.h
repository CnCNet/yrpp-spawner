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

/**
* Use when some function argument is unneeded.
* Currently that happens when faking __thiscall functions
* via __fastcall ones (fastcall function accepts args via
* ECX, EDX, then stack, thiscall via ECX for this and stack
* for rest, so second arg in fastcall-faked function would need to be discarded).
*/
typedef size_t discard_t;

#define NAKED __declspec(naked)

#pragma region Patch Macros

#pragma region Patch Structs
#pragma pack(push, 1)
#pragma warning(push)
#pragma warning( disable : 4324)

#define LJMP_OPCODE 0xE9
#define CALL_OPCODE 0xE8
#define NOP_OPCODE  0x90

typedef void JumpType;

typedef JumpType LJMP;
struct _LJMP
{
	byte opcode;
	DWORD pointer;

	constexpr
		_LJMP(DWORD offset, DWORD pointer) :
		opcode(LJMP_OPCODE),
		pointer(pointer - offset - 5)
	{ };
};

typedef JumpType CALL;
struct _CALL
{
	byte opcode;
	DWORD pointer;

	constexpr
		_CALL(DWORD offset, DWORD pointer) :
		opcode(CALL_OPCODE),
		pointer(pointer - offset - 5)
	{ };
};

typedef JumpType CALL6;
struct _CALL6
{
	byte opcode;
	DWORD pointer;
	byte nop;

	constexpr
		_CALL6(DWORD offset, DWORD pointer) :
		opcode(CALL_OPCODE),
		pointer(pointer - offset - 5),
		nop(NOP_OPCODE)
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

typedef JumpType OFFSET;
typedef _VTABLE _OFFSET;

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

#define DEFINE_PATCH_TYPED(type, offset, ...)                     \
	namespace STATIC_PATCH##offset                                \
	{                                                             \
		const type data[] = {__VA_ARGS__};                        \
	}                                                             \
	_ALLOCATE_STATIC_PATCH(offset, sizeof(data), data);

#define DEFINE_PATCH(offset, ...)                                 \
	DEFINE_PATCH_TYPED(byte, offset, __VA_ARGS__);

#define DEFINE_JUMP(jumpType, offset, pointer)                    \
	namespace STATIC_PATCH##offset                                \
	{                                                             \
		const _##jumpType data (offset, pointer);                 \
	}                                                             \
	_ALLOCATE_STATIC_PATCH(offset, sizeof(data), &data);

#define DEFINE_NAKED_HOOK(hook, funcname)                         \
	void funcname();                                              \
	DEFINE_JUMP(LJMP, hook, GET_OFFSET(funcname))                 \
	void NAKED funcname()

#pragma endregion Static Patch

#pragma region Dynamic Patch
#define _ALLOCATE_DYNAMIC_PATCH(name, offset, size, data)         \
	namespace DYNAMIC_PATCH_##name                                \
	{                                                             \
		Patch patch = {offset, size, (byte*)data};                \
	}                                                             \
	Patch* const name = &DYNAMIC_PATCH_##name::patch;

#define DEFINE_DYNAMIC_PATCH_TYPED(type, name, offset, ...)       \
	namespace DYNAMIC_PATCH_##name                                \
	{                                                             \
		const type data[] = {__VA_ARGS__};                        \
	}                                                             \
	_ALLOCATE_DYNAMIC_PATCH(name, offset, sizeof(data), data);

#define DEFINE_DYNAMIC_PATCH(name, offset, ...)                   \
	DEFINE_DYNAMIC_PATCH_TYPED(byte, name, offset, __VA_ARGS__)

#define DEFINE_DYNAMIC_JUMP(jumpType, name, offset, pointer)      \
	namespace DYNAMIC_PATCH_##name                                \
	{                                                             \
		const _##jumpType data (offset, pointer);                 \
	}                                                             \
	_ALLOCATE_DYNAMIC_PATCH(name, offset, sizeof(data), &data);
#pragma endregion Dynamic Patch

#pragma region Thiscall Patch
#define _GET_FUNCTION_ADDRESS(function, getterName)               \
	static constexpr __forceinline uintptr_t getterName()         \
	{                                                             \
		uintptr_t addr;                                           \
		{ _asm mov eax, function }                                \
		{ _asm mov addr, eax }                                    \
		return addr;                                              \
	}

#define DEFINE_FUNCTION_JUMP(jumpType, offset, function)          \
	namespace NAMESPACE_THISCALL_JUMP##offset                     \
	{                                                             \
		_GET_FUNCTION_ADDRESS(function, GetAddr)                  \
		DEFINE_JUMP(jumpType, offset, GetAddr())                  \
	}
#pragma endregion

#pragma endregion Macros
#pragma endregion Patch Macros
