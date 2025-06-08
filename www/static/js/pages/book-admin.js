function editBook(event) {
    event.preventDefault();

    let form = event.target;
    let title = form["title"].value;
    let isbn = form["isbn"].value.trim();
    let description = form["description"].value.trim();
    let authors = form["authors"]
        .value
        .split("&")
        .map(it => it.trim())
        .filter(it => it != "");
    let tags = form["tags"]
        .value
        .split(" ")
        .filter(it => it != "");
    const bookID = form["bookid"].value;

    fetch(`/api/books/${bookID}/edit`, {
        method: "POST",
        body: JSON.stringify({
                             title: title,
                             description: description || null,
                             isbn: isbn || null,
                             authors: authors,
                             tags: tags,
        }),
        credentials: "include"
    })
        .then(res => {
            if (res.status == 404) {
                showDialog("Failed to find book. Has the book been deleted since you opened the page?");
            } else if (res.status != 200) {
                showDialog("Failed to update book");
            } else if (res.status == 200) {
                res.json().then(() => {
                    location.reload();
                });
            }
        })
        .catch(err => {
            console.error(err);
            showDialog("Failed to connect to the server");
        });
    return false;
}

document.getElementById("edit-book-btn").onclick =
    ev => {
        ev.preventDefault();
        initModal("edit-dialog", true)
    };
document.getElementById("close-edit-modal-btn").onclick =
    ev => {
        ev.preventDefault();
        closeModal("edit-dialog");
    };

registerOnSubmit(
    "edit-book",
    editBook
)
