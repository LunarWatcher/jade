function addLibrary(ev) {
    ev.preventDefault();

    const form = ev.target;
    const eLibLoc = form.elements["library_location"];
    const libLoc = eLibLoc.value;


    fetch("/api/admin/create-library", {
        method: "POST",
        body: JSON.stringify({
            location: libLoc
        }),
        credentials: "include"
    })
        .then(res => {
            if (res.status != 201) {
                res.json().then(json => {
                    console.error(json);
                    showDialog(json["message"]);
                })
            } else {
                eLibLoc.value = "";
                location.reload();
            }
        })
        .catch(err => {
            showDialog("Failed to connect to the server. Try again later");
            console.error(err);
        });

}

function reindex() {
    callAPI(
        "admin/reindex",
        "POST",
        null,
        {
            202: () => {
                showDialog("Reindex requested successfully.");
            },
            409: () => {
                showDialog("Reindex has already been requested, and is being processed.");
            }
        }
    )
}

registerOnSubmit("add-library", addLibrary)
document.getElementById("reindex-btn")
    ?.addEventListener("click", reindex);
