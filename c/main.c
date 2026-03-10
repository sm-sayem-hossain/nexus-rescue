#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX 100
#define INF 99999

// --- Structures ---

typedef struct Edge {
    int dest, weight, blocked;
    struct Edge *next;
} Edge;

typedef struct {
    char name[50];
    Edge *head;
    int hazard; // 0=safe 1=warning 2=danger
} Node;

typedef struct {
    int node, dist;
} HeapItem;

typedef struct {
    HeapItem data[MAX*2];
    int size;
} MinHeap;

typedef struct {
    int data[MAX];
    int front, rear, size;
} Queue;

typedef struct {
    int id, loc, available;
    char name[50];
} Team;

typedef struct {
    int id, loc, rescued, team_id;
} Victim;

// --- Globals ---

Node nodes[MAX];
int node_cnt = 0;

// --- Graph ---

int add_node(const char *name)
{
    int i = node_cnt++;
    strncpy(nodes[i].name, name, 49);
    nodes[i].head   = NULL;
    nodes[i].hazard = 0;
    return i;
}

void add_edge(int src, int dest, int w)
{
    Edge *e1 = (Edge *)malloc(sizeof(Edge));
    e1->dest = dest; e1->weight = w; e1->blocked = 0;
    e1->next = nodes[src].head; nodes[src].head = e1;

    Edge *e2 = (Edge *)malloc(sizeof(Edge));
    e2->dest = src; e2->weight = w; e2->blocked = 0;
    e2->next = nodes[dest].head; nodes[dest].head = e2;
}

void block_road(int src, int dest)
{
    for(Edge *e = nodes[src].head;  e; e = e->next) if(e->dest == dest) { e->blocked = 1; break; }
    for(Edge *e = nodes[dest].head; e; e = e->next) if(e->dest == src)  { e->blocked = 1; break; }
}

void block_all(int node)
{
    for(Edge *e = nodes[node].head; e; e = e->next) {
        e->blocked = 1;
        for(Edge *r = nodes[e->dest].head; r; r = r->next)
            if(r->dest == node) { r->blocked = 1; break; }
    }
}

void free_graph()
{
    for(int i = 0; i < node_cnt; i++) {
        Edge *e = nodes[i].head;
        while(e) { Edge *t = e; e = e->next; free(t); }
        nodes[i].head = NULL;
    }
    node_cnt = 0;
}

// --- Min-Heap ---

void heap_push(MinHeap *h, int node, int dist)
{
    int i = h->size++;
    h->data[i] = (HeapItem){node, dist};
    while(i > 0) {
        int p = (i-1)/2;
        if(h->data[p].dist > h->data[i].dist) {
            HeapItem t = h->data[p]; h->data[p] = h->data[i]; h->data[i] = t;
            i = p;
        } else break;
    }
}

HeapItem heap_pop(MinHeap *h)
{
    HeapItem top = h->data[0];
    h->data[0] = h->data[--h->size];
    int i = 0;
    while(1) {
        int s = i, l = 2*i+1, r = 2*i+2;
        if(l < h->size && h->data[l].dist < h->data[s].dist) s = l;
        if(r < h->size && h->data[r].dist < h->data[s].dist) s = r;
        if(s == i) break;
        HeapItem t = h->data[s]; h->data[s] = h->data[i]; h->data[i] = t;
        i = s;
    }
    return top;
}

// --- Queue ---

void q_push(Queue *q, int v) { q->rear = (q->rear+1)%MAX; q->data[q->rear] = v; q->size++; }
int  q_pop(Queue *q)         { int v = q->data[q->front]; q->front = (q->front+1)%MAX; q->size--; return v; }

// --- Dijkstra ---
// dist[] = shortest distance from src, prev[] = path trace

void dijkstra(int src, int *dist, int *prev)
{
    for(int i = 0; i < node_cnt; i++) { dist[i] = INF; prev[i] = -1; }
    dist[src] = 0;

    MinHeap h; h.size = 0;
    heap_push(&h, src, 0);

    while(h.size > 0) {
        HeapItem cur = heap_pop(&h);
        int u = cur.node;
        if(cur.dist > dist[u]) continue;

        for(Edge *e = nodes[u].head; e; e = e->next) {
            if(e->blocked) continue;
            int nd = dist[u] + e->weight;
            if(nd < dist[e->dest]) {
                dist[e->dest] = nd;
                prev[e->dest] = u;
                heap_push(&h, e->dest, nd);
            }
        }
    }
}

// --- BFS Hazard Propagation ---
// spreads hazard from origin, blocks all roads in affected zone

void bfs_hazard(int origin, int max_level)
{
    int vis[MAX] = {0}, level[MAX];
    for(int i = 0; i < MAX; i++) level[i] = -1;

    Queue q; q.front = 0; q.rear = -1; q.size = 0;
    vis[origin] = 1; level[origin] = 0;
    nodes[origin].hazard = 2;
    block_all(origin);
    q_push(&q, origin);

    printf("\n  Hazard Origin : %s\n", nodes[origin].name);
    printf("  %-20s %-8s %s\n", "Location", "Level", "Status");
    printf("  %-20s %-8s %s\n", "--------", "-----", "------");
    printf("  %-20s %-8d %s\n", nodes[origin].name, 0, "DANGER");

    while(q.size > 0) {
        int cur = q_pop(&q);
        if(level[cur] >= max_level) continue;

        for(Edge *e = nodes[cur].head; e; e = e->next) {
            int nb = e->dest;
            if(vis[nb]) continue;
            vis[nb] = 1;
            level[nb] = level[cur] + 1;
            nodes[nb].hazard = (level[nb] == 1) ? 2 : 1;
            block_all(nb);
            printf("  %-20s %-8d %s\n", nodes[nb].name, level[nb],
                   level[nb] == 1 ? "DANGER" : "WARNING");
            q_push(&q, nb);
        }
    }
    printf("\n  [!] All roads in disaster zone BLOCKED.\n");
}

// --- Greedy Team Assignment ---
// for each victim, assign nearest available team using Dijkstra

void print_route(int dest, int *prev, int *dist)
{
    if(dist[dest] == INF) { printf("      Route : NO ROUTE\n"); return; }
    int path[MAX], len = 0, cur = dest;
    while(cur != -1) { path[len++] = cur; cur = prev[cur]; }
    printf("      Route : ");
    for(int i = len-1; i >= 0; i--) {
        printf("%s", nodes[path[i]].name);
        if(i > 0) printf(" -> ");
    }
    printf("\n      Cost  : %d\n", dist[dest]);
}

void assign_teams(Team *teams, int tc, Victim *victims, int vc)
{
    int dist[MAX], prev[MAX], bdist[MAX], bprev[MAX];

    for(int v = 0; v < vc; v++) {
        printf("\n  Victim #%d at [%s]\n", victims[v].id, nodes[victims[v].loc].name);

        int best = -1, best_d = INF;
        for(int t = 0; t < tc; t++) {
            if(!teams[t].available) continue;
            dijkstra(teams[t].loc, dist, prev);
            int d = dist[victims[v].loc];
            printf("    %-14s @ %-12s -> %s\n", teams[t].name,
                   nodes[teams[t].loc].name, d == INF ? "NO ROUTE" : "");
            if(d != INF) printf("    %*s%d units\n", 32, "", d);
            if(d < best_d) {
                best_d = d; best = t;
                memcpy(bdist, dist, sizeof(int)*node_cnt);
                memcpy(bprev, prev, sizeof(int)*node_cnt);
            }
        }
        if(best == -1) { printf("\n    [X] No team can reach Victim #%d!\n", victims[v].id); continue; }

        teams[best].available = 0;
        victims[v].rescued    = 1;
        victims[v].team_id    = teams[best].id;
        printf("\n    [ASSIGNED] %s -> Victim #%d\n", teams[best].name, victims[v].id);
        print_route(victims[v].loc, bprev, bdist);
    }
}

// --- Print Helpers ---

void print_map()
{
    printf("\n  %-20s %-10s %s\n", "Location", "Status", "Roads");
    printf("  %-20s %-10s %s\n",   "--------", "------", "-----");
    for(int i = 0; i < node_cnt; i++) {
        const char *st = nodes[i].hazard == 0 ? "SAFE" : nodes[i].hazard == 1 ? "WARNING" : "DANGER";
        int open = 0, blk = 0;
        for(Edge *e = nodes[i].head; e; e = e->next) e->blocked ? blk++ : open++;
        printf("  %-20s %-10s Open:%-2d Blocked:%-2d\n", nodes[i].name, st, open, blk);
    }
}

void print_summary(Team *teams, int tc, Victim *victims, int vc)
{
    printf("\n  %-10s %-15s %-15s %s\n", "Victim", "Location", "Status", "Team");
    printf("  %-10s %-15s %-15s %s\n",   "------", "--------", "------", "----");
    int rescued = 0, stranded = 0;
    for(int v = 0; v < vc; v++) {
        if(victims[v].rescued) {
            printf("  #%-9d %-15s %-15s %s\n", victims[v].id,
                   nodes[victims[v].loc].name, "RESCUED",
                   teams[victims[v].team_id - 1].name);
            rescued++;
        } else {
            printf("  #%-9d %-15s %s\n", victims[v].id,
                   nodes[victims[v].loc].name, "STRANDED");
            stranded++;
        }
    }
    printf("\n  Total: %d | Rescued: %d | Stranded: %d\n", vc, rescued, stranded);
    printf("\n  Team Status:\n");
    for(int t = 0; t < tc; t++)
        printf("  %-15s -> %s\n", teams[t].name, teams[t].available ? "Available" : "On Mission");
}

// --- Main ---

int main()
{
    printf("\n  NEXUS RESCUE - CSE 124 Lab | Group 5\n\n");

    // step 1: build map
    printf("== STEP 1: Build Map ==\n");
    int mirpur    = add_node("Mirpur");
    int farmgate  = add_node("Farmgate");
    int gulshan   = add_node("Gulshan");
    int mohakhali = add_node("Mohakhali");
    int dhanmondi = add_node("Dhanmondi");
    int motijheel = add_node("Motijheel");
    int badda     = add_node("Badda");

    add_edge(mirpur,    farmgate,  5);
    add_edge(farmgate,  gulshan,   4);
    add_edge(farmgate,  dhanmondi, 3);
    add_edge(gulshan,   mohakhali, 2);
    add_edge(mohakhali, badda,     3);
    add_edge(badda,     motijheel, 6);
    add_edge(dhanmondi, motijheel, 7);
    add_edge(mirpur,    dhanmondi, 8);
    add_edge(mohakhali, motijheel, 5);

    print_map();

    // step 2: disaster strikes
    printf("\n== STEP 2: Hazard Propagation (BFS) ==\n");
    printf("  [ALERT] Flood at Farmgate!\n");
    bfs_hazard(farmgate, 1);
    print_map();

    // step 3: deploy teams & victims
    printf("\n== STEP 3: Deploy Teams & Victims ==\n");
    Team teams[] = {
        {1, motijheel, 1, "Alpha Team"},
        {2, badda,     1, "Bravo Team"},
        {3, mohakhali, 1, "Delta Team"},
    };
    Victim victims[] = {
        {1, mirpur,    0, -1},
        {2, gulshan,   0, -1},
        {3, dhanmondi, 0, -1},
    };
    int tc = 3, vc = 3;

    printf("\n  Teams:\n");
    for(int i = 0; i < tc; i++)
        printf("    [%d] %-15s @ %s\n", teams[i].id, teams[i].name, nodes[teams[i].loc].name);
    printf("\n  Victims:\n");
    for(int i = 0; i < vc; i++)
        printf("    [%d] Trapped at %s\n", victims[i].id, nodes[victims[i].loc].name);

    // step 4: route & assign
    printf("\n== STEP 4: Dijkstra + Greedy Assignment ==\n");
    assign_teams(teams, tc, victims, vc);

    // step 5: summary
    printf("\n== STEP 5: Mission Summary ==\n");
    print_summary(teams, tc, victims, vc);

    free_graph();
    return 0;
}
