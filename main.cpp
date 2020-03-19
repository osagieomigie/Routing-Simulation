#include <iostream>
#include <climits>
#include <vector>
#include <map>
#include <list>
#include <stack>
#include <cstring>
#include <sstream>

using namespace std;

#define CALL_ARRIVAL 0
#define CALL_END 1
#define MAX_ROW 26
#define MAX_COL 26

/* Event record */
struct Event
{
  float event_time;
  int event_type;
  int callid;
  char source;
  char destination;
  float duration;
  stack<string> route;
} typedef EventList;

/* Global matrix data structure for network topology/state information */
int propdelay[MAX_ROW][MAX_COL];
int capacity[MAX_ROW][MAX_COL];
map<string, int> linkCapacity;
int available[MAX_ROW][MAX_COL];
int cost[MAX_ROW][MAX_COL];
list<EventList> workLoad;

/* A utility function to find the vertex with minimum distance
value, from the set of vertices not yet included in shortest
path tree */
int minDistance(int dist[], bool sptSet[])
{
  // Initialize min value
  int min = INT_MAX, min_index;

  for (int v = 0; v < MAX_ROW; v++)
  {
    if (sptSet[v] == false && dist[v] <= min)
    {
      min = dist[v];
      min_index = v;
    }
  }

  return min_index;
}

stack<int> djikstras(int graph[MAX_ROW][MAX_COL], char src, char dst)
{

  int dist[MAX_ROW];
  bool setPath[MAX_ROW];
  int previous[MAX_ROW];

  for (int i = 0; i < MAX_ROW; i++)
  {
    dist[i] = INT_MAX;
    setPath[i] = false;
    previous[i] = INT_MAX;
  }

  dist[src - 'A'] = 0;

  //find the path from src to dst and update the availabilities and as well as the capacities
  for (int count = 0; count < MAX_ROW - 1; count++)
  {
    int u = minDistance(dist, setPath);

    setPath[u] = true;

    //if U is the DST then early exit
    if (u == dst - 'A')
    {
      break;
    }

    for (int v = 0; v < MAX_ROW; v++)
    {
      if (!setPath[v] && graph[u][v] && dist[u] != INT_MAX && dist[u] + graph[u][v] < dist[v])
      {
        dist[v] = dist[u] + graph[u][v];
        previous[v] = u;
      }
    }
  }

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

int RouteCall(char source, char destination, EventList currentEvent)
{
  stack<int> path = djikstras(cost, source, destination);
  int success = 0;
  int tmp;
  string alpha = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  string link;
  string srcNode, dstNode;

  // gets source destination
  tmp = path.top();
  path.pop();

  // get source node character
  srcNode = alpha[tmp];
  //cout << "Souce node " << srcNode << endl;

  while (!path.empty())
  {
    tmp = path.top();
    path.pop();
    //currentEvent.route.push_back(tmp); // add path currently taken by call
    //cout << "tmp: " << tmp << endl;

    // get next node character
    dstNode = alpha[tmp];

    // update link capacity
    link = srcNode + dstNode;

    // add path currently taken by call
    currentEvent.route.push(link);

    if (linkCapacity[link] > 0) // check if link is at capacity
    {
      //cout << "Link capcity: " << linkCapacity[link] << endl;
      linkCapacity[link] = linkCapacity[link] - 1;
      success = 1;
    }
    else
    {
      return 0; // link is at capacity, reject call
    }

    srcNode = dstNode; // sets to nextnode
  }

  return success;
}

// frees up link taken by call
void ReleaseCall(EventList currentEvent)
{
  string link = "";
  while (!currentEvent.route.empty())
  {
    link = currentEvent.route.top();
    currentEvent.route.pop();

    linkCapacity[link] = linkCapacity[link] + 1; // add 1 to links capacity
  }
}

int main()
{
  FILE *fp1 = fopen("topology.dat", "r");
  int i, row, col = 0;
  char src, dst;
  int delay, cap;
  string link = "";

  //fp1 = fopen("topology.dat", "r");
  while ((i = fscanf(fp1, "%c %c %d %d\n", &src, &dst, &delay, &cap)) == 4)
  {
    link += src;
    link += dst;
    //cout << "init link: " << link << endl;
    linkCapacity.insert(pair<string, int>(link, cap));

    row = src - 'A';
    col = dst - 'A';
    propdelay[row][col] = delay;
    propdelay[col][row] = delay;
    capacity[row][col] = cap;
    capacity[col][row] = cap;
    available[row][col] = cap;
    available[col][row] = cap;
    cost[row][col] = 1;
    cost[col][row] = 1;

    link = ""; // reset link
  }
  fclose(fp1);

  //* Next read in the calls from "callworkload.dat" and set up events */
  i = 0;
  int id = 0;
  float dur, arr;
  FILE *fp2 = fopen("callworkload.dat", "r");
  while ((i = fscanf(fp2, "%f %c %c %f\n", &arr, &src, &dst, &dur)) == 4)
  {
    //cout << "Duration: " << arr <<endl;

    // create new event
    EventList tmpEvent;
    tmpEvent.event_time = arr;
    tmpEvent.event_type = CALL_ARRIVAL;
    tmpEvent.callid = id;
    tmpEvent.source = src;
    tmpEvent.destination = dst;
    tmpEvent.duration = dur;
    workLoad.push_back(tmpEvent);

    // add end call event, for the created event
    EventList tmpEvent2;
    tmpEvent2.event_time = arr + dur; // length of call
    tmpEvent2.event_type = CALL_END;
    tmpEvent2.callid = id;
    tmpEvent2.source = src;
    tmpEvent2.destination = dst;
    tmpEvent2.duration = 0;
    workLoad.push_back(tmpEvent2);

    id++; // increment callid for next event
  }
  fclose(fp2);

  // sort topology list so the event arrive in order

  /* Now simulate the call arrivals and departures */
  float time = 0.0;
  int success = 0, blocked = 0;
  int numevents = workLoad.size();

  //cout << "Workload length: " << numevents << endl;

  while ((numevents > 0) && (!workLoad.empty()))
  {

    // get current event
    EventList currentEvent = workLoad.front();
    workLoad.pop_front();

    /* Get information about the current event */
    printf("Event of type %d at time %8.6f (call %d from %c to %c)\n", currentEvent.event_type, currentEvent.event_time, currentEvent.callid, currentEvent.source, currentEvent.destination);

    if (currentEvent.event_type == CALL_ARRIVAL)
    {
      if (RouteCall(currentEvent.source, currentEvent.destination, currentEvent) == 1)
      {
        success++;
      }
      else
      {
        blocked++;
      }
    }
    else
    {
      ReleaseCall(currentEvent);
    }
  }
  /* Print final report here */
  cout << "Success: " << success << " Blocked: " << blocked << endl;
}