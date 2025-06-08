function loadTags() {
    for (let elem of document.getElementsByClassName("tag")) {
        let tagName = elem.getAttribute("data-rawtag");
        // TODO: can innerText even be null?
        if (tagName !== null && (elem.innerText == null || elem.innerText.length == 0)) {
            elem.innerText = tagName.replace("_", " ");
        }
    }
}

loadTags();
