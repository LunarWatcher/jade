function createLinkElem(page, active = false) {
    const elem = document.createElement("li");
    if (page != null) {
        const link = document.createElement("a");
        const url = new URL(
            document.location.href
        );
        url.searchParams.set("page", page.toString());
        link.href = url.toString();
        link.innerText = page.toString()
        if (active) {
            link.classList.add("active");
        }

        elem.appendChild(link);
    } else {
        elem.innerText = "...";
        elem.ariaHidden = "true";
    }

    return elem;
}
function loadPaginator() {
    const elem = /** @type {HTMLElement} */ (document.getElementById("paginator"));
    const dataList = /** @type {HTMLUListElement} */ (elem.children[0]);

    // 1-indexed
    let allPages = parseInt(elem.attributes["data-pages"].value);
    // 1-indexed
    let currPage = parseInt(elem.attributes["data-current-page"].value);
    if (currPage > allPages) {
        elem.style.visibility = "hidden";
        return;
    }

    const width = 7;
    const allocatableSides = width - 1;
    let availablePre = currPage - 1;
    let availablePost = allPages - currPage;

    let pre = availablePre;
    let post = availablePost;
    let elements = [];

    if (pre >= 3 && post >= 3) {
        pre = 3;
        post = 3;
    } else if (pre > post) {
        // pre >= 3, post < 3
        pre = Math.min(
            allocatableSides - post,
            pre
        );

    } else if (post > pre) {
        // post >= 3, pre < 3
        post = Math.min(
            allocatableSides - pre,
            post
        );
    }
    console.log(pre, post);
    
    let includeFirst = false, includeLast = false;
    if (availablePre > 3 && currPage - pre > 0) {
        pre -= 1;
        includeFirst = true;
    }
    if (availablePost > 3 && currPage + post <= allPages ) {
        post -= 1;
        includeLast = true;
    }

    if (includeFirst) {
        elements.push(
            createLinkElem(1),
            createLinkElem(null),
        );
    }

    for (let i = pre; i > 0; --i) {
        elements.push(createLinkElem(currPage - i))
    }
    elements.push(createLinkElem(currPage, true));
    for (let i = 1; i <= post; ++i) {
        elements.push(createLinkElem(currPage + i))
    }

    if (includeLast) {
        elements.push(
            createLinkElem(null),
            createLinkElem(allPages)
        );
    }
    console.log("Elements:", elements);
    let prevPage = document.getElementById("next-page").parentElement;
    for (let newElement of elements) {
        dataList.insertBefore(newElement, prevPage);
    }

    if (pre == 0) {
        document.getElementById("prev-page")?.removeAttribute("href");
    }
    if (post == 0) {
        document.getElementById("next-page")?.removeAttribute("href");
    }
}

function navToPage(/** @type {Number | null} */ page, /** @type {Number | null}*/ nextPrev = null) {
    let params = new URLSearchParams(document.location.search);
    let currPage = parseInt(params.get("page") || "0");

    let url = new URL(
        window.location.pathname,
        window.location.origin
    );

    if (page == null) {
        if (nextPrev == null) {
            throw Error("page and nextPrev can't both be null");
        }
        const nextPage = currPage + nextPrev;
        params.set("page", nextPage.toString());
    } else {
        params.set("page", page.toString());
    }
    url.search = params.toString();
    window.location.href = url.toString();
}

function skipToPage(ev) {
    ev.preventDefault();
    navToPage(
        ev.target.elements["page"].value
    )
}

loadPaginator();

document.getElementById("skip-to-page").reset();

registerOnSubmit("skip-to-page", skipToPage)
document.getElementById("prev-page")
    ?.addEventListener("click", ev => {
        ev.preventDefault();
        navToPage(null, -1)
    });
document.getElementById("next-page")
    ?.addEventListener("click", ev => {
        ev.preventDefault();
        navToPage(null, 1)
    });
