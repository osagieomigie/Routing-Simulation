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
#define TOPOLOGY_DATA   "topology.dat" //"topology_small.dat"  "top_sina.dat" 
#define WORKLOAD_DATA    "callworkload.dat" //"callworkload_small.dat" "work_sina.dat"

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
int reset[MATRIX_SIZE][MATRIX_SIZE];
list<EventList> workLoad;
map<int, stack<int> > routesUsed;

// variables needed for the average 
double shpfHops = 0, sdpfHops = 0, llpHops =0, mfcHops =0; 
double shpfDelay = 0, sdpfDelay = 0, llpDelay =0, mfcDelay=0; 

// custom link comparison; for sorting events
bool compareEvent(const EventList &first, const EventList &second)
{
  return (first.event_time < second.event_time);
}

//used in SHPF, and SDPF to determine shortest available path 
void resetCost(int graph[MATRIX_SIZE][MATRIX_SIZE], string algo){
  
  for (int i = 0; i < MATRIX_SIZE; i++){
    for (int j = 0; j < MATRIX_SIZE; j++){
      if (algo == "SHPF"){
        if (available[i][j] > 0){ // reset link availability
          graph[i][j] = 1;
        }else{
          graph[i][j] = 0;
        }
      }
      else if (algo == "SDPF"){
        if (available[i][j] > 0){
          graph[i][j] = propdelay[i][j];
        }else{
          graph[i][j] = 0;
        }
      }
      else if (algo == "RESET")
      {
        if (reset[i][j] > 0){
          graph[i][j] = reset[i][j];
        }
      }
      
    }
  }
}

// Determines if a call event can be routed 
int RouteCall(char source, char destination, EventList currentEvent, int resource[MATRIX_SIZE][MATRIX_SIZE], string algo)
{
  stack<int> path;

  //get path for each policy 
  if (algo == "SHPF"){
    resetCost(cost, "SHPF"); // reset cost graph 
    path = djikstras(resource, source, destination);
    
  }
  else if (algo == "SDPF")
  {
    int delayCost[MATRIX_SIZE][MATRIX_SIZE];
    resetCost(delayCost, "SDPF");
    path = djikstras(delayCost, source, destination);
  }
  else if (algo == "LLP"){
    path = llpDjikstras(resource, capacity, source, destination, algo); 
  }
  else if (algo == "MFC")
  {
    path = mfcDjikstras(resource, capacity, source, destination, algo); 
  }

  // no path found 
  if (path.size() == 0){
    return 0; // call is blocked 
  }

  stack<int> route = path;
  stack<int> tmproute = path;
  string alpha = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  string link;

  int srcNode = 0, dstNode=0;
  int srcNode1 = 0, dstNode1=0;

  string s, d; 

  srcNode = path.top();  s = alpha[srcNode];
  path.pop();

  //determine if there are any unavailable links in path
  while(!path.empty())
  {
    dstNode = path.top(); d = alpha[dstNode];
    path.pop();

    link = s + d;
    //cout <<"Link: " << link << endl;
    
    // check if a link taken by call is at capacity 
    if ((available[srcNode][dstNode] == 0) || (available[dstNode][srcNode] == 0)){
      return 0; // Call can't be completed 
    }
    srcNode = dstNode; s = d; // set to next node 
  }

  srcNode1 = tmproute.top(); s = alpha[srcNode1];
  tmproute.pop(); 
 
  while(!tmproute.empty()){
    dstNode1 = tmproute.top(); d = alpha[dstNode1];
    tmproute.pop();

    link = s + d;
    //cout <<" Taking from Link: " << link << endl;
    
    // restore capacity back to link 
    available[srcNode1][dstNode1] = available[srcNode1][dstNode1] - 1; 
    available[dstNode1][srcNode1] = available[dstNode1][srcNode1] - 1;

    if (algo == "SHPF"){
      shpfHops++; // increase links it went through, for shortest hop path first 
      shpfDelay += propdelay[srcNode1][dstNode1]; // store delay for links traversed through 
    }else if (algo == "SDPF"){
      sdpfHops++; // increase links it went through, for shortest delay path first 
      //cout << "link delay: " << propdelay[srcNode1][dstNode1]<< endl;
      sdpfDelay += propdelay[srcNode1][dstNode1]; // store delay for links traversed through 
      //cout << "total delay: " << sdpfDelay<< endl;
    }else if (algo == "LLP"){
      llpHops++; // increase links it went through, for Least Loaded path
      llpDelay = llpDelay + propdelay[srcNode1][dstNode1]; // store delay for links traversed through 
    }else if (algo == "MFC"){
      mfcHops++; // increase links it went through, for Least Loaded path
      mfcDelay = mfcDelay + propdelay[srcNode1][dstNode1]; // store delay for links traversed through 
    }

    srcNode1 = dstNode1; s = d;
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
    
    //cout << "release route length: " << route.size() << endl; 
    srcNode = route.top(); s = alpha[srcNode]; // get first node 
    route.pop();

    while (!route.empty())
    {
      dstNode = route.top(); d = alpha[dstNode];
      route.pop();

      link = s + d;
      //cout <<"Link Released: " << link << endl;

      // increase capacity by 1 
      //cout << " link cap  1: " << available[srcNode][dstNode] << " link cap 2: " << available[srcNode][dstNode] << endl; 

      available[srcNode][dstNode] = available[srcNode][dstNode] +1; 
      available[dstNode][srcNode] = available[dstNode][srcNode] +1;

      srcNode = dstNode; // set to previous destination 
      s = alpha[srcNode];
    }
  }
  else{
    //cout << "Invalid release call" << endl;
  }
}

// Simulates all 4 policies 
void simulatePolicy(string algo, int resource[MATRIX_SIZE][MATRIX_SIZE], list<EventList> wl){
  //Simulates the call arrivals and departures
  float time = 0.0;
  list<EventList> workLoad = wl; 
  float success = 0, blocked = 0, totalCalls = 0;
  int numevents = workLoad.size();
  float shpfAverage = 0; 

  while ((numevents > 0) && (!workLoad.empty()))
  {
    // get current event
    EventList currentEvent = workLoad.front();
    workLoad.pop_front();

    // Get information about the current event 
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

  // determine which policy to print 

  if (algo == "SHPF"){
    cout << "SHPF\t" << totalCalls << "\t" << success <<"\t" << blocked << " ("<<  (blocked/totalCalls)*100 << "%)" << "\t" << shpfHops/success << "\t " << shpfDelay/success << endl;
  }else if(algo == "SDPF"){ 
    cout << "SDPF\t" << totalCalls << "\t" << success <<"\t" << blocked << " ("<< (blocked/totalCalls)*100 << "%)"<<"\t" << sdpfHops/success << "\t " << sdpfDelay/success << endl;
  } else if(algo == "LLP"){ 
    cout << "LLP\t" << totalCalls << "\t" << success <<"\t" << blocked << " ("<< (blocked/totalCalls)*100 << "%)"<<"\t" << llpHops/success << "\t " << llpDelay/success << endl;
  } else if(algo == "MFC"){ 
    cout << "MFC\t" << totalCalls << "\t" << success <<"\t" << blocked << " ("<< (blocked/totalCalls)*100 << "%) "<<"\t " << mfcHops/success << "\t" << mfcDelay/success << endl;
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
    reset[row][col] = cap;
    reset[col][row] = cap;
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
    tmpEvent2.event_time = tmpEvent.event_time + tmpEvent.duration; // length of call
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
  cout << "Policy\tCalls\tSuccess\tBlocked(%)\tHops\t  Delay"<< endl;
  cout <<"  ---------------------------------------------------" << endl; 
  simulatePolicy("SHPF", cost, workLoad); // Run Simulation for Short Hop Path First
  routesUsed.clear(); // clear map for next round 

  simulatePolicy("SDPF", propdelay, workLoad); // Run Simulation for Short Delay Path First 
  routesUsed.clear(); // clear map for next round 

  simulatePolicy("LLP", available, workLoad); // Run Simulation for Short Delay Path First
  routesUsed.clear(); // clear map for next round 

  //simulatePolicy("MFC", available, workLoad); // Run Simulation for Maximum free circuits
  //routesUsed.clear(); // clear map for next round 
}