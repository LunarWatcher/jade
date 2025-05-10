function setEnabled(state) {
    const button = document.getElementById("submit");

    if (button == null) {
        throw button;
    }
    
    button.toggleAttribute("disabled", !state);
}

function login(ev) {
    ev.preventDefault();

    const form = ev.target;
    const username = form.elements["username"].value;
    const password = form.elements["password"].value;

    fetch("/api/auth/login", {
        method: "POST",
        body: JSON.stringify({
            "username": username,
            "password": password
        }),
        credentials: "include"
    })
        .then(res => {
            setEnabled(true);
            if (res.status != 200) {
                res.json().then(json => {
                    console.log(json);
                    showDialog(json["message"]);
                })
            } else {
                res.json().then(json => {
                    localStorage.setItem("user", JSON.stringify(json["user"]));
                    window.location.href = "/";
                })
            }
        })
        .catch(err => {
            setEnabled(true);
            showDialog("Failed to connect to the server. Try again later");
            console.log(err);
        });

    setEnabled(false);
    return false;
}

// Properly handle weird backwards shit
setEnabled(true);
