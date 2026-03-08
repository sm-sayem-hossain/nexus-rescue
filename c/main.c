/*
 * ============================================================
 *  NEXUS RESCUE
 *  Disaster Rescue Route Optimizer
 *  with Dynamic Hazard Propagation
 *
 *  Complete Program - All Modules Merged
 *  Course  : CSE 124 - Data Structure Lab
 *  Group   : 3
 *
 *  Modules:
 *    1. Graph Build       (Adjacency List + Dynamic Memory)
 *    2. Dijkstra          (Min-Heap Priority Queue)
 *    3. BFS Hazard        (Hazard Propagation)
 *    4. Greedy Assignment (Multi-Team Rescue)
 * ============================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Constants ─────────────────────────────────────────── */
#define MAX_NAME    50
#define INF         99999
#define MAX_NODES   100
#define MAX_TEAMS   10
#define MAX_VICTIMS 10

/* ══════════════════════════════════════════════════════════
 *  DATA STRUCTURES
 * ══════════════════════════════════════════════════════════ */

/* Edge — road between two locations */
typedef struct Edge {
    int dest;
    int weight;
    int blocked;
    struct Edge *next;
} Edge;

/* Node — a location on the map */
typedef struct {
    char  name[MAX_NAME];
    Edge *head;
    int   hazard_level;   /* 0=SAFE, 1=WARNING, 2=DANGER */
} Node;

/* Graph — the full disaster map */
typedef struct {
    Node *nodes;
    int   nodeCount;
    int   capacity;
} Graph;

/* Min-Heap item */
typedef struct {
    int node;
    int dist;
} HeapItem;

/* Min-Heap Priority Queue */
typedef struct {
    HeapItem *data;
    int       size;
    int       capacity;
} MinHeap;

/* Queue for BFS */
typedef struct {
    int data[MAX_NODES];
    int front, rear, size;
} Queue;

/* Rescue Team */
typedef struct {
    int  id;
    char name[MAX_NAME];
    int  location;
    int  available;
} RescueTeam;

/* Victim */
typedef struct {
    int id;
    int location;
    int rescued;
    int team_id;
} Victim;


/* ══════════════════════════════════════════════════════════
 *  GRAPH FUNCTIONS
 * ══════════════════════════════════════════════════════════ */
Graph *createGraph(int cap) {
    Graph *g     = (Graph *)malloc(sizeof(Graph));
    g->nodes     = (Node *)malloc(cap * sizeof(Node));
    g->nodeCount = 0;
    g->capacity  = cap;
    return g;
}

int addNode(Graph *g, const char *name) {
    if (g->nodeCount == g->capacity) {
        g->capacity *= 2;
        g->nodes = (Node *)realloc(g->nodes, g->capacity * sizeof(Node));
    }
    int idx = g->nodeCount;
    strncpy(g->nodes[idx].name, name, MAX_NAME - 1);
    g->nodes[idx].name[MAX_NAME - 1] = '\0';
    g->nodes[idx].head          = NULL;
    g->nodes[idx].hazard_level  = 0;
    g->nodeCount++;
    return idx;
}

Edge *createEdge(int dest, int weight) {
    Edge *e  = (Edge *)malloc(sizeof(Edge));
    e->dest    = dest;
    e->weight  = weight;
    e->blocked = 0;
    e->next    = NULL;
    return e;
}

void addEdge(Graph *g, int src, int dest, int weight) {
    Edge *e1 = createEdge(dest, weight);
    e1->next = g->nodes[src].head;
    g->nodes[src].head = e1;

    Edge *e2 = createEdge(src, weight);
    e2->next = g->nodes[dest].head;
    g->nodes[dest].head = e2;
}

void blockRoad(Graph *g, int src, int dest) {
    Edge *e = g->nodes[src].head;
    while (e) { if (e->dest == dest) { e->blocked = 1; break; } e = e->next; }
    e = g->nodes[dest].head;
    while (e) { if (e->dest == src)  { e->blocked = 1; break; } e = e->next; }
}

void blockAllRoadsOf(Graph *g, int node) {
    Edge *e = g->nodes[node].head;
    while (e) {
        e->blocked = 1;
        Edge *rev = g->nodes[e->dest].head;
        while (rev) { if (rev->dest == node) { rev->blocked = 1; break; } rev = rev->next; }
        e = e->next;
    }
}

void freeGraph(Graph *g) {
    for (int i = 0; i < g->nodeCount; i++) {
        Edge *e = g->nodes[i].head;
        while (e) { Edge *t = e; e = e->next; free(t); }
    }
    free(g->nodes);
    free(g);
}


/* ══════════════════════════════════════════════════════════
 *  MIN-HEAP FUNCTIONS
 * ══════════════════════════════════════════════════════════ */
MinHeap *createMinHeap(int cap) {
    MinHeap *h  = (MinHeap *)malloc(sizeof(MinHeap));
    h->data     = (HeapItem *)malloc(cap * sizeof(HeapItem));
    h->size     = 0;
    h->capacity = cap;
    return h;
}

void swapH(HeapItem *a, HeapItem *b) {
    HeapItem t = *a; *a = *b; *b = t;
}

void heapifyUp(MinHeap *h, int i) {
    while (i > 0) {
        int p = (i - 1) / 2;
        if (h->data[p].dist > h->data[i].dist) {
            swapH(&h->data[p], &h->data[i]);
            i = p;
        } else break;
    }
}

void heapifyDown(MinHeap *h, int i) {
    int s = i, l = 2*i+1, r = 2*i+2;
    if (l < h->size && h->data[l].dist < h->data[s].dist) s = l;
    if (r < h->size && h->data[r].dist < h->data[s].dist) s = r;
    if (s != i) { swapH(&h->data[s], &h->data[i]); heapifyDown(h, s); }
}

void insertHeap(MinHeap *h, int node, int dist) {
    if (h->size == h->capacity) {
        h->capacity *= 2;
        h->data = (HeapItem *)realloc(h->data, h->capacity * sizeof(HeapItem));
    }
    h->data[h->size].node = node;
    h->data[h->size].dist = dist;
    heapifyUp(h, h->size++);
}

HeapItem extractMin(MinHeap *h) {
    HeapItem m = h->data[0];
    h->data[0] = h->data[--h->size];
    heapifyDown(h, 0);
    return m;
}

void freeHeap(MinHeap *h) { free(h->data); free(h); }


/* ══════════════════════════════════════════════════════════
 *  QUEUE FUNCTIONS (BFS)
 * ══════════════════════════════════════════════════════════ */
void initQueue(Queue *q) { q->front = 0; q->rear = -1; q->size = 0; }
int  isEmpty(Queue *q)   { return q->size == 0; }

void enqueue(Queue *q, int val) {
    q->rear = (q->rear + 1) % MAX_NODES;
    q->data[q->rear] = val;
    q->size++;
}

int dequeue(Queue *q) {
    int val  = q->data[q->front];
    q->front = (q->front + 1) % MAX_NODES;
    q->size--;
    return val;
}


/* ══════════════════════════════════════════════════════════
 *  DIJKSTRA'S ALGORITHM
 * ══════════════════════════════════════════════════════════ */
void dijkstra(Graph *g, int src, int *dist, int *prev) {
    int n = g->nodeCount;
    for (int i = 0; i < n; i++) { dist[i] = INF; prev[i] = -1; }
    dist[src] = 0;

    MinHeap *h = createMinHeap(n * 2);
    insertHeap(h, src, 0);

    while (h->size > 0) {
        HeapItem curr = extractMin(h);
        int u = curr.node;
        if (curr.dist > dist[u]) continue;

        Edge *e = g->nodes[u].head;
        while (e) {
            if (!e->blocked) {
                int v  = e->dest;
                int nd = dist[u] + e->weight;
                if (nd < dist[v]) {
                    dist[v] = nd;
                    prev[v] = u;
                    insertHeap(h, v, nd);
                }
            }
            e = e->next;
        }
    }
    freeHeap(h);
}


/* ══════════════════════════════════════════════════════════
 *  BFS HAZARD PROPAGATION
 * ══════════════════════════════════════════════════════════ */
void propagateHazard(Graph *g, int origin, int max_levels) {
    int visited[MAX_NODES] = {0};
    int level[MAX_NODES];
    for (int i = 0; i < MAX_NODES; i++) level[i] = -1;

    Queue q;
    initQueue(&q);

    visited[origin]              = 1;
    level[origin]                = 0;
    g->nodes[origin].hazard_level = 2;
    blockAllRoadsOf(g, origin);
    enqueue(&q, origin);

    printf("\n  Hazard Origin : %s\n", g->nodes[origin].name);
    printf("  Spread Levels : %d\n\n", max_levels);
    printf("  %-20s %-8s %-10s\n", "Location", "Level", "Status");
    printf("  %-20s %-8s %-10s\n", "--------", "-----", "------");
    printf("  %-20s %-8d %-10s\n",
           g->nodes[origin].name, 0, "DANGER");

    while (!isEmpty(&q)) {
        int curr       = dequeue(&q);
        int curr_level = level[curr];

        if (curr_level >= max_levels) continue;

        Edge *e = g->nodes[curr].head;
        while (e) {
            int nb = e->dest;
            if (!visited[nb]) {
                visited[nb] = 1;
                level[nb]   = curr_level + 1;

                g->nodes[nb].hazard_level = (level[nb] == 1) ? 2 : 1;
                blockAllRoadsOf(g, nb);

                printf("  %-20s %-8d %-10s\n",
                       g->nodes[nb].name,
                       level[nb],
                       level[nb] == 1 ? "DANGER" : "WARNING");

                enqueue(&q, nb);
            }
            e = e->next;
        }
    }
    printf("\n  [!] All roads in disaster zone are now BLOCKED.\n");
}


/* ══════════════════════════════════════════════════════════
 *  GREEDY TEAM ASSIGNMENT
 * ══════════════════════════════════════════════════════════ */
void printRoute(Graph *g, int dest, int *prev, int *dist) {
    if (dist[dest] == INF) {
        printf("      Route : NO ROUTE AVAILABLE\n");
        return;
    }
    int path[MAX_NODES], len = 0, cur = dest;
    while (cur != -1) { path[len++] = cur; cur = prev[cur]; }
    printf("      Route : ");
    for (int i = len - 1; i >= 0; i--) {
        printf("%s", g->nodes[path[i]].name);
        if (i > 0) printf(" -> ");
    }
    printf("\n      Cost  : %d units\n", dist[dest]);
}

void assignTeams(Graph *g,
                 RescueTeam *teams,  int teamCount,
                 Victim     *victims, int victimCount) {

    int dist[MAX_NODES], prev[MAX_NODES];
    int best_dist_arr[MAX_NODES], best_prev_arr[MAX_NODES];

    for (int v = 0; v < victimCount; v++) {
        printf("\n  Victim #%d trapped at [%s]\n",
               victims[v].id,
               g->nodes[victims[v].location].name);

        int best_team = -1;
        int best_dist = INF;

        for (int t = 0; t < teamCount; t++) {
            if (!teams[t].available) continue;
            dijkstra(g, teams[t].location, dist, prev);
            int d = dist[victims[v].location];
            printf("    %s @ %-12s -> %s\n",
                   teams[t].name,
                   g->nodes[teams[t].location].name,
                   d == INF ? "NO ROUTE" : "");
            if (d != INF) printf("    %*s%d units\n", 32, "", d);

            if (d < best_dist) {
                best_dist = d;
                best_team = t;
                memcpy(best_dist_arr, dist, sizeof(int) * g->nodeCount);
                memcpy(best_prev_arr, prev, sizeof(int) * g->nodeCount);
            }
        }

        if (best_team == -1) {
            printf("\n    [X] No team can reach Victim #%d!\n", victims[v].id);
            continue;
        }

        teams[best_team].available = 0;
        victims[v].rescued         = 1;
        victims[v].team_id         = teams[best_team].id;

        printf("\n    [ASSIGNED] %s -> Victim #%d\n",
               teams[best_team].name, victims[v].id);
        printRoute(g, victims[v].location, best_prev_arr, best_dist_arr);
    }
}


/* ══════════════════════════════════════════════════════════
 *  PRINT HELPERS
 * ══════════════════════════════════════════════════════════ */
void printMapStatus(Graph *g) {
    printf("\n  %-20s %-10s %-8s\n", "Location", "Status", "Roads");
    printf("  %-20s %-10s %-8s\n", "--------", "------", "-----");
    for (int i = 0; i < g->nodeCount; i++) {
        char *st = g->nodes[i].hazard_level == 0 ? "SAFE" :
                   g->nodes[i].hazard_level == 1 ? "WARNING" : "DANGER";
        int open = 0, blocked = 0;
        Edge *e = g->nodes[i].head;
        while (e) { e->blocked ? blocked++ : open++; e = e->next; }
        printf("  %-20s %-10s Open:%-2d Blocked:%-2d\n",
               g->nodes[i].name, st, open, blocked);
    }
}

void printMissionSummary(Graph *g,
                          RescueTeam *teams,   int teamCount,
                          Victim     *victims, int victimCount) {

    printf("\n╔══════════════════════════════════════════════════╗\n");
    printf("║            NEXUS RESCUE - MISSION SUMMARY        ║\n");
    printf("╚══════════════════════════════════════════════════╝\n\n");

    int rescued = 0, stranded = 0;
    for (int v = 0; v < victimCount; v++) {
        if (victims[v].rescued) {
            printf("  [RESCUED ] Victim #%d at %-12s -> %s\n",
                   victims[v].id,
                   g->nodes[victims[v].location].name,
                   teams[victims[v].team_id - 1].name);
            rescued++;
        } else {
            printf("  [STRANDED] Victim #%d at %-12s -> No team reached\n",
                   victims[v].id,
                   g->nodes[victims[v].location].name);
            stranded++;
        }
    }
    printf("\n  Victims Total  : %d\n", victimCount);
    printf("  Rescued        : %d\n", rescued);
    printf("  Stranded       : %d\n", stranded);
    printf("\n  Team Status:\n");
    for (int t = 0; t < teamCount; t++)
        printf("  %-15s -> %s\n",
               teams[t].name,
               teams[t].available ? "Available" : "On Mission");
    printf("\n");
}


/* ══════════════════════════════════════════════════════════
 *  MAIN — Full Simulation
 * ══════════════════════════════════════════════════════════ */
int main() {
    printf("\n");
    printf("╔══════════════════════════════════════════════════╗\n");
    printf("║                  NEXUS RESCUE                    ║\n");
    printf("║   Disaster Rescue Route Optimizer                ║\n");
    printf("║   with Dynamic Hazard Propagation                ║\n");
    printf("║                                                  ║\n");
    printf("║   CSE 124 - Data Structure Lab  |  Group 3       ║\n");
    printf("╚══════════════════════════════════════════════════╝\n\n");

    /* ── STEP 1: Build the map ─────────────────────────── */
    printf("══════════════════════════════════════════════════\n");
    printf("  STEP 1: Building Disaster Zone Map\n");
    printf("══════════════════════════════════════════════════\n");

    Graph *g      = createGraph(10);
    int mirpur    = addNode(g, "Mirpur");
    int farmgate  = addNode(g, "Farmgate");
    int gulshan   = addNode(g, "Gulshan");
    int mohakhali = addNode(g, "Mohakhali");
    int dhanmondi = addNode(g, "Dhanmondi");
    int motijheel = addNode(g, "Motijheel");
    int badda     = addNode(g, "Badda");

    addEdge(g, mirpur,    farmgate,  5);
    addEdge(g, farmgate,  gulshan,   4);
    addEdge(g, farmgate,  dhanmondi, 3);
    addEdge(g, gulshan,   mohakhali, 2);
    addEdge(g, mohakhali, badda,     3);
    addEdge(g, badda,     motijheel, 6);
    addEdge(g, dhanmondi, motijheel, 7);
    addEdge(g, mirpur,    dhanmondi, 8);
    addEdge(g, mohakhali, motijheel, 5);

    printf("\n  Initial Map Status:\n");
    printMapStatus(g);

    /* ── STEP 2: Disaster strikes ──────────────────────── */
    printf("\n══════════════════════════════════════════════════\n");
    printf("  STEP 2: Disaster Strikes! Hazard Propagation\n");
    printf("══════════════════════════════════════════════════\n");
    printf("  [ALERT] Flood detected at Gulshan!\n");

    propagateHazard(g, gulshan, 1);

    printf("\n  Updated Map Status:\n");
    printMapStatus(g);

    /* ── STEP 3: Place rescue teams and victims ─────────── */
    printf("\n══════════════════════════════════════════════════\n");
    printf("  STEP 3: Deploying Rescue Teams and Victims\n");
    printf("══════════════════════════════════════════════════\n");

    RescueTeam teams[] = {
        {1, "Alpha Team", mirpur,    1},
        {2, "Bravo Team", dhanmondi, 1},
        {3, "Delta Team", farmgate,  1},
    };
    /* Victims are trapped in areas cut off by flood */
    Victim victims[] = {
        {1, badda,     0, -1},
        {2, motijheel, 0, -1},
        {3, badda,     0, -1},
    };

    printf("\n  Rescue Teams:\n");
    for (int i = 0; i < 3; i++)
        printf("    [%d] %-15s stationed at %s\n",
               teams[i].id, teams[i].name,
               g->nodes[teams[i].location].name);

    printf("\n  Victims (trapped in safe zones, cut off from help):\n");
    for (int i = 0; i < 3; i++)
        printf("    [%d] Victim trapped at %s\n",
               victims[i].id,
               g->nodes[victims[i].location].name);

    /* ── STEP 4: Run Dijkstra + Greedy Assignment ───────── */
    printf("\n══════════════════════════════════════════════════\n");
    printf("  STEP 4: Computing Rescue Routes (Dijkstra)\n");
    printf("          Assigning Teams (Greedy Strategy)\n");
    printf("══════════════════════════════════════════════════\n");

    assignTeams(g, teams, 3, victims, 3);

    /* ── STEP 5: Mission Summary ────────────────────────── */
    printf("\n══════════════════════════════════════════════════\n");
    printf("  STEP 5: Mission Complete\n");
    printf("══════════════════════════════════════════════════\n");

    printMissionSummary(g, teams, 3, victims, 3);

    freeGraph(g);

    printf("╔══════════════════════════════════════════════════╗\n");
    printf("║         NEXUS RESCUE - SIMULATION END            ║\n");
    printf("╚══════════════════════════════════════════════════╝\n\n");

    return 0;
}
