#pragma GCC optimize("O3,unroll-loops")
#pragma GCC target("avx2")
#include <bits/stdc++.h>

using namespace std;

class hashingGenerator
{
public:
    virtual string generateHash(string key) = 0;

};

class MyHash : hashingGenerator
{
    uint64_t mix(uint64_t value, int shift)
    {
        value ^= (value >> shift);
        value *= 0x7FFFFFFF;
        value ^= (value << (shift / 2));
        return value;
    }
public :
    string generateHash(string key)
    {
        int length = key.size();
        const uint8_t* data = reinterpret_cast<const uint8_t*>(key.data());
        uint64_t h1 = 1;
        uint64_t h2 = 2;
        uint64_t h3 = 3;
        uint64_t h4 = 4;
        for (int i = 0; i < length; i ++)
        {
            uint64_t k1 = data[i];
            uint64_t k2 = 2*data[i];
            uint64_t k3 = 3*data[i];
            uint64_t k4 = 4*data[i];
            k1 = mix(k1, 13);
            h1 ^= k1;
            h1 = mix(h1 + h2, 17);
            k2 = mix(k2, 15);
            h2 ^= k2;
            h2 = mix(h2 + h3, 19);
            k3 = mix(k3, 17);
            h3 ^= k3;
            h3 = mix(h3 + h4, 23);
            k4 = mix(k4, 19);
            h4 ^= k4;
            h4 = mix(h4 + h1, 29);
        }

        h1 ^= mix(h4, 11);
        h2 ^= mix(h1, 13);
        h3 ^= mix(h2, 15);
        h4 ^= mix(h3, 17);
        h1 ^= mix(h4, 11);

        char result[65];
        snprintf(result, 65, "%016llx%016llx%016llx%016llx", h1, h2, h3, h4);
        return string(result);
    }
};
