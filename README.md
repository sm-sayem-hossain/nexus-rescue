# NEXUS RESCUE

**Real-time disaster response coordination platform for Bangladesh.**

Live at [sm-sayem-hossain.github.io/nexus-rescue](https://sm-sayem-hossain.github.io/nexus-rescue/)

---

## Overview

NEXUS RESCUE is a multi-admin disaster management system built for national-scale emergency response operations in Bangladesh. It combines real-time mapping, intelligent team deployment, cross-admin coordination, and an AI assistant trained on live operational data — all delivered as a single, zero-dependency HTML file.

The system is actively used for coordinating rescue teams across flood, cyclone, and other disaster scenarios. Admins can deploy from any browser, with no installation required.

---

## Features

**Operational Core**
- Interactive real-time map (Leaflet + CartoDB dark tiles) centered on Bangladesh
- Place disaster markers with four priority levels: Emergency, High, Medium, Low
- Deploy rescue teams and assign them to disasters with live ETA and route calculation
- Road block marking with automatic route avoidance via OpenRouteService
- Animated route visualization with turn-by-turn path rendering

**Multi-Admin System**
- Firebase Authentication with email/password login
- Super Admin creates and manages individual admin accounts from a control panel
- Each admin owns their teams and disasters — other admins cannot modify them
- Super Admin has full override access across all data

**Auto Deployment Mode** (Super Admin only)
- Toggle-based automatic team assignment
- When enabled, any new disaster instantly receives the nearest available team
- Any newly added team is automatically routed to the highest-priority unassigned disaster
- Priority ordering: Emergency > High > Medium > Low, then distance as tiebreaker
- Mode persists across sessions via Firebase — all admins see it in real-time

**Secure Admin Chat**
- Real-time group messaging channel for all logged-in admins
- Image sharing via Firebase Storage
- Messages stored permanently in Cloudflare D1
- Sender identity shown with display name

**NEXUS AI Agent**
- Conversational AI assistant with full awareness of live map state
- Knows all active disasters, team positions, assignments, road blocks, and admin list
- Web search via Tavily — pulls real-time data from Daily Star, Prothom Alo, DDM, BMD, FFWC, and other Bangladesh sources
- Conversation memory across the session (last 10 exchanges)
- Responds in Bengali or English depending on the query language
- Backed by Groq (LLaMA 3.3 70B)

**Permanent Activity Logging**
- Every disaster, team, and road block event is logged to Cloudflare D1 with timestamp, admin identity, and coordinates
- Frontend shows a live session feed; full history is fetchable from the database
- Logs are write-only — no data is ever deleted

---

## Architecture

```
Browser (index.html)
│
├── Firebase Realtime Database    — live map state (teams, disasters, blocks, chat, settings)
├── Firebase Authentication       — admin identity and session management
├── Firebase Storage              — chat image uploads
│
└── Cloudflare Worker (royal-pond)
    ├── /route                    — proxies OpenRouteService routing API (key hidden)
    ├── /log                      — writes activity events to D1
    ├── /logs                     — reads activity history from D1
    ├── /chat                     — reads/writes chat messages to D1
    └── /ai                       — Tavily web search + Groq LLM inference
        │
        ├── Cloudflare D1 (nrv3-logs)
        │   ├── activity_log      — permanent event history
        │   ├── chat_messages     — permanent chat history
        │   └── disaster_knowledge — historical disaster data (RAG source)
        │
        ├── Tavily Search API     — real-time web search
        └── Groq API              — LLaMA 3.3 70B inference
```

**Data flow for AI queries:**
```
Admin question
  → Cloudflare Worker
    → Tavily search (live news/govt sources)
    → D1 knowledge base (historical data)
    → Firebase state (current disasters, teams, blocks)
  → Groq LLM (context assembly + response)
  → Admin
```

---

## Tech Stack

| Layer | Technology |
|---|---|
| Frontend | Vanilla HTML, CSS, JavaScript (zero frameworks) |
| Map | Leaflet.js 1.9.4 + CartoDB dark tiles |
| Realtime Database | Firebase Realtime Database |
| Authentication | Firebase Auth (email/password) |
| File Storage | Firebase Storage |
| Edge Compute | Cloudflare Workers |
| Persistent Database | Cloudflare D1 (SQLite) |
| Routing API | OpenRouteService (driving-car profile) |
| AI Inference | Groq — LLaMA 3.3 70B Versatile |
| Web Search | Tavily Search API |
| Hosting | GitHub Pages / Cloudflare Pages |

---

## Database Schema

**Cloudflare D1 — `nrv3-logs`**

```sql
CREATE TABLE activity_log (
  id           TEXT    PRIMARY KEY,
  event_type   TEXT    NOT NULL,
  admin_id     TEXT    NOT NULL,
  admin_name   TEXT    NOT NULL,
  details      TEXT,
  lat          REAL,
  lng          REAL,
  timestamp    INTEGER NOT NULL
);

CREATE TABLE chat_messages (
  id           TEXT    PRIMARY KEY,
  admin_id     TEXT    NOT NULL,
  admin_name   TEXT    NOT NULL,
  text         TEXT,
  image_url    TEXT,
  date         TEXT    NOT NULL,
  time         TEXT    NOT NULL,
  timestamp    INTEGER NOT NULL
);

CREATE TABLE disaster_knowledge (
  id           TEXT    PRIMARY KEY,
  source       TEXT    NOT NULL,
  title        TEXT,
  content      TEXT    NOT NULL,
  date         TEXT,
  location     TEXT,
  disaster_type TEXT,
  url          TEXT,
  collected_at INTEGER
);
```

**Firebase Realtime Database structure:**

```
/teams/{id}        — name, lat, lng, color, available, ownerUid
/disasters/{id}    — lat, lng, priority, assignedTeam, route, ownerUid
/blocks/{id}       — p1{lat,lng}, p2{lat,lng}, ownerUid
/admins/{uid}      — displayName, email, createdAt
/chat/{id}         — admin_id, admin_name, text, image_url, date, time, timestamp
/settings/autoMode — boolean, synced across all sessions
```

---

## Access Levels

| Role | Capabilities |
|---|---|
| Public | View map, disasters, teams, road blocks in real-time |
| Admin | Add/edit/delete own teams and disasters, block roads, use chat and AI |
| Super Admin | All admin capabilities + manage admin accounts + auto deployment control + full data override |

---

## Disaster Priority System

Emergencies follow a hybrid data structure:

- **Emergency** priority disasters are processed via a **stack** (LIFO) — most recent emergency gets attention first
- **High / Medium / Low** priority disasters are processed via a **priority queue** — ordered by severity

When Auto Deployment Mode is active, this ordering governs which disaster a newly available team is assigned to.

---

## AI Sources

The NEXUS AI agent searches the following sources in real-time:

- thedailystar.net
- prothomalo.com
- bdnews24.com
- jamunatv.com
- somoynews.tv
- ddm.gov.bd (Department of Disaster Management)
- bmd.gov.bd (Bangladesh Meteorological Department)
- ffwc.gov.bd (Flood Forecasting and Warning Centre)
- reliefweb.int

---

## Security Notes

- All third-party API keys (ORS, Groq, Tavily) are stored as Cloudflare Worker secrets — never exposed to the browser
- Firebase config is intentionally public; access is governed by Firebase Security Rules
- Firebase Rules enforce `auth != null` on all write operations and sensitive read paths
- `serviceAccount.json` and `.env` are excluded from version control

---

## Built by

S.M. Sayem Hossain
