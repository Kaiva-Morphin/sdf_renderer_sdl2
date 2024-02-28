#include <iostream>
#include <thread>
#include <future>
#include <cmath>
#include <vector>
using namespace std;
 


#include <chrono>
#include <ctime>


// A dummy function
void calc(int a, uint32_t* p)
{
    for (int i = 0; i < 100; i++){
        p[i] = a + 100 * i;
    }
}
 
// Driver code
int main()
{
    auto start = std::chrono::system_clock::now();
    vector<thread> threads;
    uint32_t results[100][100];
    for (int x = 0; x < 100; x++) {
        threads.push_back(thread(calc, x, results[x]));
    }
   
    for (int i = 0; i < 100; i++){
        threads[i].join();
        cout << results[i][99] << endl;
    }
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-start;
    cout <<  elapsed_seconds.count() << "\n";


    /*start = std::chrono::system_clock::now();
    for (int i = 0; i < 10000; i++){
        calc(3);
    }
    end = std::chrono::system_clock::now();
    elapsed_seconds = end-start;
    cout <<  elapsed_seconds.count() << "\n";*/
    return 0;
}