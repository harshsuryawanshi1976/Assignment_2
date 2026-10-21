// QUESTION 1: INTER-PROCESS COMMUNICATION USING PIPE
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstring>
#include <cctype>
using namespace std;

int main()
{
    int fd[2];
    char message[] = "hello operating system";
    char received[100] = {0};

    if (pipe(fd) == -1) {
        cout << "Pipe creation failed\n";
        return 1;
    }

    pid_t pid = fork();

    if (pid == 0) {
        close(fd[1]);
        read(fd[0], received, sizeof(received) - 1);
        close(fd[0]);

        for (int i = 0; received[i] != '\0'; i++)
            received[i] = toupper((unsigned char)received[i]);

        cout << "Child received and converted: " << received << endl;
    }
    else if (pid > 0) {
        close(fd[0]);
        write(fd[1], message, strlen(message) + 1);
        close(fd[1]);
        wait(NULL);
    }
    else {
        cout << "Process creation failed\n";
        return 1;
    }

    return 0;
}

/*
SAMPLE OUTPUT:
Child received and converted: HELLO OPERATING SYSTEM
*/


// QUESTION 2: PRODUCER-CONSUMER USING SEMAPHORES
#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
using namespace std;

const int SIZE = 3;
int buffer[SIZE];
int in = 0, out = 0, item = 1;

sem_t emptySlots, fullSlots;
pthread_mutex_t lockBuffer;

void* producer(void*)
{
    for (int i = 0; i < 5; i++) {
        sem_wait(&emptySlots);
        pthread_mutex_lock(&lockBuffer);

        buffer[in] = item;
        cout << "Producer produced: " << item << endl;
        item++;
        in = (in + 1) % SIZE;

        pthread_mutex_unlock(&lockBuffer);
        sem_post(&fullSlots);
        usleep(100000);
    }
    return nullptr;
}

void* consumer(void*)
{
    for (int i = 0; i < 5; i++) {
        sem_wait(&fullSlots);
        pthread_mutex_lock(&lockBuffer);

        int value = buffer[out];
        cout << "Consumer consumed: " << value << endl;
        out = (out + 1) % SIZE;

        pthread_mutex_unlock(&lockBuffer);
        sem_post(&emptySlots);
        usleep(150000);
    }
    return nullptr;
}

int main()
{
    pthread_t p, c;

    sem_init(&emptySlots, 0, SIZE);
    sem_init(&fullSlots, 0, 0);
    pthread_mutex_init(&lockBuffer, nullptr);

    pthread_create(&p, nullptr, producer, nullptr);
    pthread_create(&c, nullptr, consumer, nullptr);

    pthread_join(p, nullptr);
    pthread_join(c, nullptr);

    sem_destroy(&emptySlots);
    sem_destroy(&fullSlots);
    pthread_mutex_destroy(&lockBuffer);
    return 0;
}

/*
SAMPLE OUTPUT (the order of lines can vary):
Producer produced: 1
Consumer consumed: 1
Producer produced: 2
Consumer consumed: 2
Producer produced: 3
Consumer consumed: 3
Producer produced: 4
Consumer consumed: 4
Producer produced: 5
Consumer consumed: 5

Compile on Linux:
g++ q2.cpp -pthread -o q2
*/


// QUESTION 3: BANKER'S ALGORITHM
#include <iostream>
using namespace std;

int main()
{
    int n = 5, m = 3;

    int allocation[5][3] = {
        {0, 1, 0}, {2, 0, 0}, {3, 0, 2},
        {2, 1, 1}, {0, 0, 2}
    };

    int maximum[5][3] = {
        {7, 5, 3}, {3, 2, 2}, {9, 0, 2},
        {2, 2, 2}, {4, 3, 3}
    };

    int available[3] = {3, 3, 2};
    int need[5][3], finish[5] = {0};
    int safeSequence[5], count = 0;

    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++)
            need[i][j] = maximum[i][j] - allocation[i][j];

    cout << "Need Matrix:\n";
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++)
            cout << need[i][j] << " ";
        cout << endl;
    }

    while (count < n) {
        bool found = false;

        for (int i = 0; i < n; i++) {
            if (finish[i] == 0) {
                bool canRun = true;

                for (int j = 0; j < m; j++) {
                    if (need[i][j] > available[j]) {
                        canRun = false;
                        break;
                    }
                }

                if (canRun) {
                    for (int j = 0; j < m; j++)
                        available[j] += allocation[i][j];

                    safeSequence[count++] = i;
                    finish[i] = 1;
                    found = true;
                }
            }
        }

        if (!found)
            break;
    }

    if (count == n) {
        cout << "System is in a SAFE state.\n";
        cout << "Safe sequence: ";
        for (int i = 0; i < n; i++) {
            cout << "P" << safeSequence[i];
            if (i < n - 1) cout << " -> ";
        }
        cout << endl;
    } else {
        cout << "System is NOT in a safe state.\n";
    }

    return 0;
}

/*
SAMPLE OUTPUT:
Need Matrix:
7 4 3
1 2 2
6 0 0
0 1 1
4 3 1
System is in a SAFE state.
Safe sequence: P1 -> P3 -> P4 -> P0 -> P2
*/
