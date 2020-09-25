#include <stdio.h>
#include <stdlib.h>

#define N 20

typedef int Vertex;

int graph[N][N] = {};
Vertex result[N] = {};
size_t result_size = 0;

void graphviz()
{
    printf("digraph \"topsort\" {\n");
    for (size_t i = 0; i < result_size; ++i) {
        printf("    %d [label=\"%d\"];\n", result[i], i);
    }

    for (Vertex v = 0; v < N; ++v) {
        for (Vertex u = 0; u < N; ++u) {
            if (graph[v][u]) {
                printf("    %d -> %d;\n", v, u);
            }
        }
    }
    printf("}");
}

Vertex dfs[N] = {};
size_t dfs_size = 0;
int visited[N] = {};

void topsort(Vertex v)
{
    visited[v] = 1;
    dfs[dfs_size++] = v;
    for (Vertex u = 0; u < N; ++u) {
        if (!visited[u] && graph[v][u]) {
            topsort(u);
        }
    }
    result[result_size++] = dfs[--dfs_size];
}

int main()
{
    graph[0][1] = 1;
    graph[1][2] = 1;
    graph[2][3] = 1;
    graph[2][4] = 1;
    graph[1][5] = 1;
    graph[0][6] = 1;
    graph[0][7] = 1;
    graph[7][8] = 1;
    graph[8][9] = 1;
    graph[8][10] = 1;
    graph[7][11] = 1;
    topsort(0);
    graphviz();

    return 0;
}
