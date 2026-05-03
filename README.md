# Parallel_And_Concurrent_Programming_Assignment_4
================================================================================
  CS5300 -- Parallel and Concurrent Programming
  Programming Assignment 4: Spin Lock Performance Evaluation
  README
================================================================================

COMPILATION
-----------
  Compile each lock implementation with:

    g++ -std=c++17 -O2 -pthread -o tas    TAS.cpp
    g++ -std=c++17 -O2 -pthread -o ttas   TTAS.cpp
    g++ -std=c++17 -O2 -pthread -o clh    CLH.cpp
    g++ -std=c++17 -O2 -pthread -o mcs    MCS.cpp


INPUT FILE FORMAT
-----------------
  The program reads parameters from "inp-params.txt" in the current directory.
  The file uses key-value pairs, one per line:

    N       <number of threads>
    K       <number of operations per thread>
    lamda1  <mean delay inside critical section, microseconds>
    lamda2  <mean delay outside critical section, microseconds>
    pl      <lookup probability, float in [0,1]>
    pi      <insert probability among updates, float in [0,1]>

  Example (inp-params.txt):
  --------------------------
    N       16
    K       50
    lamda1  1
    lamda2  2
    pl      0.6
    pi      0.9

  Notes:
    - Keys are case-sensitive (use exactly N, K, lamda1, lamda2, pl, pi)
    - Order of keys in the file does not matter
    - Float values use decimal notation (e.g., 0.6, not .6)


EXECUTION
---------
  After compilation, place inp-params.txt in the same directory and run:

    ./tas
    ./ttas
    ./clh
    ./mcs

  Each binary will:
    1. Read parameters from inp-params.txt
    2. Launch N threads, each performing K operations on a shared linked list
    3. Print per-thread execution times and average wait times to stdout
    4. Write per-thread log files to the current directory:
         Thread_LogFile(TAS)_1.log  ... Thread_LogFile(TAS)_N.log
         Thread_LogFile(TTAS)_1.log ... Thread_LogFile(TTAS)_N.log
         Thread_LogFile(CLH)_1.log  ... Thread_LogFile(CLH)_N.log
         Thread_LogFile(MCS)_1.log  ... Thread_LogFile(MCS)_N.log


SAMPLE OUTPUT
-------------
  All threads completed.
  Thread execution times:
  Thread # 1 completed in : 12340 micro sec
  Thread # 2 completed in : 11980 micro sec
  ...

  Average waiting time for LookUp operation: 52 micro sec
  Average waiting time for Update operation: 61 micro sec


LOG FILE FORMAT
---------------
  Each log file records four events per operation:

    i th LookUp Request at <timestamp> micro sec by thread #<id>
    i th LookUp at <timestamp> micro sec by thread #<id>
    i th LookUp Exit Request at <timestamp> micro sec by thread #<id>
    i th LookUp Exit at <timestamp> micro sec by thread #<id>

  (Likewise for Insert and Delete operations.)
  "Request" events may overlap across threads -- this is expected.
  Non-"Request" events inside the CS must NOT overlap (mutual exclusion).


CORRECTNESS VERIFICATION
------------------------
  To verify mutual exclusion, scan the log files and check that no two threads'
  "Xth <Op> at T1" and "Xth <Op> Exit at T2" intervals overlap for different
  thread IDs. 


EXPERIMENTAL PARAMETERS (for report reproduction)
--------------------------------------------------
  Experiment 1 -- Vary threads:
    N in {2, 4, 8, 16, 32, 64}, K=50, lamda1=1, lamda2=2, pl=0.6, pi=0.9

  Experiment 2 -- Vary operations per thread:
    N=16, K in {50,60,70,80,90,100}, lamda1=1, lamda2=2, pl=0.6, pi=0.9

  Experiment 3 -- Vary update percentage:
    N=16, K=50, lamda1=1, lamda2=2, pl in {0.9,0.8,0.7,0.6,0.5}, pi=0.9

  Each experiment is run 5 times; results are averaged.


================================================================================
