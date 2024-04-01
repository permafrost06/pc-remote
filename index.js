"use strict"

const baseURL = "http://localhost:8000";

async function main() {
    let res = await fetch(baseURL + "/web/pc");
    pc_state.innerText = await res.text();

    if (pc_state.innerText === "on") {
        pc_button.style.display = "none"
        pc_message.style.display = "none";
        reset_panel.style.display = "block";
    } else {
        pc_button.style.display = "inline-block"
        reset_panel.style.display = "none";
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
