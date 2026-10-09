// Tests for the overlay model building and rendering. Run with: node --test tools/ObserverOverlay/tests/overlay.test.js
"use strict";

const test = require("node:test");
const assert = require("node:assert/strict");
const path = require("node:path");

const overlay = require(path.join(__dirname, "..", "www", "overlay.js"));

const UNITS_CONFIG = {
	units: [
		{ id: "HTNK", label: "Rhino" },
		{ id: "MTNK", label: "Grizzly" },
		{ id: "HARV" },
	],
	buildings: [{ id: "NAWEAP", label: "War Factory" }],
};

function samplePlayer(overrides) {
	return Object.assign(
		{
			index: 0,
			name: "Kane",
			country: "Russians",
			color: "#E01008",
			credits: 1234567,
			power: { output: 300, drain: 350 },
			defeated: false,
			production: [
				{ category: "Vehicle", current: { id: "HTNK", name: "Rhino Tank" }, progress: 50, onHold: false, done: false, queue: [
					{ id: "HTNK", name: "Rhino Tank" },
					{ id: "HTNK", name: "Rhino Tank" },
					{ id: "HTK", name: "Flak Track" },
				] },
				{ category: "Building", current: null, progress: 0, onHold: true, done: false, queue: [{ id: "NAPOWR", name: "Tesla Reactor" }] },
				{ category: "Infantry", current: { id: "E2", name: "Conscript" }, progress: 140, onHold: false, done: false, queue: [] },
			],
			counts: [
				{ category: "Building", id: "NAWEAP", name: "Soviet War Factory", count: 2 },
				{ category: "Vehicle", id: "HTNK", name: "Rhino Tank", count: 6 },
				{ category: "Vehicle", id: "HARV", name: "War Miner", count: 3 },
				{ category: "Vehicle", id: "SHAD", name: "Siege Chopper", count: 1 },
			],
		},
		overrides || {}
	);
}

test("formatCredits adds thousands separators and never shows negatives", () => {
	assert.equal(overlay.formatCredits(1234567), "$1,234,567");
	assert.equal(overlay.formatCredits(0), "$0");
	assert.equal(overlay.formatCredits(-50), "$0");
	assert.equal(overlay.formatCredits(undefined), "$0");
});

test("safeColor only accepts #RRGGBB", () => {
	assert.equal(overlay.safeColor("#00ff7F"), "#00ff7F");
	assert.equal(overlay.safeColor("red; background:url(x)"), "#888888");
	assert.equal(overlay.safeColor(null), "#888888");
});

test("groupQueue collapses consecutive duplicates", () => {
	assert.deepEqual(
		overlay.groupQueue([{ id: "HTNK", name: "Rhino" }, { id: "HTNK", name: "Rhino" }, { id: "HTK", name: "" }, { id: "HTNK", name: "Rhino" }]),
		[
			{ id: "HTNK", name: "Rhino", count: 2 },
			{ id: "HTK", name: "HTK", count: 1 },
			{ id: "HTNK", name: "Rhino", count: 1 },
		]
	);
	assert.deepEqual(overlay.groupQueue(undefined), []);
});

test("buildPlayerModel orders production by factory type and clamps progress", () => {
	const model = overlay.buildPlayerModel(samplePlayer(), UNITS_CONFIG);

	assert.deepEqual(
		model.production.map((factory) => factory.category),
		["Building", "Infantry", "Vehicle"]
	);
	const building = model.production[0];
	assert.equal(building.name, null);
	assert.equal(building.onHold, true);
	assert.equal(model.production[1].progress, 100);
	const vehicles = model.production[2];
	assert.equal(vehicles.name, "Rhino Tank");
	assert.equal(vehicles.progress, 50);
	assert.deepEqual(vehicles.queue, [
		{ id: "HTNK", name: "Rhino Tank", count: 2 },
		{ id: "HTK", name: "Flak Track", count: 1 },
	]);
});

test("buildPlayerModel only shows tracked counts, in units.json order, with labels", () => {
	const model = overlay.buildPlayerModel(samplePlayer(), UNITS_CONFIG);

	assert.deepEqual(model.counts, [
		{ id: "HTNK", label: "Rhino", count: 6, group: "units" },
		{ id: "HARV", label: "War Miner", count: 3, group: "units" },
		{ id: "NAWEAP", label: "War Factory", count: 2, group: "buildings" },
	]);
	assert.equal(model.credits, "$1,234,567");
	assert.deepEqual(model.power, { output: 300, drain: 350, low: true });
});

test("buildOverlayModel ignores missing or unknown snapshot versions", () => {
	assert.deepEqual(overlay.buildOverlayModel(null, UNITS_CONFIG), { frame: null, players: [] });
	assert.deepEqual(overlay.buildOverlayModel({ version: 99, players: [samplePlayer()] }, UNITS_CONFIG), {
		frame: null,
		players: [],
	});

	const model = overlay.buildOverlayModel({ version: 1, frame: 1800, players: [samplePlayer(), samplePlayer({ index: 1, name: "" })] }, UNITS_CONFIG);
	assert.equal(model.frame, 1800);
	assert.equal(model.players.length, 2);
	assert.equal(model.players[1].name, "Player 2");
});

// Minimal stand-in for the DOM so rendering can be tested without a browser.
function fakeDocument() {
	function createElement(tag) {
		return {
			tagName: tag.toUpperCase(),
			className: "",
			textContent: "",
			children: [],
			style: {
				props: {},
				setProperty(name, value) {
					this.props[name] = value;
				},
			},
			appendChild(child) {
				this.children.push(child);
				return child;
			},
			get innerHTML() {
				throw new Error("renderer must not use innerHTML");
			},
		};
	}
	return { createElement };
}

function findAll(node, className, found = []) {
	if ((" " + node.className + " ").includes(" " + className + " ")) {
		found.push(node);
	}
	for (const child of node.children || []) {
		findAll(child, className, found);
	}
	return found;
}

test("renderPlayer renders names as text, and shows production and counts", () => {
	const doc = fakeDocument();
	const model = overlay.buildPlayerModel(samplePlayer({ name: "<img src=x onerror=alert(1)>" }), UNITS_CONFIG);

	const panel = overlay.renderPlayer(doc, model);

	assert.equal(panel.style.props["--player-color"], "#E01008");
	assert.equal(findAll(panel, "player-name")[0].textContent, "<img src=x onerror=alert(1)>");
	assert.equal(findAll(panel, "credits")[0].textContent, "$1,234,567");
	assert.equal(findAll(panel, "power")[0].className, "power low");
	assert.equal(findAll(panel, "factory").length, 3);
	assert.equal(findAll(panel, "factory-queue")[1].textContent, "Rhino Tank ×2, Flak Track");
	assert.equal(findAll(panel, "progress-fill")[2].style.width, "50%");
	assert.deepEqual(
		findAll(panel, "count-label").map((label) => label.textContent),
		["Rhino", "War Miner", "War Factory"]
	);
});
