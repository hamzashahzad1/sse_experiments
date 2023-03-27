#ifndef TYPES
#define TYPES

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

#define KEYWORD_SIZE 32


std::map<int, std::chrono::time_point<std::chrono::high_resolution_clock>> m_begs;
std::map<int, double> timehist;

struct myTime{
    double cache_time;
    double seek_time;
    double read_time;
    double total_access_time;
};

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

myTime best_locality(uint64_t keyword_list_size){
    FILE* file;
    file = fopen("myfile.dat", "rb");
    uint64_t total_data_size = keyword_list_size*KEYWORD_SIZE;

   
    char chainHead[128];

    myTime time_variable;
    
    srand(time(NULL));
    auto pos = rand() % int(pow(2,33));

    startTimer(1);
    system("sudo hdparm -A 0 /dev/sda");
    system("echo 3 | sudo tee /proc/sys/vm/drop_caches");
    time_variable.cache_time = stopTimer(1);

    startTimer(2);
    fseek(file, pos, SEEK_SET);
    time_variable.seek_time = stopTimer(2);

    startTimer(3);
    for(uint64_t i = 0; i < total_data_size; i+= KEYWORD_SIZE){
        char chainHead[32];
        fread(chainHead, KEYWORD_SIZE, 1, file);
    }
    time_variable.read_time = stopTimer(3);

    time_variable.total_access_time = time_variable.seek_time + time_variable.read_time;

    fclose(file);

    return time_variable;
}

myTime worst_locality(uint64_t keyword_list_size){
    FILE* file;
    file = fopen("myfile.dat", "rb");
    char chainHead[128];

    srand(time(NULL));

    myTime time_variable;
    auto totalCacheTime = 0;
    auto totalSeekTime = 0;
    auto totalReadTime = 0;

    for(uint64_t i = 1; i <= keyword_list_size; i++){
        startTimer(i);
        system("sudo hdparm -A 0 /dev/sda");
        system("echo 3 | sudo tee /proc/sys/vm/drop_caches");
        auto cache_time = stopTimer(i);
        totalCacheTime += cache_time;
        
        auto pos = rand() % int(pow(2,33));

        startTimer(2*i);
        fseek(file, pos , SEEK_SET);
        auto seekTime = stopTimer(2*i);
        totalSeekTime += seekTime;

        startTimer(3*i);
        fread(chainHead, KEYWORD_SIZE, 1, file);
        auto readTime = stopTimer(3*i);
        totalReadTime += readTime;
       
    }

    time_variable.cache_time = totalCacheTime;
    time_variable.seek_time = totalSeekTime;
    time_variable.read_time = totalReadTime;
    time_variable.total_access_time = totalReadTime + totalSeekTime;
    
    fclose(file);

    return time_variable;
}

myTime logN_locality(uint64_t keyword_list_size, uint logN_value){
    FILE* file;
    file = fopen("myfile.dat", "rb");
    char chainHead[128];

    uint64_t per_block = keyword_list_size/27;
    uint64_t remaining_words = keyword_list_size;

    srand(time(NULL));

    myTime time_variable;
    auto totalCacheTime = 0;
    auto totalSeekTime = 0;
    auto totalReadTime = 0;

    for(uint i = 1; i <= logN_value; i++){
        startTimer(i);
        system("sudo hdparm -A 0 /dev/sda");
        system("echo 3 | sudo tee /proc/sys/vm/drop_caches");
        auto cache_time = stopTimer(i);
        totalCacheTime += cache_time;
        
        auto pos = rand() % int(pow(2,33));

        startTimer(2*i);
        fseek(file, pos , SEEK_SET);
        auto seekTime = stopTimer(2*i);
        totalSeekTime += seekTime;

        startTimer(3*i);
        if(i == logN_value) per_block = remaining_words;
        for(uint64_t j = 0; j < per_block; j++){
            fread(chainHead, KEYWORD_SIZE, 1, file);
        }
        auto readTime = stopTimer(3*i);
        totalReadTime += readTime;
        remaining_words -= per_block;
    }

    time_variable.cache_time = totalCacheTime;
    time_variable.read_time = totalReadTime;
    time_variable.seek_time = totalSeekTime;
    time_variable.total_access_time = totalReadTime + totalSeekTime;
    
    fclose(file);

    return time_variable;
}

int main(int argc, char** argv){
    myTime time_variable ;
    // time_variable = best_locality(134217728);
    // time_variable = logN_locality(134217728,27);
    time_variable = worst_locality(1048576);
    std::cout << "Total access time: " << time_variable.total_access_time << std::endl;
}

#endif