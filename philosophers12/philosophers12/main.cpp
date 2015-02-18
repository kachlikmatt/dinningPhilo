// Win 32 app for the Dining Philosophers
// Shell by S. Renk  2012
//written by Matthew Kachlik
#include <iostream>
#include <Windows.h>
#include <conio.h>
#include <stdio.h>
#include <time.h>

using namespace std;

// shared vars for the philos - you can add more
enum pState { GONE, PRESENT, THINKING, HUNGRY, EATING, TALKING };
int philoCount = 0;                    // # of active philosophers
int forks[5] = { 1, 1, 1, 1, 1 };             // forks on table  1=present 0=gone
int usedForkLast[5] = { -1, -1, -1, -1, -1 }; // no one has used them last
pState philoState[5] = { GONE };           // 1-present 2-thinking 3-eating
int eatCount[5] = { 0 };                  // holds the # of time a philo has eaten
clock_t startTime, endTime;                       // used for run time calculation

// locks to create mutual exclusion
HANDLE mutex = CreateMutex(NULL, FALSE, NULL);
HANDLE forkLock[5];                  // these are the forks now

// ********** Create 5 philosophers & set them to eating ****************
void main()
{ // Set up 5 threads
	HANDLE pThread[5];                  // 5 philosopher tbreads
	DWORD pThreadID[5];                 // PID of thread
	DWORD WINAPI philosopher(LPVOID);   // code for the 5 philos

	cout << "Dining Philosophers - S. Renk\n\n";
	startTime = clock();              // start the timer


	// start 5 philosopher threads
	for (int philoNbr = 0; philoNbr < 5; philoNbr++)
	{
		pThread[philoNbr] = CreateThread(NULL, 0, philosopher, NULL, 0, &pThreadID[philoNbr]);
		forkLock[philoNbr] = CreateMutex(NULL, FALSE, NULL);
	}

	WaitForMultipleObjects(5, pThread, true, INFINITE); // wait for philos to finish
	endTime = clock();              // start the timer
	cout << "\n\nRun time = " << ((endTime - startTime) * 1000) / CLOCKS_PER_SEC << "ms\n";
	cout << "press CR to end."; while (_getch() != '\r');
	return;
}

// *************** The Philosopher **************************
DWORD WINAPI philosopher(LPVOID)
{
	int me;// holds my philo #
	int left, right;                      // philo on left & right
	int leftFork = 0, rightFork = 0;      // remember to reassign these to the correct values

	// who am I?
	WaitForSingleObject(mutex, INFINITE); // lock the lock
	me = philoCount++;
	ReleaseMutex(mutex);                  // unlock the lock
	philoState[me] = PRESENT;             // I’m here!
	left = (me + 1) % 5; right = (me + 4) % 5;     // define neighbors

	// assign forks
	leftFork = me;
	rightFork = (me+4)%5;



	// wait for everybody to show up to dinner
	while (philoCount < 5) Sleep(0);      // preempt self till everybody gets here



	WaitForSingleObject(mutex, INFINITE); // lock the lock
	cout << "Philosopher # " << me << " ready to dine." << endl;
	ReleaseMutex(mutex);                  // unlock the lock


	while (eatCount[me] < 1000) // eat 100 times
	{
		// think for awhile
		philoState[me] = THINKING;
		Sleep((DWORD)rand() % 20);

		philoState[me] = HUNGRY;
		if(me != 3)
		{
			while(usedForkLast[rightFork] == me && philoState[right] == HUNGRY) Sleep(0);
			WaitForSingleObject(forkLock[rightFork], INFINITE); // lock the lock
			forks[rightFork]--;

			while(usedForkLast[leftFork] == me && philoState[left] == HUNGRY) Sleep(0);
			WaitForSingleObject(forkLock[leftFork], INFINITE); // lock the lock
			// if (you can get forks) eat for awhile
			// pick up forks
			forks[leftFork]--;
		}
		else
		{
			while(usedForkLast[leftFork] == me && philoState[left] == HUNGRY) Sleep(0);
			WaitForSingleObject(forkLock[leftFork], INFINITE); // lock the lock
			// if (you can get forks) eat for awhile
			// pick up forks
			forks[leftFork]--;

			while(usedForkLast[rightFork] == me && philoState[right] == HUNGRY) Sleep(0);
			WaitForSingleObject(forkLock[rightFork], INFINITE); // lock the lock
			forks[rightFork]--;
			
		}
		// eat for a while
		philoState[me] = EATING;

		// error check
		WaitForSingleObject(mutex, INFINITE); // lock the lock
		if (philoState[left] == EATING || philoState[right] == EATING)
			cout << "******** Eating Error ********" << endl;

		if (forks[rightFork] || forks[leftFork])
			cout << "******** Fork Error ********”  end";
		ReleaseMutex(mutex);                  // unlock the lock

		eatCount[me]++;
		Sleep((DWORD)rand() % 20);
		usedForkLast[leftFork] = me;
		usedForkLast[rightFork] = me;
		philoState[me] = TALKING;
		// return forks
		forks[rightFork]++;
		ReleaseMutex(forkLock[rightFork]);                  // unlock the lock 
		forks[leftFork]++;
		ReleaseMutex(forkLock[leftFork]);                  // unlock the lock
		
		
		// talk for a while
		
		Sleep(rand() % 10);
	}

	WaitForSingleObject(mutex, INFINITE); // lock the lock	
	cout << "philosopher # " << me << " is leaving now." << endl;
	for (int i = 0; i<5; cout << eatCount[i++] << " "); cout << endl;
	ReleaseMutex(mutex);                  // unlock the lock
	philoState[me] = GONE;
	return 0;
}