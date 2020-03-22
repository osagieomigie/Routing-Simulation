#include <iostream>
#include <vector>
#include <algorithm>
#include <map>
#include <list>
#include <stack>
#include <cstring>
#include <sstream>
#include "djikstra.hpp"

using namespace std;

// Global constants 
#define CALL_ARRIVAL 0
#define CALL_END 1
#define MATRIX_SIZE 26
#define TOPOLOGY_DATA "top_sina.dat"//"topology_small.dat"
#define WORKLOAD_DATA "work_sina.dat" //"callworkload_small.dat"

/* Event record */
struct Event
{
  float event_time;
  int event_type;
  int callid;
  char source;
  char destination;
  float duration;
} typedef EventList;

/* Global matrix data structure for network topology/state information */
int propdelay[MATRIX_SIZE][MATRIX_SIZE];
int capacity[MATRIX_SIZE][MATRIX_SIZE];
int available[MATRIX_SIZE][MATRIX_SIZE];
int cost[MATRIX_SIZE][MATRIX_SIZE];
list<EventList> workLoad;
map<int, stack<int> > routesUsed;
float shpfHops = 0, sdpfHops = 0, llpHops =0; 
float shpfDelay = 0, sdpfDelay = 0, llpDelay =0; 

// custom link comparison; for sorting events
bool compareEvent(const EventList &first, const EventList &second)
{
  return (first.event_time < second.event_time);
}

// Determines if a call event can be routed 
int RouteCall(char source, char destination, EventList currentEvent, int resource[MATRIX_SIZE][MATRIX_SIZE], string algo)
{
  stack<int> path;
  if (algo == "LLP"){
    path = llpDjikstras(resource, capacity, source, destination, algo); 
    if (path.size() == 0){
      return 0; // call is blocked 
    }
  }else
  {
    path = djikstras(resource, source, destination, algo);
  }
  
   
  stack<int> route;
  string alpha = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  string link;

  int srcNode = 0, dstNode=0;
  int srcNode1 = 0, dstNode1=0;

  string s, d; 

  srcNode = path.top();  //s = alpha[srcNode];
  path.pop();

  while(!path.empty())
  {
    dstNode = path.top(); d = alpha[dstNode];
    path.pop();

    link = s + d;
    //cout <<"Link: " << link << endl;
    route.push(srcNode); // add link to event route 
    route.push(dstNode);
    
    // check if a link taken by call is at capacity 
    if ((available[srcNode][dstNode] <= 0) && (available[dstNode][srcNode] <= 0)){
      if (!route.empty()){
        srcNode1 = route.top();
        route.pop(); 
      }
      while(!route.empty()){
        dstNode1 = route.top();
        route.pop();

        // restore capacity back to link 
        available[srcNode1][dstNode1] = available[srcNode1][dstNode1] +1; 
        available[dstNode1][srcNode1] = available[dstNode1][srcNode1] +1;

        srcNode1 = dstNode1; 
      }
      //cout << "Blocked "<< endl;
      return 0; // Call can't be completed 
        
    }
    
    // reduce capacity by one 
    //cout << "link cap  1: " << available[srcNode][dstNode] << " link cap 2: " << available[srcNode][dstNode] << endl; 
    available[srcNode][dstNode] = available[srcNode][dstNode] - 1; 
    available[dstNode][srcNode] = available[dstNode][srcNode] - 1;
    //cout << "after link cap  1: " << available[srcNode][dstNode] << " link cap 2: " << available[srcNode][dstNode] << endl; 

    if (algo == "SHPF"){
      shpfHops++; // increase links it went through, for shortest hop path first 
      shpfDelay = shpfDelay + propdelay[srcNode][dstNode]; // store delay for links traversed through 
    }else if (algo == "SDPF"){
      sdpfHops++; // increase links it went through, for shortest delay path first 
      sdpfDelay = sdpfDelay + propdelay[srcNode][dstNode];; // store delay for links traversed through 
    }else if (algo == "LLP"){
      llpHops++; // increase links it went through, for Least Loaded path
      llpDelay = llpDelay + propdelay[srcNode][dstNode];; // store delay for links traversed through 
    }
    srcNode = dstNode; s = d; // set to next node 
  }

  routesUsed.insert(pair<int, stack<int> >(currentEvent.callid, route)); 
  return 1;
}

// frees up link taken by call
void ReleaseCall(int callid)
{
  // Check if call is valid 
  if (routesUsed.find(callid) != routesUsed.end()){
    string alpha = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    string s, d, link; 
    stack<int> route = routesUsed[callid]; 
    int srcNode = 0, dstNode=0;
    
    srcNode = route.top(); s = alpha[srcNode]; // get first node 
    route.pop();

    while (!route.empty())
    {
      dstNode = route.top(); d = alpha[dstNode];
      route.pop();

      link = s + d;
      //cout <<"Link Released: " << link << endl;

      // increase capacity by 1 
      available[srcNode][dstNode] = available[srcNode][dstNode] +1; 
      available[dstNode][srcNode] = available[dstNode][srcNode] +1;

      srcNode = dstNode; // set to previous destination 
    }
  }
  else{
    //cout << "Invalid release call" << endl;
  }
}

// Simulates all 4 policies 
void simulatePolicy(string algo, int resource[MATRIX_SIZE][MATRIX_SIZE], list<EventList> workLoad){
  //Simulates the call arrivals and departures
  float time = 0.0;
  int success = 0, blocked = 0, totalCalls = 0;
  int numevents = workLoad.size();
  //float shpfAverage = 0; 

  while ((numevents > 0) && (!workLoad.empty()))
  {
    // get current event
    EventList currentEvent = workLoad.front();
    workLoad.pop_front();

    /* Get information about the current event */
    //printf("Event of type %d at time %8.6f (call %d from %c to %c)\n", currentEvent.event_type, currentEvent.event_time, currentEvent.callid, currentEvent.source, currentEvent.destination);

    if (currentEvent.event_type == CALL_ARRIVAL)
    {
      totalCalls++;

      // Pass resources to route call
      if (RouteCall(currentEvent.source, currentEvent.destination, currentEvent, resource, algo) == 1)
      {
        success++; // call was successful 
      }
      else
      {
        blocked++; // call wasn't successful
      }
    }
    else
    {
      ReleaseCall(currentEvent.callid); // Handles end call event 
    }
  }

  if (algo == "SHPF"){
    //shpfAverage = shpfHops/success; // calculate hops for SHPF
    cout << "SHPF\t" << totalCalls << "\t" << success <<"\t" << blocked << "\t" << shpfHops/success << "\t" << shpfDelay/success << endl;
  }else if(algo == "SDPF"){ 
    cout << "SDPF\t" << totalCalls << "\t" << success <<"\t" << blocked << " ("<< blocked/100 << ")"<<"\t" << sdpfHops/success << "\t" << sdpfDelay/success << endl;
  } else if(algo == "LLP"){ 
    cout << "LLP\t" << totalCalls << "\t" << success <<"\t" << blocked << " ("<< blocked/100 << ")"<<"\t " << llpHops/success << "\t" << llpDelay/success << endl;
  } 
  
}

int main()
{
  FILE *fp1 = fopen(TOPOLOGY_DATA, "r");
  int i, row, col = 0;
  char src, dst;
  int delay, cap;
  string link1 = ""; 
  string link2 = "";

  // read in topology data from "topology.dat"
  while ((i = fscanf(fp1, "%c %c %d %d\n", &src, &dst, &delay, &cap)) == 4)
  {
    // row and column position 
    row = src - 'A';
    col = dst - 'A';
    // fill in matrix with capacity, delay and cost information
    propdelay[row][col] = delay;
    propdelay[col][row] = delay;
    capacity[row][col] = cap;
    capacity[col][row] = cap;
    available[row][col] = cap;
    available[col][row] = cap;
    cost[row][col] = 1;
    cost[col][row] = 1;
  }
  fclose(fp1);

  // Read in the calls from "callworkload.dat" 
  i = 0;
  int id = 0;
  float dur, arr;
  FILE *fp2 = fopen(WORKLOAD_DATA, "r");
  while ((i = fscanf(fp2, "%f %c %c %f\n", &arr, &src, &dst, &dur)) == 4)
  {
    // create new call event
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

  // sort topology list in order for the event to arrive in order 
  workLoad.sort(compareEvent);

  //Print final report for each policy 
  cout << "Policy\tCalls\tSuccess\tBlocked(%)\tHops\tDelay"<< endl;
  cout <<"  -----------------------------------------------" << endl; 
  simulatePolicy("SHPF", cost, workLoad); // Run Simulation for Short Hop Path First
  simulatePolicy("SDPF", propdelay, workLoad); // Run Simulation for Short Delay Path First 
  simulatePolicy("LLP", available, workLoad); // Run Simulation for Short Delay Path First
}