"use strict"

const baseURL = "http://localhost:8000";

async function main() {
    let res = await fetch(baseURL + "/web/pc");
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
        await fetch(baseURL + "/web/pc/request-power-on");
    })

    reset_button.addEventListener("click", async () => {
        await fetch(baseURL + "/web/pc/request-reset");
    })

    setInterval(main, 500);
});
