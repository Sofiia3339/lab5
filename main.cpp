#include <iostream>
#include <string>
#include <future>
#include <thread>
#include <chrono>
#include <mutex>

using namespace std;
mutex cout_mutex;

void safe_print(const string& text) {
    lock_guard<mutex> lock(cout_mutex);
    cout << text << endl;
}

void quick(const string & name) {
    this_thread::sleep_for(chrono::seconds(1));
    safe_print(name + " (quick)");
}

void slow(const string& name) {
    this_thread::sleep_for(chrono::seconds(7));
    safe_print(name + " (slow)");
}

void work() {
    {
        lock_guard<mutex> lock(cout_mutex);
        cout << "Work started..." << endl;
    }

    auto start = chrono::high_resolution_clock::now();

    promise<void> a3_promise;
    shared_future<void> a3_future = a3_promise.get_future();

    auto thread1 = async(launch::async, [&a3_future]() {
        quick("A1");
        slow("A2");
        quick("B1");

        a3_future.wait();
        quick("C1");
    });

    auto thread2 = async(launch::async, [&a3_promise]() {
        quick("A3");

        a3_promise.set_value();

        slow("B2");
        quick("C2");
    });

    thread1.wait();
    thread2.wait();

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = end - start;

    {
        lock_guard<mutex> lock(cout_mutex);
        cout << "Time: " << elapsed.count() << " s\n";
    }
}

int main() {
    work();
    {
        lock_guard<mutex> lock(cout_mutex);
        cout << "Work is done!" << endl;
    }
    return 0;
}