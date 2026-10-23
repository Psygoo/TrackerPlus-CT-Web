#include "order.h"

uint32_t h11_chance_rank(uint32_t value)
{
    if (value<=100u) return value;
    if (value<=102u) return value+39u;
    return value-2u;
}

uint32_t h11_chance_value(uint32_t rank)
{
    if (rank<=100u) return rank;
    if (rank<=139u) return rank+2u;
    return rank-39u;
}

uint32_t h11_chance_edit(uint32_t value, int32_t delta)
{
    if (value>141u) {
        /* Preserve the old 32-bit add and numeric clamp for unassigned IDs. */
        int32_t sum=(int32_t)(value+(uint32_t)delta);
        return sum<0 ? 0u : sum>141 ? 141u : (uint32_t)sum;
    }
    int32_t rank=(int32_t)h11_chance_rank(value);
    if (delta < -rank) rank=0;
    else if (delta > 141-rank) rank=141;
    else rank+=delta;
    return h11_chance_value((uint32_t)rank);
}
