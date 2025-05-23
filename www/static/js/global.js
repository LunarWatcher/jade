function showDialog(message) {
    // TODO: make proper dialogs at some point:tm:
    alert(message);
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

