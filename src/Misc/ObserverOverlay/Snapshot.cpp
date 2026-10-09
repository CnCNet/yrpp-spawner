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

#include "Snapshot.h"

#include <cstdio>

namespace ObserverOverlay
{
	namespace
	{
		void AppendKey(std::string& out, std::string_view key)
		{
			AppendJsonString(out, key);
			out += ':';
		}

		void AppendInt(std::string& out, int value)
		{
			out += std::to_string(value);
		}

		void AppendBool(std::string& out, bool value)
		{
			out += value ? "true" : "false";
		}

		void AppendColor(std::string& out, unsigned char r, unsigned char g, unsigned char b)
		{
			char buffer[8];
			std::snprintf(buffer, sizeof(buffer), "#%02X%02X%02X", r, g, b);
			AppendJsonString(out, buffer);
		}

		void AppendType(std::string& out, const TypeEntry& type)
		{
			out += '{';
			AppendKey(out, "id");
			AppendJsonString(out, type.ID);
			out += ',';
			AppendKey(out, "name");
			AppendJsonString(out, type.Name.empty() ? type.ID : type.Name);
			out += '}';
		}

		void AppendFactory(std::string& out, const FactoryState& factory)
		{
			out += '{';
			AppendKey(out, "category");
			AppendJsonString(out, factory.Category);
			out += ',';
			AppendKey(out, "current");
			if (factory.HasCurrent)
				AppendType(out, factory.Current);
			else
				out += "null";
			out += ',';
			AppendKey(out, "progress");
			AppendInt(out, factory.ProgressPercent);
			out += ',';
			AppendKey(out, "onHold");
			AppendBool(out, factory.OnHold);
			out += ',';
			AppendKey(out, "done");
			AppendBool(out, factory.IsDone);
			out += ',';
			AppendKey(out, "queue");
			out += '[';
			for (size_t i = 0; i < factory.Queue.size(); ++i)
			{
				if (i)
					out += ',';
				AppendType(out, factory.Queue[i]);
			}
			out += "]}";
		}

		void AppendCount(std::string& out, const CountEntry& count)
		{
			out += '{';
			AppendKey(out, "category");
			AppendJsonString(out, count.Category);
			out += ',';
			AppendKey(out, "id");
			AppendJsonString(out, count.Type.ID);
			out += ',';
			AppendKey(out, "name");
			AppendJsonString(out, count.Type.Name.empty() ? count.Type.ID : count.Type.Name);
			out += ',';
			AppendKey(out, "count");
			AppendInt(out, count.Count);
			out += '}';
		}

		void AppendPlayer(std::string& out, const PlayerState& player)
		{
			out += '{';
			AppendKey(out, "index");
			AppendInt(out, player.Index);
			out += ',';
			AppendKey(out, "name");
			AppendJsonString(out, player.Name);
			out += ',';
			AppendKey(out, "country");
			AppendJsonString(out, player.Country);
			out += ',';
			AppendKey(out, "color");
			AppendColor(out, player.ColorR, player.ColorG, player.ColorB);
			out += ',';
			AppendKey(out, "credits");
			AppendInt(out, player.Credits);
			out += ',';
			AppendKey(out, "power");
			out += '{';
			AppendKey(out, "output");
			AppendInt(out, player.PowerOutput);
			out += ',';
			AppendKey(out, "drain");
			AppendInt(out, player.PowerDrain);
			out += "},";
			AppendKey(out, "defeated");
			AppendBool(out, player.Defeated);
			out += ',';
			AppendKey(out, "production");
			out += '[';
			for (size_t i = 0; i < player.Production.size(); ++i)
			{
				if (i)
					out += ',';
				AppendFactory(out, player.Production[i]);
			}
			out += "],";
			AppendKey(out, "counts");
			out += '[';
			for (size_t i = 0; i < player.Counts.size(); ++i)
			{
				if (i)
					out += ',';
				AppendCount(out, player.Counts[i]);
			}
			out += "]}";
		}
	}

	void AppendJsonString(std::string& out, std::string_view value)
	{
		out += '"';
		for (const char ch : value)
		{
			const auto byte = static_cast<unsigned char>(ch);
			switch (ch)
			{
			case '"':
				out += "\\\"";
				break;
			case '\\':
				out += "\\\\";
				break;
			case '\b':
				out += "\\b";
				break;
			case '\f':
				out += "\\f";
				break;
			case '\n':
				out += "\\n";
				break;
			case '\r':
				out += "\\r";
				break;
			case '\t':
				out += "\\t";
				break;
			default:
				if (byte < 0x20)
				{
					char buffer[7];
					std::snprintf(buffer, sizeof(buffer), "\\u%04X", byte);
					out += buffer;
				}
				else
				{
					// Bytes >= 0x80 are part of UTF-8 sequences and are valid in JSON strings as-is.
					out += ch;
				}
				break;
			}
		}
		out += '"';
	}

	std::string ToJson(const Snapshot& snapshot)
	{
		std::string out;
		out.reserve(4096);
		out += '{';
		AppendKey(out, "version");
		AppendInt(out, SnapshotVersion);
		out += ',';
		AppendKey(out, "frame");
		AppendInt(out, snapshot.Frame);
		out += ',';
		AppendKey(out, "players");
		out += '[';
		for (size_t i = 0; i < snapshot.Players.size(); ++i)
		{
			if (i)
				out += ',';
			AppendPlayer(out, snapshot.Players[i]);
		}
		out += "]}";
		return out;
	}

	int ProgressToPercent(int steps, int totalSteps)
	{
		if (totalSteps <= 0 || steps <= 0)
			return 0;
		if (steps >= totalSteps)
			return 100;
		return steps * 100 / totalSteps;
	}
}
