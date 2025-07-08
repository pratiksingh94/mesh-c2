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

> [!CAUTION]
> UNDER DEVELOPMENT, NO SECURITY NOTHING IS THERE LMAO, so come back later idk

---

## 🖼️ Architecture

```mermaid
graph TD
    Dashboard["📊 Dashboard UI"]
    C2["☁️ C2 Server
    (Picks random Impant)"]
    A["🧠 Implant A"]
    B["🧠 Implant B"]
    C["🧠 Implant C"]
    
    Dashboard <--API Calls--> C2
    C2 <--Payload/Control Calls--> A
    C2 <--Payload/Control Calls--> B
    C2 <--Payload/Control Calls--> C
    
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
| `dashboard/` | JS     | Control Dashboard (map of nodes, logs, stats) |

---


## 🔐 Security Notes

* **No TLS/encryption** yet (plaintext JSON over TCP lol)
* Gossiping done over raw TCP — will get noisy
* No persistence — implants die when you close terminal
* Designed to run inside **your own VMs or lab network**

---

## 💡 BIG IDEAS Roadmap (in future)

* [ ] 🔒 AES/ChaCha20 encrypted payloads
* [ ] 🧬 Auto discovery via broadcast or multicast
* [ ] 🛡️ Implant obfuscation / packing
* [ ] 📦 Multi-platform binary builder (makefile? idk)

---

## ⚠️ Disclaimer

> **This project is a POC and for educational research only.**
> Use it on your own lab environment, VM net, or testbed.
> **Don’t run this on any network without permission.**

