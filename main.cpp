#include <iostream>
#include <vector>
#include <map>
#include <string>
#include<sstream>

using namespace std;

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

int main()
{
    map<char, map<char, vector<int> > > networkTopology;
    map<char, map<char, vector<int> > >::iterator networkIt;
    map<string, int> capacity;
    map<string, int> available;
    vector<Event> workLoad;

    FILE *fp1 = fopen("topology.dat", "r");
    int i = 0;
    char src, dst;
    int delay, cap;
    string link;

    while ((i = fscanf(fp1, "%c %c %d %d\n", &src, &dst, &delay, &cap)) == 4)
    {
        // keep track of link capacity and availablity 
        link = "";
        link += src;
        link += "-";
        link += dst;

        capacity.insert(pair<string,int>(link, cap));
        available.insert(pair<string,int>(link, cap));

        if (networkTopology.find(src) == networkTopology.end())
        {
            map<char, vector<int> > tmpMap;
            vector<int> tmpVect;

            // add propagation delay, and capacity to vector
            tmpVect.push_back(delay);
            tmpVect.push_back(cap);
            tmpMap.insert(pair<char, vector<int> >(dst, tmpVect));

            // create new entry in topology map 
            networkTopology.insert(pair<char, map<char, vector<int> > >(src, tmpMap));
        }
        else
        {
            vector<int> tmpVect;
            // add propagation delay, and capacity to vector
            tmpVect.push_back(delay);
            tmpVect.push_back(cap);

            // insert new neighbour
            networkTopology[src].insert(pair<char, vector<int> >(dst, tmpVect));
        }

        
    }
    fclose(fp1);


    /* Next read in the calls from "callworkload.dat" and set up events */
    i = 0; 
    float dur, arr;
    FILE *fp2 = fopen("callworkload.dat", "r");
    while ((i = fscanf(fp2, "%f %c %c %f\n", &arr, &src, &dst, &dur)) == 4){
        //cout << "Duration: " << arr <<endl;
        EventList tmpEvent; 
        tmpEvent.event_time = arr;
        tmpEvent.source = src;
        tmpEvent.destination = dst;
        tmpEvent.duration = dur;
        workLoad.push_back(tmpEvent);
    }
    fclose(fp2);

    /* Now simulate the call arrivals and departures */
    // while (numevents > 0)
    // {
    //     /* Get information about the current event */
    //     printf("Event of type %d at time %8.6f (call %d from %c to %c)\n", type, start, callid, src, dst);

    //     if (type == CALL_ARRIVAL)
    //         if (RouteCall(src, dst, callid) > 0)
    //             success++;
    //         else
    //             blocked++;
    //     else
    //         ReleaseCall(src, dst, callid);
    // }
    /* Print final report here */

    // for(auto const &ent1 : networkTopology) {
    //     auto const &outer_key = ent1.first;
    //     cout << outer_key<< endl;
    // }

}