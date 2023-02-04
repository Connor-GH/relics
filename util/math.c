#include "include/math.h"
#include "include/inttypes.h"

static u32 calculate_power_using_lsb(u32 num, u32 power) {

    u32 answer = 1;

    while (power > 0) {

        if (power & 1)
            answer *= num;

        num *= num;
        power >>= 1;
    }
    return answer;
}

u32 ppowi(u32 num, u32 power) {

    const u32 powers_of_two[32] = {
        2,
        4,
        8,
        16,
        32,
        64,
        128,
        256,
        512,
        1024,
        2048,
        4096,
        8192,
        16384,
        32768,
        65536,
        131072,
        262144,
        524288,
        1048576,
        2097152,
        4194304,
        8388608,
        16777216,
        33554432,
        67108864,
        134217728,
        268435456,
        536870912,
        1073741824,
        2147483648U,
        (4294967296U - 1U) /* prints ~0 */
    };
    if (power == 0)
        return 1;
    if (power == 1)
        return num;

    if (num == 2)
        return powers_of_two[power-1];

    return calculate_power_using_lsb(num, power);
}

