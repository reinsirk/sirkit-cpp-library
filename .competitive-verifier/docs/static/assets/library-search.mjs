// SPDX-License-Identifier: CC0-1.0

function normalize(text) {
    return text.normalize("NFKC")
        .replace(/([a-z0-9])([A-Z])/g, "$1 $2")
        .toLowerCase()
        .replace(/[ァ-ヶ]/g, letter => String.fromCharCode(letter.charCodeAt(0) - 0x60))
        .replace(/[^\p{L}\p{N}\p{M}+#]+/gu, " ")
        .trim();
}

export function prepareSearchIndex(entries) {
    return entries.map(entry => {
        const basename = entry.source.split("/").at(-1);
        const stem = basename.replace(/\.cpp$/, "");
        const fields = [[entry.source, 54], [basename, 48], [entry.title, 40], [stem, 36],
                        ...entry.keywords.map(term => [term, 28]),
                        [entry.group, 8], [entry.source.split("/")[0], 8]];
        return {entry, fields: fields.map(([text, weight]) => {
            const value = normalize(text);
            return {value, compact: value.replaceAll(" ", ""), words: value.split(" "), weight};
        })};
    });
}

function fieldScore(query, field) {
    const {value, compact, words, weight} = field;
    const joined = query.replaceAll(" ", "");
    if (value === query || compact === joined) return weight + 64;
    if (words.includes(query)) return weight + 48;
    if (words.some(word => word.startsWith(query)) || value.startsWith(query)) return weight + 16;
    if (/^[a-z0-9+#]{1,3}$/.test(joined)) return 0;
    if (value.includes(query) || compact.includes(joined)) return weight;
    return 0;
}

export function searchLibraries(index, text) {
    const query = normalize(text);
    if (!query) return [];
    const terms = query.split(" ");
    const ranked = [];
    const best = (term, fields) => Math.max(...fields.map(field => fieldScore(term, field)));
    for (const {entry, fields} of index) {
        const scores = terms.map(term => best(term, fields));
        if (scores.every(Boolean)) {
            ranked.push({entry, score: scores.reduce((sum, score) => sum + score, 0) + 2 * best(query, fields)});
        }
    }
    ranked.sort((a, b) => b.score - a.score || a.entry.title.localeCompare(b.entry.title, "ja"));
    return ranked.map(result => result.entry);
}

function mountSearch(section) {
    const form = section.querySelector("form");
    const input = section.querySelector("input");
    const status = section.querySelector('[role="status"]');
    const results = section.querySelector("ul");
    const catalog = document.querySelector("[data-library-catalog]");
    const versionRows = new Map([...catalog?.querySelectorAll("[data-library-source]") ?? []]
        .map(row => [row.dataset.librarySource, row]));
    const indexUrl = new URL(section.dataset.searchIndex, document.baseURI);
    const libraryRoot = new URL("../", indexUrl);
    let indexPromise;
    let updateNumber = 0;

    function loadIndex() {
        indexPromise ??= fetch(indexUrl)
            .then(response => {
                if (!response.ok) throw new Error(`HTTP ${response.status}`);
                return response.json();
            })
            .then(prepareSearchIndex)
            .catch(error => {
                indexPromise = undefined;
                throw error;
            });
        return indexPromise;
    }

    function resultRow(entry) {
        const versionRow = versionRows.get(entry.source);
        if (versionRow) {
            const row = versionRow.cloneNode(true);
            row.removeAttribute("data-library-source");
            return row;
        }
        const row = document.createElement("li");
        const name = document.createElement("div");
        const link = document.createElement("a");
        link.href = new URL(entry.path, libraryRoot);
        link.textContent = entry.title;
        const filename = document.createElement("small");
        filename.textContent = entry.source;
        name.append(link, filename);
        const group = document.createElement("span");
        group.className = "library-search-group";
        group.textContent = entry.group;
        row.append(name, group);
        return row;
    }

    async function update() {
        const number = ++updateNumber;
        const text = input.value;
        if (!normalize(text)) {
            results.replaceChildren();
            results.hidden = true;
            status.textContent = "";
            if (catalog) catalog.hidden = false;
            return;
        }

        if (!indexPromise) status.textContent = "検索データを読み込み中…";
        try {
            const index = await loadIndex();
            if (number !== updateNumber) return;
            const matches = searchLibraries(index, text);
            results.replaceChildren(...matches.map(resultRow));
            results.hidden = !matches.length;
            status.textContent = matches.length ? `${matches.length}件` : "一致するライブラリはありません。";
            if (catalog) catalog.hidden = true;
        } catch {
            if (number !== updateNumber) return;
            results.replaceChildren();
            results.hidden = true;
            status.textContent = "検索データを読み込めませんでした。もう一度検索してください。";
            if (catalog) catalog.hidden = false;
        }
    }

    section.hidden = false;
    input.addEventListener("input", event => {
        if (!event.isComposing) update();
    });
    input.addEventListener("compositionend", update);
    input.addEventListener("keydown", event => {
        if (event.key === "Escape" && !event.isComposing && input.value) {
            event.preventDefault();
            input.value = "";
            update();
        }
    });
    form.addEventListener("submit", event => {
        event.preventDefault();
        update();
    });
    update();
}

if (typeof document !== "undefined") {
    for (const section of document.querySelectorAll("[data-library-search]")) mountSearch(section);
}
