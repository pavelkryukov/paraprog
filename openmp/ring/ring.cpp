/**
 * ring.cpp
 *
 * Send/Recv messages with pthreads in ring using pthreads
 *
 * @author pikryukov
 * @version 1.0
 *
 * e-mail: kryukov@frtk.ru
 *
 * Copyright (C) Kryukov Pavel 2012
 * for MIPT MPI course.
 */

#include <cstdlib>

#include <iostream>
#include <vector>

#include <pthread.h>

/**
 * Mutex class wrapper
 */
class Mutex
{
private:
    friend class Cond;
    pthread_mutex_t m;
public:
    inline Mutex()  { pthread_mutex_init(&m, NULL); }
    inline ~Mutex() { pthread_mutex_destroy(&m); }
    inline int Lock()    { return pthread_mutex_lock(&m); }
    inline int Unlock()  { return pthread_mutex_unlock(&m); }
    inline int TryLock() { return pthread_mutex_trylock(&m); }
};

/**
 * Condition class wrapper
 */
class Cond
{
private:
    pthread_cond_t c;
public:
    inline Cond()  { pthread_cond_init(&c, NULL); }
    inline ~Cond() { pthread_cond_destroy(&c); }
    inline int Wait(Mutex& mutex)
    {
        return pthread_cond_wait(&c, &mutex.m);
    }
    inline int TimedWait(Mutex& mutex, const timespec* time)
    {
        return pthread_cond_timedwait(&c, &mutex.m, time);
    }
    inline int Signal() { return pthread_cond_signal(&c); }
    inline int Broadcast() { return pthread_cond_broadcast(&c); }
};

/**
 * Simple port between two threads
 */
template<typename T>
class Port
{
private:
    Mutex mutex;
    T data;
    bool isValid;
    Cond full;
    Cond free;
public:
    Port() : isValid(false) { }

    void Write(const T& what)
    {
        mutex.Lock();
        while (isValid)
            free.Wait(mutex);

        data = what;
        isValid = true;
        full.Broadcast();
        mutex.Unlock();
    }

    T Read() 
    {
        mutex.Lock();
        while(!isValid)
            full.Wait(mutex);

        T result = data;
        isValid = false;
        free.Broadcast();

        mutex.Unlock();
        return result;
    }
};

/**
 * Thread unit
 */
class Unit
{
private:
    Port<unsigned>* readPort;
    Port<unsigned>* writePort;
    static Mutex printMutex; ///< Mutex for locking stdout
    int rank;
    unsigned maxAmount;
public:
    Unit(int rank, int maxAmount)
        : readPort(NULL)
        , writePort(NULL)
        , rank(rank)
        , maxAmount(maxAmount)
    { }
    
    ~Unit() { delete readPort; }
    
    inline int GetRank() { return rank; }
    inline unsigned GetAmount() { return maxAmount; }
    
    static void Connect(Unit* to, Unit* from)
    {
        to->readPort = from->writePort = new Port<unsigned>;
    }

    /// Method for forwarding from previous to next
    void Forward()
    {
        unsigned i = readPort->Read() + 1;
        printMutex.Lock();
        std::cout << "[" << rank << "] received data (" << i << " time)" << std::endl;
        printMutex.Unlock();
        writePort->Write(i);
    }

    inline void Pulse() { writePort->Write(0); }
    inline void Catch() { readPort->Read(); }
};

Mutex Unit::printMutex = Mutex();

void* unitRun(void* ptr)
{
    Unit* unit = reinterpret_cast<Unit*>(ptr);
    int rank = unit->GetRank();
    unsigned amount = unit->GetAmount();
    
    // Creating pulse
    if (rank == 0)
        unit->Pulse();
    
    // Pulse is going around the threads
    for (size_t i = 0; i < amount; ++i)
        unit->Forward();
    
    // Catching pulse
    if (rank == 1)
        unit->Catch();
        
    return static_cast<void*>(0);
}

void ring(unsigned size, int counter)
{
    // Creating units (thread tasks)
    std::vector<Unit*> units(size);
    for (size_t i = 0; i < size; ++i)
        units[i] = new Unit(i, counter);

    // Units connection
    for (size_t i = 0; i < size - 1; ++i)
        Unit::Connect(units[i + 1], units[i]);

    // Looping
    Unit::Connect(units[0], units[size - 1]);

    // Creating threads
    std::vector<pthread_t> threads(size);
    for (size_t i = 0; i < size; ++i)
        pthread_create(&threads[i], 0, unitRun, reinterpret_cast<void*>(units[i]));
        
    for (size_t i = 0; i < size; ++i)
    {
        pthread_join(threads[i], NULL);
        delete units[i];
    }
}
    
int main(int argc, char** argv)
{
    if (argc != 3)
    {
        std::cerr << "Syntax error! First argument is number of threads,"
                  << " second is amount of operations" << std::endl;
        return 1;
    }

    unsigned size = strtoul(argv[1], NULL, 0);
    int counter = strtoul(argv[2], NULL, 0);
    
    ring(size, counter);

    return 0;
}
