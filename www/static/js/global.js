function showDialog(message) {
    // TODO: make proper dialogs at some point:tm:
    alert(message);
}

function registerOnSubmit(id, func, maybeNull = false) {
    let elem = null;
    if (typeof id === "string") {
        elem = document.getElementById(id);
    } else {
        elem = id;
    }

    if (elem === null) {
        if (maybeNull) {
            return;
        }
        throw Error("Failed to find form elem");
    }

    elem.addEventListener(
        "submit",
        func
    )
}

function initModal(/** @type {String} */ id, /** @type {Boolean} */ reloadForm) {
    let elem = /** @type {HTMLDialogElement | null} */ (document.getElementById(id));
    if (elem == null) {
        alert("Failed to open dialog: element not found (software bug)");
        return false;
    }

    // Dialog open is a noop
   if (elem.open) {
        return false;
    }
    
    elem.showModal();

    if (reloadForm) {
        for (let form of elem.getElementsByTagName("form")) {
            form.reset();
        }
    }
    

    return false;
}

function closeModal(/** @type {String} */ id) {
    let elem = /** @type {HTMLDialogElement | null} */ (document.getElementById(id));
    if (elem == null) {
        alert("Failed to open dialog: element not found (software bug)");
        return false;
    }
    elem.close();
    
    return false;
}

function callAPI(endpoint, method, body, statusHandlers) {
    fetch("/api/" + endpoint, {
        method: method || "GET",
        body: JSON.stringify(body),
        credentials: "include"
    })
        .then(res => {
            // First, check for special handlers. These can do whatever.
            if (statusHandlers && statusHandlers[res.status]) {
                statusHandlers[res.status]();
                return;
            }
            // If no special handlers are available, default to standard checks.
            // 200-299: Refresh
            // 404: Custom error message (JSON return is not guaranteed)
            // >=400 (except 404): The value of the "message" field in the JSON response is displayed.
            if (res.status >= 400) {
                if (res.status == 404) {
                    showDialog("The requested endpoint does not exist (programmer error), or you do not have the permissions required to use it.");
                } else {
                    res.json().then(json => {
                        showDialog("An error occurred while processing your request: " + json["body"]);
                    });
                }
            } else if (res.status >= 200 && res.status < 300) {
                location.reload();
            }
        })
        .catch(err => {
            showDialog("Failed to connect to the server. Try again later");
            console.error(err);
        })
}
