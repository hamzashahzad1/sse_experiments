#include <iostream>
#include <string.h>
#include <inttypes.h>
#include <array>
#include <wmmintrin.h> 
#include <emmintrin.h>
#include <math.h>
#include <stdlib.h> 
#include <map>
#include <chrono>
#include <ctime>
#include <unistd.h>

std::map<int, std::chrono::time_point<std::chrono::high_resolution_clock>> m_begs;
std::map<int, double> timehist;

void startTimer(int id) {
    std::chrono::time_point<std::chrono::high_resolution_clock> m_beg = std::chrono::high_resolution_clock::now();
    m_begs[id] = m_beg;
}

double stopTimer(int id) {
    double t = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - m_begs[id]).count();
    timehist.erase(id);
    timehist[id] = t;
    return t;
}

int main()
{
    startTimer(1);
    usleep(200000);
    std::cout << stopTimer(1) << std::endl;
}