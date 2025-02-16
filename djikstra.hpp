#include <iostream>
#include <climits>
#include <map>
#include <stack>
#include <cstring>
#include <algorithm> 

#define MATRIX_SIZE 26

using namespace std;

/* A utility function to find the vertex with minimum distance
value, from the set of vertices not yet included in shortest
path tree */
int minDistance(int dist[], bool sptSet[])
{
  // Initialize min value
  int min = INT_MAX, min_index;

  for (int v = 0; v < MATRIX_SIZE; v++){
    if (sptSet[v] == false && dist[v] <= min){
      min = dist[v], min_index = v;
    }
  }

  return min_index;
}

stack<int> djikstras(int graph[MATRIX_SIZE][MATRIX_SIZE], char src, char dst)
{
  int dist[MATRIX_SIZE];
  bool setPath[MATRIX_SIZE];
  int previous[MATRIX_SIZE];

  for (int i = 0; i < MATRIX_SIZE; i++)
  {
    dist[i] = INT_MAX;
    setPath[i] = false;
    previous[i] = INT_MAX;
  }
  dist[src - 'A'] = 0; // distance to it self is 0 

  //find the path from src to dst 
  for (int count = 0; count < MATRIX_SIZE - 1; count++)
  {
    int u = minDistance(dist, setPath);

    setPath[u] = true;

    for (int v = 0; v < MATRIX_SIZE; v++)
    {
      if (!setPath[v] && graph[u][v] && dist[u] != INT_MAX && dist[u] + graph[u][v] < dist[v]) 
      {
        dist[v] = dist[u] + graph[u][v];
        previous[v] = u;
      }
    }
  }

    // backtracks, in order to get path used 
  stack<int> path;
  int u = dst - 'A';
  if (previous[u] != INT_MAX)
  {
    while (u != INT_MAX)
    {
      path.push(u);
      u = previous[u];
    }
  }

  return path;
} 

int min1Distance(double dist[], bool sptSet[])
{
  // Initialize min value
  int min = INT_MAX, min_index;
  for (int v = 0; v < MATRIX_SIZE; v++){
    if (sptSet[v] == false && dist[v] <= min){
      min = dist[v], min_index = v;
    }
  }
  
  return min_index;
}

// LLP djikstra implementation 
stack<int> llpDjikstras(int graph[MATRIX_SIZE][MATRIX_SIZE], int capacity[MATRIX_SIZE][MATRIX_SIZE], char src, char dst, string algo)
{
    string alpha = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    double dist[MATRIX_SIZE];
    bool setPath[MATRIX_SIZE];
    double previous[MATRIX_SIZE];
    double load = 0;
    double cost = 0; 

  for (int i = 0; i < MATRIX_SIZE; i++)
  {
    dist[i] = INT_MAX;
    setPath[i] = false;
    previous[i] = INT_MAX;
  }
  dist[src - 'A'] = 0; // distance to it self is 0 

  //find the path from src to dst 
  for (int count = 0; count < MATRIX_SIZE - 1; count++)
  {
    int u = min1Distance(dist, setPath); 
    setPath[u] = true;
    
    for (int v = 0; v < MATRIX_SIZE; v++)
    { 
        load = 0;
        if(graph[u][v]) {
            load = 1.0 - ((double) graph[u][v] / (double) capacity[u][v]);
        }

        cost = max(load,dist[u]);
        if (!setPath[v] && graph[u][v] && dist[u] != INT_MAX && cost < dist[v]) {
            //dist[v] = max(dist[u], dist[v]);
            dist[v] = cost;
            previous[v] = u;
        }
    }
  }

  stack<int> path;
  int u = dst - 'A';
  if (previous[u] != INT_MAX)
  {
    while (u != INT_MAX){
      path.push(u);
      u = previous[u];
    }
  }

  return path;
}

/* A utility function to find the vertex with minimum distance
value, from the set of vertices not yet included in shortest
path tree */
int min2Distance(int dist[], bool sptSet[])
{
  // Initialize min value
  int min = -1, min_index;
  for (int v = 0; v < MATRIX_SIZE; v++){
    if (sptSet[v] == false && dist[v] > min){
      min = dist[v], min_index = v;
    }
  }
  
  return min_index;
}

// MFC djikstra implementation 
stack<int> mfcDjikstras(int graph[MATRIX_SIZE][MATRIX_SIZE], int capacity[MATRIX_SIZE][MATRIX_SIZE], char src, char dst, string algo)
{
    string alpha = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int dist[MATRIX_SIZE];
    bool setPath[MATRIX_SIZE];
    int previous[MATRIX_SIZE];
    double load = 0; 

  for (int i = 0; i < MATRIX_SIZE; i++)
  {
    dist[i] = -1;
    setPath[i] = false;
    previous[i] = -1;
  }
  dist[src - 'A'] = 0; // distance to it self is 0 

  //find the path from src to dst 
  for (int count = 0; count < MATRIX_SIZE - 1; count++)
  {
    int u = min2Distance(dist, setPath); 
    setPath[u] = true;
    
    for (int v = 0; v < MATRIX_SIZE; v++)
    { 
      if (graph[u][v]){
          load = (double) graph[u][v];
      }else
      {
        load = min(dist[u], capacity[u][v]);
      }
      
      if (!setPath[v] && graph[u][v] && dist[u] != -1 && load > dist[v]) {
        dist[v] = load; //min(dist[u], dist[v]);
        previous[v] = u;
      }
    }
  }

  stack<int> path;
  int u = dst - 'A';
  if (previous[u] != -1)
  {
    while (u != -1){
      path.push(u);
      u = previous[u];
    }
  }

  return path;
}