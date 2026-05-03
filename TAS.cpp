#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <vector>
#include <list>
#include <algorithm>
#include <random>
#include <string>
#include <atomic>
#include <numeric>
using namespace std;

struct params {
    int N;
    int numOps;
    double lamda1;
    double lamda2;
    double pl;
    double pi;
};

class TASLock {
    private:
        atomic <bool> state;
    public:
        TASLock() {
            state.store(false, memory_order_seq_cst);
        }
        void lock() {
            while (state.exchange(true)); //Spin
        }
        void unlock() {
            state.store(false, memory_order_seq_cst);
        }
};

list <int> myList; // Shared Linked List
vector <long long> thrTimes; // Record Thread Execution Time
vector <long long> thrLookUpWaitTimes; // Record Thread LookUp Wait Time
vector <long long> thrUpdateWaitTimes; // Record Thread Update Wait Time

params readInputFile(const string &fileName);
long long getCurrTime();
void threadWork(int id, int numOps, double lamda1, double lamda2, double pl, double pi, TASLock& lckObj);

int main() {
    try {
        params p = readInputFile("inp-params.txt");
        long long sum;

        thrTimes.assign(p.N, 0);
        thrLookUpWaitTimes.assign(p.N, 0);
        thrUpdateWaitTimes.assign(p.N, 0);

        TASLock lckObj;
        vector <thread> threads;
        threads.reserve(p.N);

        for (int i = 0; i < p.N; i++) {
            threads.emplace_back(threadWork, i, p.numOps, p.lamda1, p.lamda2, p.pl, p.pi, ref(lckObj));
        }

        for (int i = 0; i < p.N; i++) {
            threads[i].join();
        }

        cout << "All threads completed." << endl;
        cout << "Thread execution times: " << endl;
        for (int i = 0; i < p.N; i++) {
            cout << "Thread # " << (i+1) << " completed in : " << thrTimes[i] << " micro sec" << endl;
        }
        sum = accumulate(thrLookUpWaitTimes.begin(), thrLookUpWaitTimes.end(), 0LL);
        cout << "\nAverage waiting time for LookUp operation: " << (sum / p.N) << " micro sec"<< endl;
        sum = accumulate(thrUpdateWaitTimes.begin(), thrUpdateWaitTimes.end(), 0LL);
        cout << "\nAverage waiting time for Update operation: " << (sum / p.N) << " micro sec" << endl;

    } catch(const exception &e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    return 0;
}

params readInputFile(const string &fileName) {
    params p;
    ifstream fin(fileName);
    if (!fin) {
        throw runtime_error("Error in Opening File");
    }

    string key;
    while (fin >> key) {
        if (key == "N") fin >> p.N;
        else if (key == "K") fin >> p.numOps;
        else if (key == "lamda1") fin >> p.lamda1;
        else if (key == "lamda2") fin >> p.lamda2;
        else if (key == "pl") fin >> p.pl;
        else if (key == "pi") fin >> p.pi;
    }
    return p;
}

long long getCurrTime() {
    return chrono::duration_cast<chrono::microseconds>(chrono::steady_clock::now().time_since_epoch()).count();
}

void threadWork(int id, int numOps, double lamda1, double lamda2, double pl, double pi, TASLock& lckObj) {
    int turn;
    double sleepT1, sleepT2;
    long long reqTime, actTime, exitReqTime, actExitTime;

    string logFileName = "Thread_LogFile(TAS)_" + to_string(id+1) + ".log";
    ofstream fout(logFileName);

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> toss(0, 1);
    uniform_int_distribution<int> distr(1, 10000);
    exponential_distribution<double> expDist1(1.0 / lamda1);
    exponential_distribution<double> expDist2(1.0 / lamda2);

    int lookUpCounter = pl * numOps;
    int updateCounter = numOps - lookUpCounter;
    int insertCounter = pi * updateCounter;
    int deleteCounter = updateCounter - insertCounter;

    for (int i = 1; i <= numOps; i++) {
        turn = toss(gen);
        if ((turn == 0 && lookUpCounter > 0) || (lookUpCounter > 0 && updateCounter == 0)) {
            reqTime = getCurrTime();
            fout << i << " th LookUp Request at " << reqTime << " micro sec by thread #" << (id+1) << endl;
            lckObj.lock(); // Lock Acquisition
            actTime = getCurrTime();
            fout << i << " th LookUp at " << actTime << " micro sec by thread #" << (id+1) << endl;

            auto val = find(myList.begin(), myList.end(), distr(gen)); // LookUp Operation

            sleepT1 = expDist1(gen);         
            this_thread::sleep_for(chrono::microseconds((int)sleepT1));

            exitReqTime = getCurrTime();
            fout << i << " th LookUp Exit Request at " << exitReqTime << " micro sec by thread #" << (id+1) << endl;
            lckObj.unlock(); // Lock Release
            actExitTime = getCurrTime();
            fout << i << " th LookUp Exit at " << actExitTime << " micro sec by thread #" << (id+1) << endl;
            thrTimes[id] += (actExitTime - reqTime);
            thrLookUpWaitTimes[id] += (actTime - reqTime);
            --lookUpCounter;
        } else {
            turn = toss(gen);
            if ((turn == 0 && insertCounter > 0) || (insertCounter > 0 && deleteCounter == 0)) {
                reqTime = getCurrTime();
                fout << i << " th Insert Request at " << reqTime << " micro sec by thread #" << (id+1) << endl;
                lckObj.lock(); // Lock Acquisition
                actTime = getCurrTime();
                fout << i << " th Insert at " << actTime << " micro sec by thread #" << (id+1) << endl;

                myList.push_back(distr(gen)); // Insert Operation

                sleepT1 = expDist1(gen);         
                this_thread::sleep_for(chrono::microseconds((int)sleepT1));

                exitReqTime = getCurrTime();
                fout << i << " th Insert Exit Request at " << exitReqTime << " micro sec by thread #" << (id+1) << endl;
                lckObj.unlock(); // Lock Release
                actExitTime = getCurrTime();
                fout << i << " th Insert Exit at " << actExitTime << " micro sec by thread #" << (id+1) << endl;
                thrTimes[id] += (actExitTime - reqTime);
                thrUpdateWaitTimes[id] += (actTime - reqTime);
                --insertCounter;
            } else {
                reqTime = getCurrTime();
                fout << i << " th Delete Request at " << reqTime << " micro sec by thread #" << (id+1) << endl;
                lckObj.lock(); // Lock Acquisition
                actTime = getCurrTime();
                fout << i << " th Delete at " << actTime << " micro sec by thread #" << (id+1) << endl;
                
                if (!myList.empty())
                    myList.pop_back(); // Delete Operation

                sleepT1 = expDist1(gen);         
                this_thread::sleep_for(chrono::microseconds((int)sleepT1));

                exitReqTime = getCurrTime();
                fout << i << " th Delete Exit Request at " << exitReqTime << " micro sec by thread #" << (id+1) << endl;
                lckObj.unlock(); // Lock Release
                actExitTime = getCurrTime();
                fout << i << " th Delete Exit at " << actExitTime << " micro sec by thread #" << (id+1) << endl;
                thrTimes[id] += (actExitTime - reqTime);
                thrUpdateWaitTimes[id] += (actTime - reqTime);               
                --deleteCounter;
            }
            --updateCounter;
        }
        sleepT2 = expDist2(gen);
        this_thread::sleep_for(chrono::microseconds((int)sleepT2));
    }
    fout.close();
}

