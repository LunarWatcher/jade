let params = new URLSearchParams(document.location.search);
let form = document.getElementById("book-filter");
const search = params.get("search")
const pagesize = params.get("pagesize");
if (search) {
    form.elements["search"].value = search;
}
if (pagesize) {
    // TODO: to a user-configurable value, or maybe just store the last used value in localStorage?
    form.elements["pagesize"].value = pagesize;
}
