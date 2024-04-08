"use strict"

const baseURL = "http://localhost:8000";

async function fetchWithCode(url) {
    return await fetch(url, {
        headers: {
            "Secret-Code": localStorage.getItem("pc-remote-secret-code")
        }
    });
}

async function main() {
    let res = await fetchWithCode(baseURL + "/web/pc");
    if (!res.ok) {
        window.location = baseURL + "/set-code";
    }
    pc_state.innerText = await res.text();

    if (pc_state.innerText === "on") {
        pc_button.disabled = true
        pc_message.style.display = "none";
        reset_button.disabled = false;
    } else {
        pc_button.disabled = false
        reset_button.disabled = true;
    }
}

document.addEventListener("DOMContentLoaded", async () => {
    pc_button.addEventListener("click", async () => {
        pc_message.style.display = "inline-block";
        await fetchWithCode(baseURL + "/web/pc/request-power-on");
    })

    reset_button.addEventListener("click", async () => {
        await fetchWithCode(baseURL + "/web/pc/request-reset");
    })

    pc_pwr_button.addEventListener("click", async () => {
        await fetchWithCode(baseURL + "/web/pc/request-power-button-press");
    });

    setInterval(main, 2000);
});
