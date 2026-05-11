/*There are N classes 1, 2, . . ., N that need to be scheduled for the final exam. Each class must be assigned to a time-slot and a room.
There are M rooms 1, 2, …, M that can be used for scheduling the exam. Each room i has capacity c(i) (number of places of the room)
Each day is divided into 4 slots 1, 2, 3, 4.
Each class i has number of students d(i) (i = 1,..., N).
Among N classes, there are K pairs of classes (i, j) in which class i and class j have the same student participating in the exam. It means that these 2 classes cannot be scheduled in the same time-slot.
Objective: Compute the exam time-table such that the number of days used is minimal.

A solution is represented by 2 array s and r in which s[i] is the start slot and r[i] is the room of course i (NOTE: slots of different days are indexed as consecutive numbers 1, 2, 3, 4, 5, 6, 7, 8, ...)
Input
Line 1: contains N
Line 2: contains d1, d2, …, dN
Line 3: contains M
Line 4: contains c1, c2, …, cM
Line 5: contains K
Line 5 + k (k = 1,…, K): contains 2 integers i and j (2 courses having a same student registerd,  these courses cannot be scheduled in the same slot)
Output
Each line i ( i = 1, 2, . . ., N): contains 3 integer i, s[i], and r[i] */

/*
Dev's note:
Delete the 0.7 probability
Put the hyperparameters outside the gen neighbour function, don't hardcode them into the function
*/

#include <bits/stdc++.h>

using namespace std;

//randomizer
mt19937 rng(random_device{}());

/*
    N: number of classes
    M: number of rooms
    K: number of conflicts
*/
int N, M, K;
int currentMaxSlot = 0;
int currentMaxDays = 0;
double bestMaxDays = DBL_MAX;
const int HORIZON_EXTENSION = 4;
pair<vector<int>, vector<int>> bestSlotRoom;
pair<vector<int>, vector<int>> initialSlotRoom;
double initialMaxSlot;
double initialMaxDays;
vector<int> c,d,s,r;
vector<set<int>> conflict;
vector<vector<bool>> occupied; //track room - time slot occupation 

void input(){
    cin >> N >> M;

    //input classes' info
    d = vector<int>(N + 1);
    for(int i = 1; i <= N; i++){
        cin >> d[i];
    } 

    //input rooms' info
    c = vector<int>(M + 1);
    for(int j = 1; j <= M; j++){
        cin >> c[j];
    }

    //input conflicts
    conflict = vector<set<int>>(N + 1);
    cin >> K;
    int x, y;
    for(int k = 0; k < K; k++){
        cin >> x >> y;
        conflict[x].insert(y);
        conflict[y].insert(x);
    }

    //initiate slots and rooms
    s = vector<int>(N + 1);
    r = vector<int>(N + 1);

    //initiate slot room usage tracking matrix
    occupied = vector<vector<bool>>(N + 1, vector<bool>(M + 1)); //at most only N time slots should be used
}

//initialize a solution

void init(){
    //sort classes in number of conflicts descending
    vector<int> order(N);
    iota(order.begin(), order.end(), 1);
    sort(order.begin(), order.end(), [&](int a, int b){
        return conflict[a].size() > conflict[b].size(); 
    });

    //assigning time slot and room for each class 
    for(int i : order){
        for(int ts = 1; ts <= N; ts++){ //time slot
            bool tsOk = true;
            for(int j : conflict[i]){
                if(s[j] == ts){
                    tsOk = false;
                    break;
                }
            }
            if(!tsOk) continue;
            //found a time slot with no conflict
            
            bool foundRoom = false;
            for(int rm = 1; rm <= M; rm++){
                if(occupied[ts][rm] || c[rm] < d[i]) continue;
                s[i] = ts;
                r[i] = rm;
                occupied[ts][rm] = true;
                foundRoom = true;
                break;
            }
            if(foundRoom) break;
            else continue;
        }
    }
    initialSlotRoom = {s, r};
    initialMaxSlot = *max_element(initialSlotRoom.first.begin(), initialSlotRoom.first.end());
    initialMaxDays = (initialMaxSlot + 3)/4;
}

//neighbour generation
/*
pick a random class i:
    1. rearrange its time slot:
        ts to ts' such that:
            ts' must not be shared with any conflicting classes
            there has to be a room with enough capacity free at ts'
    2. rearrange its room
        rm to rm' such that:
            rm' has enough capacity
            rm' is free at ts
*/

struct Move{
    int classId;
    int newSlot;
    int newRoom;
    Move(int i, int ts, int rm): classId(i), newSlot(ts), newRoom(rm){}; 
};

//this function creates a random set of <classId, slot, room> and returns a random one
Move generateNeighbour(){
    //pick random classId
    uniform_int_distribution classDist(1, N);
    int classId = classDist(rng);

    //create a set of <slot, room> for classId
    vector<pair<int, int>> candidates;

    int horizon = min(*max_element(s.begin() + 1, s.end()) + HORIZON_EXTENSION, N); //extend search for slot to up to 1 day
    vector<bool> conflictedSpots = vector<bool>(horizon + 1);

    for(int j : conflict[classId]){
        conflictedSpots[s[j]] = true;
    }

    for(int ts = 1; ts <= horizon; ts++){
        if(conflictedSpots[ts]) continue;

        //try rooms, starting from same room
        if(!occupied[ts][r[classId]] && c[r[classId]] >= d[classId]){
            candidates.push_back({ts, r[classId]});
        }
        for(int rm = 1; rm <= M; rm++){
            if(rm == r[classId]) continue;
            if(!occupied[ts][rm] && c[rm] >= d[classId]){
                candidates.push_back({ts, rm});
            }
        }
    }

    //no candidate? No neighbour then, stand still.
    if(candidates.empty()) return(Move(classId, s[classId], r[classId]));

    //yes candidate? pick random one and move there
    uniform_int_distribution candidateDist(0, (int)candidates.size() - 1);
    auto [newTs, newRm] = candidates[candidateDist(rng)];

    return Move(classId, newTs, newRm);
}

//simulated annealing hyperparameters
double T = 100.0;
const double min_T = 0.01;
const double cooling = 0.9999;
const int MAX_ITERATION = 1000000;

//simulated annealing function
uniform_real_distribution<double> prob(0.0, 1.0);
void simulatedAnnealing(){
    init();
    currentMaxSlot = *max_element(s.begin() + 1, s.end());
    currentMaxDays = (currentMaxSlot + 3)/4;

    for(int iter = 0; iter < MAX_ITERATION && T > min_T; iter++){
        Move mv = generateNeighbour();

        //apply move temporarily
        int oldSlot = s[mv.classId];
        int oldRoom = r[mv.classId];

        s[mv.classId] = mv.newSlot;
        r[mv.classId] = mv.newRoom;

        int newSlot = mv.newSlot;
        int candidateMaxSlot = currentMaxSlot;
        int candidateMaxDays = currentMaxDays;

        //if newSlot >= currentMaxSlot then update currentMaxSlot
        if(newSlot >= currentMaxSlot){
            candidateMaxSlot = newSlot;
            candidateMaxDays = (candidateMaxSlot + 3)/4;
        }
        
        //if newSlot < currentMaxSlot and we free the maxslot, then rescan and update
        else{
            if(oldSlot == currentMaxSlot){ 
                candidateMaxSlot = *max_element(s.begin() + 1, s.end());
                candidateMaxDays = (candidateMaxSlot + 3)/4;
            }
        }

        int delta = candidateMaxSlot - currentMaxSlot;
        //move to neighbour if delta < 0, otherwise only move with a probability of exp(-delta/T)
        if(delta < 0 || prob(rng) < exp(-delta/T)){
            //accept
            currentMaxSlot = candidateMaxSlot;
            currentMaxDays = candidateMaxDays;
            //update other stuff
            occupied[oldSlot][oldRoom] = false;
            occupied[mv.newSlot][mv.newRoom] = true;
        }
        else{
            //revert
            s[mv.classId] = oldSlot;
            r[mv.classId] = oldRoom;
        }

        if(currentMaxDays < bestMaxDays){
            bestMaxDays = currentMaxDays;
            bestSlotRoom = {s, r};
        }

        T *= cooling;
    }
}

int main(){
    input();
    simulatedAnnealing();
    for(int i = 1; i <= N; i++){
        cout << i << " " << (bestSlotRoom.first)[i] << " " << (bestSlotRoom.second)[i] << endl;
    }
}
