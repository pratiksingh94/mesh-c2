# 🕸️ P2P Distributed Command & Control Mesh (PoC)

<!-- ![Status](https://img.shields.io/badge/build-pass-brightgreen?style=flat-square) -->
![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg?style=flat-square)

---

## 🧠 Concept

This project is a **proof-of-concept decentralized C2 framework** with:
- Peer-to-peer **"gossiping"** implants
- Self-healing network
- A central C2 server that can **die anytime** — but the ops continue

Perfect for exploring malware C2 ideas, network resilience, and implant coordination.
---

## 🚧 Under Development

This project is **actively under development** and is missing many features.

### ✅ What works right now
- Implants can connect to the C2 server and register themselves
- Commands can be sent from C2 server which will be flooded from youngest implant to all other implants (no execution yet)
- Sync peers and tasks with each other by "gossiping", if an implants joins the mesh late, it will still get the command which were sent before his joining by the magic of "gossiping" (p2p syncing)

### ❌ What doesn't work yet
- No encryption or authentication
- No command execution
- No web dashboard

Stay tuned for updates as i got school shit too :)


---

## 🚀 How to use it
<details>
<summary>📥 <strong>Clone the Repository</strong></summary>

First, clone the repository to your local machine:

```sh
git clone https://github.com/pratiksingh94/mesh-c2.git
cd mesh-c2
```
</details>

<details>
<summary>☁️ <strong>C2 Server</strong></summary>

1. **Navigate to the server directory:**
```sh
cd C2
```
2. **Create virtual environment and install the stuff**
```sh
python3 -m venv venv
# For Unix or macOS, use:
source venv/bin/activate
# For Windows, use:
venv\Scripts\activate
pip3 install -r requirements.txt
```
3. **Start the C2 server:**
```sh
python3 server.py
```
</details>

<details>
<summary>🧠 <strong>Implant</strong></summary>

1. **Navigate to the implant directory:**
    ```sh
    cd implant
    ```

2. **Change the configuration**

    Make a copy of `/includes/config.example.h` and rename it to `config.h`
    Now edit the content of the the file according to your setup anc choice

3. **Run the implant:**
    > ⚠️ **Before proceeding, ensure you have followed step 2 and configured `config.h` as described above. This step is mandatory for both methods below.**

    ---

    ### **Method 1: 🐳 Docker (Recommended)**

    1. **Navigate to the implant directory:**
        ```sh
        cd implant
        ```
    2. **Build the Docker image:**
        ```sh
        docker build -t mesh-c2-implant .
        ```
        > If the build fails, please [open an issue](https://github.com/pratiksingh94/mesh-c2/issues).

    3. **Run the implant container (you can run this multiple times for multiple instances :D):**
        ```sh
        docker run --rm mesh-c2-implant
        ```

    ---

    ### **Method 2: 🛠️ Make (Manual Build & Run)**

    1. **Navigate to the implant directory:**
        ```sh
        cd implant
        ```
    2. **Build the implant using Make:**
        ```sh
        make
        ```
    3. **Copy the resulting binary (`implant`) to each VM or system you want in the mesh.**

    4. **Run the implant on each system:**
        ```sh
        ./implant
        ```
</details>

<!-- <details>
<summary>📊 <strong>Dashboard (Not added yet)</strong></summary>

1. **Navigate to the dashboard directory:**
    ```sh
    cd dashboard
    ```
2. **Install dependencies:**
    ```sh
    npm install
    ```
3. **Start the dashboard:**
    ```sh
    npm start
    ```

</details> -->
<details>
<summary>🧪 <strong>Test it: Send a Command</strong></summary>

Once your C2 server and at least one implant are running, you can test sending a command to the mesh using a simple `curl` request (no dashboard yet):

```sh
curl -X POST http://localhost:8000/admin/send-command \
    -H "Content-Type: application/json" \
    -d '{"cmd": "whoami"}'
```

- You can see the output on the implant logs
- You can also see all implants end up having same amount of peers after few minutes (rounds of gossiping), because they will be share with each other and synced
- Replace `whoami` with any command you want to send to the implants
- Adjust the URL/port if your C2 server is running elsewhere


<!-- > The command will be distributed through the mesh, but **actual execution is not implemented yet** (see roadmap above). -->
</details>

## 🖼️ Architecture

```mermaid
graph TD
    Dashboard["📊 Dashboard UI"]
    C2["☁️ C2 Server
    (Picks youngest Impant)"]
    A["🧠 Implant A"]
    B["🧠 Implant B"]
    C["🧠 Implant C"]
    D["🧠 Implant D"]
    
    Dashboard <--API Calls--> C2

    C2 <--Payload/Control Calls--> D
    
    
    D <--Gossip/Flood--> A
    D <--Gossip/Flood--> B
    D <--Gossip/Flood--> C
    A <--Gossip--> B
    B <--Gossip--> C
    C <--Gossip--> A
````

> Each implant gossips with its neighbors, passes commands, relays results
> If C2 dies, implants still talk to each other. This is the REAL SHIT 🗣️ lmao

---

## 🧩 Components

| Part         | Lang   | Description                                   |
| ------------ | ------ | --------------------------------------------- |
| `implant/`   | C      | Listener + client that fetches, gossips, runs |
| `server/`    | Python | REST API to register implants & send commands |
| `dashboard/` | JS     | Control Dashboard                             |

---


<!-- ## 🔐 Security Notes

* **No TLS/encryption** yet (plaintext JSON over TCP lol)
* Gossiping done over raw TCP — will get noisy
* No persistence — implants die when you close terminal
* Designed to run inside **your own VMs or lab network** -->

---

<!-- ## 💡 BIG IDEAS Roadmap (in future)

* [ ] 🔒 AES/ChaCha20 encrypted payloads
* [ ] 🧬 Auto discovery via broadcast or multicast
* [ ] 🛡️ Implant obfuscation / packing
* [ ] 📦 Multi-platform binary builder (makefile? idk) -->

---

## ⚠️ Disclaimer

> **This project is a POC and for educational research only.**
> Use it on your own lab environment, VM net, or testbed.
> **Don’t run this on any network without permission.**

