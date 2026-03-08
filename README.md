# NEXUS RESCUE
### Disaster Rescue Route Optimizer with Dynamic Hazard Propagation

> **Course:** CSE 124 — Data Structure Lab | **Group:** 5

---

## 🚨 Overview

NEXUS Rescue is a web-based disaster rescue simulation system that computes optimal rescue routes in real-time as hazards (floods, earthquakes) dynamically spread across a city map. Built on core data structures, it demonstrates how Graph Theory, Dijkstra's Algorithm, BFS, and Greedy Assignment can solve life-critical real-world problems.

---

## 🌐 Live Demo

> Open `web/nexus_rescue.html` in any browser
>
> **Admin Login:** `admin` / `nexus123`

---

## ✨ Features

- 🗺 **Interactive Dhaka Map** — Real locations using Leaflet.js + OpenStreetMap
- 🌊 **Dynamic Hazard Propagation** — BFS-based disaster spread animation
- 🚁 **Shortest Rescue Route** — Dijkstra's Algorithm with animated path
- 👥 **Multi-Team Assignment** — Greedy strategy for multiple rescue teams
- 🔐 **Admin Panel** — Trigger disasters, block roads, assign teams
- 📡 **Public View** — Read-only map for situational awareness
- 📋 **System Log** — Real-time operation log with timestamps

---

## 🧠 Data Structures Used

| Data Structure | Usage |
|---|---|
| Graph (Adjacency List) | City map representation |
| Min-Heap Priority Queue | Dijkstra's Algorithm |
| Queue | BFS hazard propagation |
| Struct + Array | Rescue team and victim management |

---

## 🗂 Project Structure

```
nexus-rescue/
├── c/
│   ├── graph.c              # Module 1: Graph build
│   ├── dijkstra.c           # Module 2: Shortest path
│   ├── bfs_hazard.c         # Module 3: Hazard propagation
│   ├── greedy_assignment.c  # Module 4: Team assignment
│   └── main.c               # Complete merged program
├── web/
│   └── nexus_rescue.html    # Full web application
├── docs/
│   └── Group_5_Proposal.pdf # Lab project proposal
└── README.md
```

---

## ⚙️ How to Run (C Program)

```bash
gcc c/main.c -o nexus_rescue
./nexus_rescue
```

---

## 🔄 System Flow

```
Input Map Data
      ↓
Build Graph (Adjacency List)
      ↓
Admin Triggers Disaster
      ↓
BFS Hazard Propagation → Block Affected Roads
      ↓
Dijkstra's Algorithm → Shortest Safe Route
      ↓
Greedy Assignment → Assign Teams to Victims
      ↓
Visualize on Web Map
      ↓
All Rescued? → END | New Hazard? → Re-run
```

---

## 🚀 Future Scope

- IoT sensor integration for real-time hazard detection
- Drone navigation using computed routes
- Android/iOS mobile application
- Machine learning for predictive road failure
- National emergency coordination network

---

## 📚 References

1. Cormen et al. *Introduction to Algorithms*, 3rd ed. MIT Press, 2009.
2. Bangladesh Disaster Management Bureau. www.ddm.gov.bd
3. GeeksforGeeks. Dijkstra's Algorithm in C. www.geeksforgeeks.org
4. Sedgewick & Wayne. *Algorithms*, 4th ed. Addison-Wesley, 2011.

---

*NEXUS Rescue | Group 5 | CSE 124 — Data Structure Lab*
