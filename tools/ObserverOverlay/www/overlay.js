/*
 * CnCNet Yuri's Revenge observer overlay.
 *
 * Polls /state.json (the spawner's observer_overlay.json) and renders one panel per player:
 * credits, power, production queues and counts of key units / buildings from /units.json.
 *
 * The model-building functions are pure so they can be tested with `node --test`.
 */
(function (root, factory) {
	const api = factory();
	if (typeof module === "object" && module.exports) {
		module.exports = api;
	} else {
		root.ObserverOverlay = api;
	}
})(typeof self !== "undefined" ? self : this, function () {
	"use strict";

	const CATEGORY_ORDER = ["Building", "Defense", "Infantry", "Vehicle", "Naval", "Aircraft"];
	const FALLBACK_COLOR = "#888888";
	const SUPPORTED_VERSION = 1;

	function formatCredits(credits) {
		const value = Number.isFinite(credits) ? Math.max(0, Math.trunc(credits)) : 0;
		return "$" + value.toString().replace(/\B(?=(\d{3})+(?!\d))/g, ",");
	}

	function safeColor(color) {
		return typeof color === "string" && /^#[0-9A-Fa-f]{6}$/.test(color) ? color : FALLBACK_COLOR;
	}

	function clampPercent(value) {
		if (!Number.isFinite(value)) {
			return 0;
		}
		return Math.min(100, Math.max(0, Math.round(value)));
	}

	function buildPower(power) {
		const output = Number.isFinite(power && power.output) ? power.output : 0;
		const drain = Number.isFinite(power && power.drain) ? power.drain : 0;
		return { output: output, drain: drain, low: drain > 0 && drain > output };
	}

	// Collapse consecutive identical items: [Rhino, Rhino, Flak] -> [Rhino x2, Flak x1].
	function groupQueue(queue) {
		const grouped = [];
		for (const item of Array.isArray(queue) ? queue : []) {
			const last = grouped[grouped.length - 1];
			if (last && last.id === item.id) {
				last.count += 1;
			} else {
				grouped.push({ id: item.id, name: item.name || item.id, count: 1 });
			}
		}
		return grouped;
	}

	function buildProduction(production) {
		const factories = Array.isArray(production) ? production.slice() : [];
		factories.sort(function (a, b) {
			return CATEGORY_ORDER.indexOf(a.category) - CATEGORY_ORDER.indexOf(b.category);
		});
		return factories.map(function (factory) {
			const current = factory.current || null;
			return {
				category: factory.category,
				id: current ? current.id : null,
				name: current ? current.name || current.id : null,
				progress: current ? clampPercent(factory.progress) : 0,
				onHold: Boolean(factory.onHold),
				done: Boolean(factory.done),
				queue: groupQueue(factory.queue),
			};
		});
	}

	// Counts for the units / buildings listed in units.json, in that order. Types with a zero count are skipped.
	function buildTrackedCounts(counts, unitsConfig) {
		const byId = new Map();
		for (const entry of Array.isArray(counts) ? counts : []) {
			byId.set(String(entry.id).toUpperCase(), entry);
		}
		const tracked = [];
		const config = unitsConfig || {};
		for (const group of ["units", "buildings"]) {
			for (const wanted of Array.isArray(config[group]) ? config[group] : []) {
				const entry = byId.get(String(wanted.id).toUpperCase());
				if (entry && entry.count > 0) {
					tracked.push({
						id: entry.id,
						label: wanted.label || entry.name || entry.id,
						count: entry.count,
						group: group,
					});
				}
			}
		}
		return tracked;
	}

	function buildPlayerModel(player, unitsConfig) {
		return {
			index: player.index,
			name: player.name || "Player " + (Number(player.index) + 1),
			country: player.country || "",
			color: safeColor(player.color),
			credits: formatCredits(player.credits),
			power: buildPower(player.power),
			defeated: Boolean(player.defeated),
			production: buildProduction(player.production),
			counts: buildTrackedCounts(player.counts, unitsConfig),
		};
	}

	function buildOverlayModel(state, unitsConfig) {
		if (!state || state.version !== SUPPORTED_VERSION || !Array.isArray(state.players)) {
			return { frame: null, players: [] };
		}
		return {
			frame: state.frame,
			players: state.players.map(function (player) {
				return buildPlayerModel(player, unitsConfig);
			}),
		};
	}

	// ---- DOM rendering. Player names come from other players, so only ever use textContent. ----

	function element(doc, tag, className, text) {
		const node = doc.createElement(tag);
		if (className) {
			node.className = className;
		}
		if (text !== undefined && text !== null) {
			node.textContent = String(text);
		}
		return node;
	}

	function renderPlayer(doc, model) {
		const panel = element(doc, "section", "player" + (model.defeated ? " defeated" : ""));
		panel.style.setProperty("--player-color", model.color);

		const header = element(doc, "header", "player-header");
		header.appendChild(element(doc, "span", "player-name", model.name));
		header.appendChild(element(doc, "span", "player-country", model.country));
		panel.appendChild(header);

		const stats = element(doc, "div", "player-stats");
		stats.appendChild(element(doc, "span", "credits", model.credits));
		const power = element(
			doc,
			"span",
			"power" + (model.power.low ? " low" : ""),
			"⚡ " + model.power.output + " / " + model.power.drain
		);
		stats.appendChild(power);
		panel.appendChild(stats);

		const production = element(doc, "ul", "production");
		for (const factory of model.production) {
			const row = element(doc, "li", "factory" + (factory.onHold ? " on-hold" : "") + (factory.done ? " done" : ""));
			row.appendChild(element(doc, "span", "factory-category", factory.category));
			row.appendChild(element(doc, "span", "factory-item", factory.name || "—"));
			const bar = element(doc, "span", "progress");
			const fill = element(doc, "span", "progress-fill");
			fill.style.width = factory.progress + "%";
			bar.appendChild(fill);
			row.appendChild(bar);
			if (factory.queue.length) {
				const queueText = factory.queue
					.map(function (item) {
						return item.count > 1 ? item.name + " ×" + item.count : item.name;
					})
					.join(", ");
				row.appendChild(element(doc, "span", "factory-queue", queueText));
			}
			production.appendChild(row);
		}
		panel.appendChild(production);

		const counts = element(doc, "ul", "counts");
		for (const count of model.counts) {
			const item = element(doc, "li", "count " + count.group);
			item.appendChild(element(doc, "span", "count-value", count.count));
			item.appendChild(element(doc, "span", "count-label", count.label));
			counts.appendChild(item);
		}
		panel.appendChild(counts);

		return panel;
	}

	function render(doc, container, overlayModel, slot) {
		const players =
			slot === undefined || slot === null
				? overlayModel.players
				: overlayModel.players.slice(slot, slot + 1);
		container.replaceChildren.apply(
			container,
			players.map(function (player) {
				return renderPlayer(doc, player);
			})
		);
		container.classList.toggle("waiting", overlayModel.players.length === 0);
	}

	function start(options) {
		const doc = options.doc || document;
		const fetchImpl = options.fetchImpl || fetch.bind(window);
		const intervalMs = options.intervalMs || 500;
		let unitsConfig = { units: [], buildings: [] };

		function fetchJson(url) {
			return fetchImpl(url, { cache: "no-store" }).then(function (response) {
				return response.ok ? response.json() : null;
			});
		}

		function tick() {
			return fetchJson("state.json")
				.then(function (state) {
					render(doc, options.container, buildOverlayModel(state, unitsConfig), options.slot);
				})
				.catch(function () {
					render(doc, options.container, buildOverlayModel(null, unitsConfig), options.slot);
				});
		}

		return fetchJson("units.json")
			.then(function (config) {
				if (config) {
					unitsConfig = config;
				}
			})
			.catch(function () {})
			.then(function () {
				tick();
				return setInterval(tick, intervalMs);
			});
	}

	return {
		CATEGORY_ORDER: CATEGORY_ORDER,
		formatCredits: formatCredits,
		safeColor: safeColor,
		groupQueue: groupQueue,
		buildProduction: buildProduction,
		buildTrackedCounts: buildTrackedCounts,
		buildPlayerModel: buildPlayerModel,
		buildOverlayModel: buildOverlayModel,
		renderPlayer: renderPlayer,
		render: render,
		start: start,
	};
});
