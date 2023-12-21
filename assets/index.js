async function search(prompt) {
    const results = document.getElementById("results")
    results.innerHTML = "";
    const response = await fetch("/api/search", {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: prompt,
    });
    const json = await response.json();
	console.log(json);
    results.innerHTML = "";
    for ([path, rank] of json) {
        let item = document.createElement("span");
        item.appendChild(document.createTextNode(path));
        item.appendChild(document.createElement("br"));
        results.appendChild(item);
    }
}

let search_input = document.getElementById("search-input");
let currentSearch = Promise.resolve()

search_input.addEventListener("keypress", (e) => {
    if (e.key == "Enter") {
        currentSearch.then(() => search(search_input.value));
    }
})
