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
                });
            } else {
                res.json().then(json => {
                    localStorage.setItem("user", JSON.stringify(json["user"]));
                    let params = new URLSearchParams(window.location.search);
                    let redirect = params.get("redirect");
                    // Some validation; make sure the redirect is in the form `/someurl` by making sure the string starts with `/`,
                    // while making sure it isn't `//`, because for example `//kagi.com` would redirect off-site
                    if (redirect != null && redirect.startsWith("/") && (redirect.length == 1 || redirect[1] != "/")) {
                        window.location.href = redirect;
                    } else {
                        window.location.href = "/";
                    }
                });
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

function signup(ev) {
    ev.preventDefault();

    const form = ev.target;
    const username = form.elements["username"].value;
    const password = form.elements["password"].value;

    fetch("/api/auth/signup", {
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
