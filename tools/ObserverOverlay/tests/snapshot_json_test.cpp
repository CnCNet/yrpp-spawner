// Unit test for the game-independent part of the observer overlay export.
//
// Build and run from the repository root with any C++20 compiler, e.g.:
//   g++ -std=c++20 -Wall -Wextra -I src/Misc/ObserverOverlay src/Misc/ObserverOverlay/Snapshot.cpp tools/ObserverOverlay/tests/snapshot_json_test.cpp -o snapshot_json_test
//   ./snapshot_json_test > snapshot.json
// The program exits non-zero on failure and prints the sample snapshot JSON on success,
// which test_server.py's sample can be compared against.

#include "Snapshot.h"

#include <cstdio>
#include <cstdlib>
#include <string>

namespace
{
	int Failures = 0;

	void Expect(bool condition, const char* pMessage)
	{
		if (!condition)
		{
			std::fprintf(stderr, "FAIL: %s\n", pMessage);
			++Failures;
		}
	}

	void ExpectEqual(const std::string& actual, const std::string& expected, const char* pMessage)
	{
		if (actual != expected)
		{
			std::fprintf(stderr, "FAIL: %s\n  expected: %s\n  actual:   %s\n", pMessage, expected.c_str(), actual.c_str());
			++Failures;
		}
	}

	std::string Quote(std::string_view value)
	{
		std::string out;
		ObserverOverlay::AppendJsonString(out, value);
		return out;
	}
}

int main()
{
	using namespace ObserverOverlay;

	// String escaping
	ExpectEqual(Quote("Rhino Tank"), "\"Rhino Tank\"", "plain strings are quoted");
	ExpectEqual(Quote("a\"b\\c"), "\"a\\\"b\\\\c\"", "quotes and backslashes are escaped");
	ExpectEqual(Quote("tab\tnew\nline\r"), "\"tab\\tnew\\nline\\r\"", "common control characters are escaped");
	ExpectEqual(Quote(std::string_view("\x01\x1F", 2)), "\"\\u0001\\u001F\"", "other control characters use \\u escapes");
	ExpectEqual(Quote("\xD0\x9A\xD0\xB8\xD1\x80\xD0\xBE\xD0\xB2"), "\"\xD0\x9A\xD0\xB8\xD1\x80\xD0\xBE\xD0\xB2\"", "UTF-8 passes through");

	// Progress conversion
	Expect(ProgressToPercent(0, 54) == 0, "0 steps is 0%");
	Expect(ProgressToPercent(27, 54) == 50, "27 of 54 steps is 50%");
	Expect(ProgressToPercent(54, 54) == 100, "54 of 54 steps is 100%");
	Expect(ProgressToPercent(60, 54) == 100, "progress is clamped to 100%");
	Expect(ProgressToPercent(-3, 54) == 0, "negative progress is clamped to 0%");
	Expect(ProgressToPercent(10, 0) == 0, "zero total steps doesn't divide by zero");

	// Empty snapshot
	Snapshot empty;
	empty.Frame = 7;
	ExpectEqual(ToJson(empty), "{\"version\":1,\"frame\":7,\"players\":[]}", "empty snapshot");

	// Full snapshot
	Snapshot snapshot;
	snapshot.Frame = 1800;

	PlayerState soviet;
	soviet.Index = 0;
	soviet.Name = "Kane \"the\" Prophet";
	soviet.Country = "Russians";
	soviet.ColorR = 0xE0;
	soviet.ColorG = 0x10;
	soviet.ColorB = 0x08;
	soviet.Credits = 4250;
	soviet.PowerOutput = 300;
	soviet.PowerDrain = 350;

	FactoryState vehicles;
	vehicles.Category = "Vehicle";
	vehicles.HasCurrent = true;
	vehicles.Current = { "HTNK", "Rhino Tank" };
	vehicles.ProgressPercent = 50;
	vehicles.Queue = { { "HTNK", "Rhino Tank" }, { "HTK", "" } };
	soviet.Production.push_back(vehicles);

	FactoryState buildings;
	buildings.Category = "Building";
	buildings.HasCurrent = false;
	buildings.OnHold = true;
	buildings.Queue = { { "NAPOWR", "Tesla Reactor" } };
	soviet.Production.push_back(buildings);

	soviet.Counts.push_back({ "Vehicle", { "HTNK", "Rhino Tank" }, 6 });
	soviet.Counts.push_back({ "Building", { "NAWEAP", "Soviet War Factory" }, 2 });

	PlayerState allied;
	allied.Index = 1;
	allied.Name = "Tanya";
	allied.Country = "Americans";
	allied.ColorR = 0x00;
	allied.ColorG = 0x5A;
	allied.ColorB = 0xFF;
	allied.Credits = 0;
	allied.Defeated = true;

	snapshot.Players = { soviet, allied };

	const std::string json = ToJson(snapshot);
	const std::string expected =
		"{\"version\":1,\"frame\":1800,\"players\":["
		"{\"index\":0,\"name\":\"Kane \\\"the\\\" Prophet\",\"country\":\"Russians\",\"color\":\"#E01008\","
		"\"credits\":4250,\"power\":{\"output\":300,\"drain\":350},\"defeated\":false,"
		"\"production\":["
		"{\"category\":\"Vehicle\",\"current\":{\"id\":\"HTNK\",\"name\":\"Rhino Tank\"},\"progress\":50,\"onHold\":false,\"done\":false,"
		"\"queue\":[{\"id\":\"HTNK\",\"name\":\"Rhino Tank\"},{\"id\":\"HTK\",\"name\":\"HTK\"}]},"
		"{\"category\":\"Building\",\"current\":null,\"progress\":0,\"onHold\":true,\"done\":false,"
		"\"queue\":[{\"id\":\"NAPOWR\",\"name\":\"Tesla Reactor\"}]}"
		"],"
		"\"counts\":["
		"{\"category\":\"Vehicle\",\"id\":\"HTNK\",\"name\":\"Rhino Tank\",\"count\":6},"
		"{\"category\":\"Building\",\"id\":\"NAWEAP\",\"name\":\"Soviet War Factory\",\"count\":2}"
		"]},"
		"{\"index\":1,\"name\":\"Tanya\",\"country\":\"Americans\",\"color\":\"#005AFF\","
		"\"credits\":0,\"power\":{\"output\":0,\"drain\":0},\"defeated\":true,\"production\":[],\"counts\":[]}"
		"]}";
	ExpectEqual(json, expected, "full snapshot");

	if (Failures)
	{
		std::fprintf(stderr, "%d failure(s)\n", Failures);
		return EXIT_FAILURE;
	}

	std::printf("%s\n", json.c_str());
	return EXIT_SUCCESS;
}
