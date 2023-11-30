


#ifndef MISC_H
#define MISC_H

#include <stdint.h>

#define _reg_set(BaseAddress, RegOffset, Data) \
    *(volatile uint32_t*)((BaseAddress) + (RegOffset >> 2)) = (uint32_t)(Data)
#define _reg_get(BaseAddress, RegOffset) \
    *(volatile uint32_t*)((BaseAddress) + (RegOffset >> 2))

#endif // MISC_H
