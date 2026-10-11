/**
*  yrpp-spawner
*
*  Copyright(C) 2026-present CnCNet
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

#include <winsock2.h>
#include <iphlpapi.h>
#include <memory>
#include <cstdint>

#include <UDPInterfaceClass.h>
#include <Memory.h>
#include <Utilities/Debug.h>
#include <Utilities/Macro.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

void AddAddress(DynamicVectorClass<void*>& addresses, DWORD address)
{
	auto value = GameCreate<DWORD>(address);
	if (!addresses.AddItem(value))
		GameDelete(value);
}

void LocalInterfaces()
{
	constexpr unsigned IPv4AddressBits = 32;
	const auto pUDPInterface = UDPInterfaceClass::Instance;
	constexpr ULONG InitialAdapterBufferSize = 15 * 1024;

	ULONG bufferSize = InitialAdapterBufferSize;
	const ULONG flags = GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER | GAA_FLAG_SKIP_FRIENDLY_NAME;
	std::unique_ptr<char[]> buffer;
	ULONG result = ERROR_BUFFER_OVERFLOW;

	// Adapter changes can increase the required buffer between calls.
	for (int attempt = 0; attempt < 3 && result == ERROR_BUFFER_OVERFLOW; ++attempt)
	{
		buffer = std::make_unique_for_overwrite<char[]>(bufferSize);
		auto adapters = reinterpret_cast<IP_ADAPTER_ADDRESSES*>(buffer.get());
		result = GetAdaptersAddresses(AF_INET, flags, nullptr, adapters, &bufferSize);
	}

	if (result != NO_ERROR)
	{
		Debug::Log("GetAdaptersAddresses failed - error code %lu", result);
		return;
	}

	// The game owns and frees the separately allocated four-byte addresses.
	for (auto address : pUDPInterface->LocalAddresses)
		GameDelete(static_cast<DWORD*>(address));

	pUDPInterface->LocalAddresses.Count = 0;

	auto adapters = reinterpret_cast<IP_ADAPTER_ADDRESSES*>(buffer.get());
	for (auto adapter = adapters; adapter; adapter = adapter->Next)
	{
		if (adapter->IfType == IF_TYPE_SOFTWARE_LOOPBACK)
			continue;

		for (auto entry = adapter->FirstUnicastAddress; entry; entry = entry->Next)
		{
			auto socketAddress = entry->Address.lpSockaddr;
			if (!socketAddress || socketAddress->sa_family != AF_INET
				|| entry->Address.iSockaddrLength < static_cast<int>(sizeof(sockaddr_in)))
				continue;

			const auto address = reinterpret_cast<const sockaddr_in*>(socketAddress)->sin_addr.s_addr;
			if (!address)
				continue;

			AddAddress(pUDPInterface->LocalAddresses, address);

			const auto prefix = entry->OnLinkPrefixLength;
			if (prefix > IPv4AddressBits)
				continue;

			// Address and broadcast use network byte order.
			const auto hostBits = prefix == IPv4AddressBits ? 0u : UINT32_MAX >> prefix;
			const DWORD broadcast = address | htonl(hostBits);
			bool exists = false;
			for (auto value : pUDPInterface->BroadcastAddresses)
				exists |= *static_cast<DWORD*>(value) == broadcast;

			// Each broadcast has a port in the game's fixed 256-entry array.
			const auto index = pUDPInterface->BroadcastAddresses.Count;
			if (!exists && index < static_cast<int>(std::size(pUDPInterface->BroadcastPorts)))
			{
				AddAddress(pUDPInterface->BroadcastAddresses, broadcast);
				if (pUDPInterface->BroadcastAddresses.Count > index)
					pUDPInterface->BroadcastPorts[index] = UDPInterfaceClass::UDPListenPort;
			}
		}
	}
}

// Replace gethostname/gethostbyname and the unchecked hostent traversal.
DEFINE_HOOK(0x7B3493, UDPInterfaceClass_StartListening_LocalInterfaces, 0x9)
{
	LocalInterfaces();
	return 0x7B35E4;
}
