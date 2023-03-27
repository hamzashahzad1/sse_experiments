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

#include <openssl/aes.h>
#include <openssl/sha.h>
#include <openssl/evp.h>

#define AES_KEY_SIZE 32

// Each array element is a 128 bit int vector
static __m128i key_schedule[20];
std::map<int, std::chrono::time_point<std::chrono::high_resolution_clock>> m_begs;
std::map<int, double> timehist;

#define DO_ENC_BLOCK(m,k) \
        do{\
        m = _mm_xor_si128       (m, k[ 0]); \
        m = _mm_aesenc_si128    (m, k[ 1]); \
        m = _mm_aesenc_si128    (m, k[ 2]); \
        m = _mm_aesenc_si128    (m, k[ 3]); \
        m = _mm_aesenc_si128    (m, k[ 4]); \
        m = _mm_aesenc_si128    (m, k[ 5]); \
        m = _mm_aesenc_si128    (m, k[ 6]); \
        m = _mm_aesenc_si128    (m, k[ 7]); \
        m = _mm_aesenc_si128    (m, k[ 8]); \
        m = _mm_aesenc_si128    (m, k[ 9]); \
        m = _mm_aesenclast_si128(m, k[10]);\
    }while(0)


#define DO_DEC_BLOCK(m,k) \
        do{\
        m = _mm_xor_si128       (m, k[10+0]); \
        m = _mm_aesdec_si128    (m, k[10+1]); \
        m = _mm_aesdec_si128    (m, k[10+2]); \
        m = _mm_aesdec_si128    (m, k[10+3]); \
        m = _mm_aesdec_si128    (m, k[10+4]); \
        m = _mm_aesdec_si128    (m, k[10+5]); \
        m = _mm_aesdec_si128    (m, k[10+6]); \
        m = _mm_aesdec_si128    (m, k[10+7]); \
        m = _mm_aesdec_si128    (m, k[10+8]); \
        m = _mm_aesdec_si128    (m, k[10+9]); \
        m = _mm_aesdeclast_si128(m, k[0]);\
    }while(0)

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

// for AES expansion
static __m128i aes_128_key_expansion(__m128i key, __m128i keygened) {
    keygened = _mm_shuffle_epi32(keygened, _MM_SHUFFLE(3, 3, 3, 3));
    key = _mm_xor_si128(key, _mm_slli_si128(key, 4));
    key = _mm_xor_si128(key, _mm_slli_si128(key, 4));
    key = _mm_xor_si128(key, _mm_slli_si128(key, 4));
    return _mm_xor_si128(key, keygened);
}

void aes128_enc(int8_t *plainText, int8_t *cipherText) {
    __m128i m = _mm_loadu_si128((__m128i *) plainText);

    DO_ENC_BLOCK(m, key_schedule);

    _mm_storeu_si128((__m128i *) cipherText, m);
}

void aes128_dec(int8_t *cipherText, int8_t *plainText) {
    __m128i m = _mm_loadu_si128((__m128i *) cipherText);

    DO_DEC_BLOCK(m, key_schedule);

    _mm_storeu_si128((__m128i *) plainText, m);
}

void aes128_load_key(int8_t *enc_key){
    key_schedule[0] =  _mm_loadu_si128((const __m128i*) enc_key);
    
    
    key_schedule[1] = aes_128_key_expansion(key_schedule[0], _mm_aeskeygenassist_si128(key_schedule[0], 0x00));
    key_schedule[2] = aes_128_key_expansion(key_schedule[1], _mm_aeskeygenassist_si128(key_schedule[1], 0x00));
    key_schedule[3] = aes_128_key_expansion(key_schedule[2], _mm_aeskeygenassist_si128(key_schedule[2], 0x00));
    key_schedule[4] = aes_128_key_expansion(key_schedule[3], _mm_aeskeygenassist_si128(key_schedule[3], 0x00));
    key_schedule[5] = aes_128_key_expansion(key_schedule[4], _mm_aeskeygenassist_si128(key_schedule[4], 0x00));
    key_schedule[6] = aes_128_key_expansion(key_schedule[5], _mm_aeskeygenassist_si128(key_schedule[5], 0x00));
    key_schedule[7] = aes_128_key_expansion(key_schedule[6], _mm_aeskeygenassist_si128(key_schedule[6], 0x00));
    key_schedule[8] = aes_128_key_expansion(key_schedule[7], _mm_aeskeygenassist_si128(key_schedule[7], 0x00));
    key_schedule[9] = aes_128_key_expansion(key_schedule[8], _mm_aeskeygenassist_si128(key_schedule[8], 0x00));
    key_schedule[10] = aes_128_key_expansion(key_schedule[9], _mm_aeskeygenassist_si128(key_schedule[9], 0x00));
    
    uint temp = 9;
    for (uint i = 11; i<= 19; i++){
        key_schedule[i] = _mm_aesimc_si128(key_schedule[temp]);
        temp--;
    }

}




// TO RUN THE PROGRAM: g++ encryption.cpp -std=c++17 -march=native -o encryption
int main(int argc, char** argv)
{
    std::string key_id_128 = "";
    for(uint8_t i = 0; i < AES_KEY_SIZE; i++){
        key_id_128.push_back(char(i));
    }

    std::array<uint8_t, AES_KEY_SIZE> plaintext;
    unsigned char curkey[AES_KEY_SIZE];
    std::array<uint8_t, AES_KEY_SIZE> ciphertext;

    // uint keyword_list_size = uint(pow(2,23));
    uint keyword_list_size = 134217728;
    
    startTimer(1);
    for(uint i = 0; i < keyword_list_size; i++){
        aes128_load_key((int8_t*) curkey);
        aes128_enc((int8_t*) key_id_128.data(), (int8_t*) ciphertext.data());
    }
    auto enc_time = stopTimer(1);

    std::cout <<"Encryption time: " << enc_time <<std::endl;

    startTimer(2);
    for(uint i = 0; i < keyword_list_size; i++){
        aes128_load_key((int8_t*) curkey);
        aes128_dec((int8_t*) ciphertext.data(), (int8_t*) plaintext.data());  
    }
    auto dec_time = stopTimer(2);  
    std::cout <<"Decryption Time: " << dec_time <<std::endl;

    // std::cout << key_id_128.size() << std::endl;
    // std::cout << plaintext.size() << std::endl;
    // std:: cout << ciphertext.size() << std::endl;
}

#endif