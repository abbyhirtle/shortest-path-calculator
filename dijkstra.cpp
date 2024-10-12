// Abby Hirtle
// This program computes the shortest distance from a given vertex
// utilizing dijkstras algorithm

using namespace std;
#include <iostream>
#include "HashTable.h"
#include "graph.h"
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <climits>

// Struct containing components for implementing Dijkstra's algorithm
struct locations
{
    string vertex;
    bool marked;
    int totalDist;
    string prevVertex;

};

void buildGraph(ifstream& file, Graph<string>& travelDest, vector<string>& verticies, HashTable<string>& Cities);
void selectorScreen(Graph<string> travelDest, vector<string>& verticies, HashTable<string> Cities);
void dijkstra(Graph<string> travelDest, string startLoc, vector<string>& verticies, bool& cycle);
bool isCycle(Graph<string> travelDest, string toLoc, locations vertexGroup[], int numVert);
bool dps(string toLoc, Graph<string> travelDest, locations vertexGroup[], bool vertexStack[], int numVert);
void initArr(locations vertexGroup[], vector<string> verticies, string startLoc, int& currLocIndex);
void deadEnd(locations vertexGroup[], int numVert);
void printLocations(vector<string>& verticies);
void printInfo(locations vertexGroup[], int index);
int findIndex(locations vertexGroup[], string key, int numVert);
int findSmallest(locations vertexGroup[], int numVert, string& newLoc);
void checkLoc(HashTable<string> Cities, string& startLoc);
bool checkMarks(locations vertexGroup[], int numVert);
int getSize(Queue<string> queue);
string stuffString(string x);
void clearScreen();

int main(int argc, char* argv[])
{
    clearScreen();
    
    //Checking for file input in command line
    if(argc ==1)
    {
        cout << "Error -- No Filename Entered! " << endl;
        return 1;
    }
    else if(argc > 2)
    {
        cout << "Error -- You Can Only Edit One File at a Time! " << endl;
        return 1;
    }

    string filename = argv[1];
    ifstream file(filename.c_str());

    if(!file)
    {
        cout << "Error -- The File Entered is Invalid! " << endl;
        return 1;
    }

    Graph<string> travelDest(50);
    vector<string> verticies;
    const string notFound = "ZZZ";
    HashTable<string> Cities(notFound, 50);

    buildGraph(file, travelDest, verticies, Cities);
    selectorScreen(travelDest, verticies, Cities);

    return 0;
}

// Reads in data from file to build graph
void buildGraph(ifstream& file, Graph<string>& travelDest, vector<string>& verticies, HashTable<string>& Cities)
{
    string currIn;

    while(getline(file, currIn))
    {   
        // Getting indexes of each variable separated by ";"
        int originIndex = currIn.find(';');
        int tripIndex = currIn.find(';',originIndex+1);

        // Assigning Variables
        string origin = currIn.substr(0, originIndex);
        string dest = currIn.substr(originIndex+1, tripIndex-originIndex-1);
        string triplen = currIn.substr(tripIndex+1, currIn.length()-tripIndex);
        int tripLen = stoi(triplen);

        // Adding Citites to graph, hashtable, and vector if they have not already been added
        if((Cities.find(origin) != origin))
        {   
            travelDest.AddVertex(origin);
            Cities.insert(origin);
            verticies.push_back(origin);
        }
        if((Cities.find(dest) != dest))
        {
            travelDest.AddVertex(dest);
            Cities.insert(dest);
            verticies.push_back(dest);
        }

        // Adding Edge
        travelDest.AddEdge(origin,dest,tripLen);
    }

    sort(verticies.rbegin(),verticies.rend());
}

// This method displays the main screen 
void selectorScreen(Graph<string> travelDest, vector<string>& verticies, HashTable<string> Cities)
{
    cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DIJKSTRA'S ALGORITHM ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
    printLocations(verticies);
    cout << "\n\nPlease input your starting vertex: ";
    string startLoc;
    getline(cin,startLoc);
    checkLoc(Cities, startLoc);
    cout << "----------------------------------------------------------------------------------------------" << endl;
    cout << stuffString("Vertex") << stuffString("Distance") << stuffString("Previous") << "\n" << endl;
    bool cycle = false;
    dijkstra(travelDest, startLoc, verticies, cycle);
    cout << "\n----------------------------------------------------------------------------------------------" << endl;
    if(cycle)
       cout << "\t\t\t\t The graph contains a cycle" << endl;
    else
        cout << "\t\t\t\t The graph does not contain a cycle" << endl;
}

// Computes shortest path from a starting index chosen by user
void dijkstra(Graph<string> travelDest, string startLoc, vector<string>& verticies, bool& cycle)
{
    int currLocIndex;
    int numVert = verticies.size();
    locations vertexGroup[numVert];
    initArr(vertexGroup, verticies, startLoc, currLocIndex);
    
    bool deadend = false;
    bool allMarked = false;
    string currLoc = startLoc;
    int toLocIndex;
    while(!allMarked)
    {
        Queue<string> vertexQ(50);
        travelDest.GetToVertices(currLoc, vertexQ);
        
        while(!vertexQ.isEmpty())
        {
            string selected;
            string toVertex = vertexQ.getFront();
            vertexQ.dequeue();

            toLocIndex = findIndex(vertexGroup, toVertex, numVert);
            int weight = travelDest.WeightIs(currLoc, toVertex);
            int tableWeight = vertexGroup[toLocIndex].totalDist;
            int lastDist = vertexGroup[currLocIndex].totalDist;

            //Update unmarked total distances if incomming weight is smaller than current
            if( !vertexGroup[toLocIndex].marked)
            {
                if(tableWeight > (weight + lastDist) )
                {
                vertexGroup[toLocIndex].totalDist = weight+lastDist;
                vertexGroup[toLocIndex].prevVertex = currLoc;
                }
            }
        }
        string newLoc;
        int newIndex = findSmallest(vertexGroup,numVert,newLoc);

        //newLoc string is empty if deadend is reached
        if(newLoc.empty())
        {
            allMarked = true;
            deadend = true;
            deadEnd(vertexGroup, numVert);
        }
        
        if(!deadend)
        {
            currLoc = newLoc;
            currLocIndex = newIndex;
            vertexGroup[newIndex].marked = true;  
            if( !deadend)
                printInfo(vertexGroup,newIndex);
            
            allMarked = checkMarks(vertexGroup, numVert);
        }
    }
    cycle = isCycle(travelDest, startLoc, vertexGroup, numVert);
}

// Sets all vertex markings to false and sends to recursive DPS method to check for cycle
bool isCycle(Graph<string> travelDest, string toLoc, locations vertexGroup[], int numVert)
{
    bool vertexStack[numVert];
    for(int i = 0; i < numVert; i++)
        vertexGroup[i].marked = false;

    for(int i = 0; i < numVert; i++)
    {
        if(!vertexGroup[i].marked)
        {
            if(dps(vertexGroup[i].vertex,travelDest,vertexGroup,vertexStack,numVert))
                return true;
        }
    }
    return false;
}

// Recursively checks for cycle using depth-first search
bool dps(string toLoc, Graph<string> travelDest, locations vertexGroup[], bool vertexStack[], int numVert)
{   
    int index = findIndex(vertexGroup, toLoc, numVert);
    if(vertexStack[index])
        return true;
    
    if(vertexGroup[index].marked)
        return false;
    
    vertexGroup[index].marked = true;
    vertexStack[index] = true;
    string vertex = vertexGroup[index].vertex;
    Queue<string> neighbors(50);
    travelDest.GetToVertices(vertex,neighbors);
    int count = getSize(neighbors);

    for(int i = 0; i < count-1; i++)
    {
        if(!dps(neighbors.getFront(), travelDest, vertexGroup, vertexStack, numVert))
            return true;
        
        neighbors.dequeue();
    }

    vertexStack[index] = false;
    return false;
}

// Initializes components in struct to default values.
// Finds index of starting location and prints info.
void initArr(locations vertexGroup[], vector<string> verticies, string startLoc, int& currLocIndex)
{   
    int size = verticies.size();

    for(int i = 0; i < size; i++)
    {
        vertexGroup[i] = {verticies.back(), false, INT_MAX, "N/A"};
        verticies.pop_back();
    }
    
    currLocIndex = findIndex(vertexGroup, startLoc, size);
    vertexGroup[currLocIndex].marked = true;
    vertexGroup[currLocIndex].totalDist = 0;
    printInfo(vertexGroup, currLocIndex);
}

//Print remaining unmarked verticies. (Unreached due to dead end)
void deadEnd(locations vertexGroup[], int numVert)
{
    locations temp[numVert];

    for(int i = 0; i < numVert; i++)
    {
        if(!vertexGroup[i].marked)
        {
            temp[i].vertex = vertexGroup[i].vertex;
            temp[i].prevVertex = "N/A";
            temp[i].totalDist = -1;
            printInfo(temp, i);
        }
    }
}

//Prints vertex with associated total distance and previous vertex.
void printInfo(locations vertexGroup[], int index)
{
    cout << stuffString( vertexGroup[index].vertex );
    string dist;
    if(vertexGroup[index].totalDist == -1)
        dist = "Not On Path";
    else
        dist = to_string(vertexGroup[index].totalDist);
    cout << stuffString(dist);
    cout << stuffString( vertexGroup[index].prevVertex ) << endl;
}

// This method takes an unsorted vector of cities and prints them in sorted order, 3 per row
void printLocations(vector<string>& verticies)
{
    cout << "A Weighted Graph Has Been Built for These " << verticies.size() << " Cities: \n" << endl;
    vector<string> temp = verticies;
    sort(temp.rbegin(),temp.rend());
    int index = 1;

    while(!temp.empty())
    {
        if(index < 4){
            cout << stuffString(temp.back());
            temp.pop_back();
            index++;
        }
        else
        {
            cout << "\n" << stuffString(temp.back());
            temp.pop_back();
            index = 2;
        }
    }
}

// Returns index of smallest unmarked weight
int findSmallest(locations vertexGroup[], int numVert, string& newLoc)
{
    int smallestWeight = INT_MAX;
    int newIndex;
    for(int i = 0; i < numVert; i++)
    {
        if(!vertexGroup[i].marked)
        {
            if(vertexGroup[i].totalDist < smallestWeight)
            {
                smallestWeight = vertexGroup[i].totalDist;
                newLoc = vertexGroup[i].vertex;
                newIndex = i;
            }
        }
    }

    return newIndex;
}

// Returns the index of a given vertex in an array of locations
int findIndex(locations vertexGroup[], string key, int numVert)
{
    int keyIndex = INT_MAX;
    int index = 0;

    while(keyIndex > numVert)
    {
        if(vertexGroup[index].vertex == key)
            keyIndex = index;
        
        index++;
    }

    return keyIndex;
}

// Checks if all verticies have been marked
bool checkMarks(locations vertexGroup[], int numVert)
{
    bool allMark = true;
    for(int i = 0; i < numVert; i++)
        if(!vertexGroup[i].marked)
            allMark = false;

    return allMark;
}

// This method checks if the user input a valid city. Continues until they input a valid city
void checkLoc(HashTable<string> Cities, string& startLoc)
{
    while(Cities.find(startLoc) != startLoc)
    {
        cout << "Starting location does not exit..." << endl;
        cout << "Please input your starting vertex: ";
        getline(cin,startLoc);
    }
}

//Returns size of queue
int getSize(Queue<string> queue){
    int count = 0;
    while(!queue.isEmpty())
    {    
        count++;
        queue.dequeue();
    }

    return count;
}

//Buffers string to be of length 31
string stuffString(string x)
{
    while(x.length() <= 31)
        x = " " + x;

    return x;
}

//Clears screen
void clearScreen()
{
  cout << "\033[2J\033[1;1H" << endl;
}
