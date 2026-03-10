# NEXUS RESCUE

**Disaster Route Optimizer with Real-Time Hazard Management**

[![Live Demo](https://img.shields.io/badge/Live%20Demo-Online-brightgreen)](https://sm-sayem-hossain.github.io/nexus-rescue/)
[![Firebase](https://img.shields.io/badge/Database-Firebase-orange)](https://firebase.google.com/)
[![License](https://img.shields.io/badge/License-MIT-blue)](LICENSE)

---

## Overview

NEXUS Rescue is a real-time disaster response coordination system that enables emergency administrators to deploy rescue teams, identify disaster zones, compute optimal road routes, and manage road blockages — all on a live interactive map. Any authorized user anywhere in the world can monitor the situation in real time through the public view.

The system uses the OpenRouteService API for real road-network routing with dynamic road avoidance, Firebase Realtime Database for live cross-device synchronization, and Leaflet.js for interactive map rendering.

**Live:** [https://sm-sayem-hossain.github.io/nexus-rescue/](https://sm-sayem-hossain.github.io/nexus-rescue/)

---

## Features

**Disaster Management**
- Place disaster markers anywhere on the map with four priority levels: Emergency, High, Medium, and Low
- The system automatically identifies the nearest available rescue team and computes the shortest road route
- Each disaster displays its assigned team, priority, and live status
- Individual disasters can be cleared, releasing the assigned team back to available status

**Real Road Routing**
- Routes are computed over the actual road network using the OpenRouteService Directions API
- Blocked roads are passed as avoidance polygons to the routing engine, forcing genuine detours
- Animated route lines are rendered on the map and persist across sessions via Firebase

**Road Blocking**
- Administrators can mark any road segment as blocked by selecting two points on the map
- Blocked segments are visually indicated and affect all subsequent route calculations
- Blocks are synced in real time to all connected clients

**Team Management**
- Four rescue teams are deployed by default; additional teams can be added by clicking the map
- Teams display live status: Ready or On Mission
- Team names are editable inline; teams can be deleted with automatic disaster reassignment handling
- Clicking a team marker shows a popup with mission details including destination and ETA

**Access Control**
- Admin login provides full control over the map
- Public view is read-only, showing live disaster locations, team statuses, active routes, and blocked roads
- All changes made by the admin are reflected instantly across all connected sessions

---

## Tech Stack

| Layer | Technology |
|---|---|
| Frontend | HTML, CSS, JavaScript |
| Map | Leaflet.js + CartoDB Dark Tiles |
| Routing | OpenRouteService Directions API v2 |
| Database | Firebase Realtime Database |
| Hosting | GitHub Pages |

---

## How It Works

```
Admin places disaster on map
          ↓
System queries ORS API for routes to all available teams
(blocked roads passed as avoid_polygons)
          ↓
Nearest team is assigned — route drawn on map
          ↓
Data saved to Firebase Realtime Database
          ↓
All connected clients (public/admin) receive update instantly
          ↓
Admin clears disaster → team status resets to Ready
```

---

## Project Structure

```
nexus-rescue/
├── c/
│   ├── graph.c                  # Graph construction (Adjacency List)
│   ├── dijkstra.c               # Shortest path (Min-Heap Priority Queue)
│   ├── bfs_hazard.c             # Hazard propagation (BFS)
│   ├── greedy_assignment.c      # Team assignment (Greedy Algorithm)
│   └── main.c                   # Complete merged simulation
├── web/
│   └── nexus_rescue.html        # Full web application (single file)
├── docs/
│   └── Group_5_Proposal.pdf     # Project proposal document
├── index.html                   # Root redirect
└── README.md
```

---

## Running the C Simulation

The `c/` directory contains a standalone command-line simulation of the core algorithms independent of the web interface.

```bash
gcc c/main.c -o nexus_rescue
./nexus_rescue
```

The simulation models a flood scenario across a Dhaka city graph, propagates hazard zones via BFS, computes shortest safe routes using Dijkstra's algorithm, and assigns three rescue teams using a greedy strategy.

---

## Setup

The web application is a single HTML file with no build step required. To run locally, open `web/nexus_rescue.html` in any modern browser.

**Admin credentials:** `admin` / `nexus123`

An active internet connection is required for map tiles, routing, and Firebase sync.

---

## Future Scope

- IoT sensor integration for automated hazard detection
- Mobile application for field rescue teams
- ML-based predictive road failure modeling
- Multi-city and national emergency coordination network
- Role-based authentication with Firebase Auth

---

## Author

**S.M. Sayem Hossain**
[github.com/sm-sayem-hossain](https://github.com/sm-sayem-hossain)
