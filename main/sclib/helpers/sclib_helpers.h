#ifndef SCLIB_HELPERS_H
#define SCLIB_HELPERS_H

#include <stdbool.h>

// Mappa un valore float da un intervallo a un altro, opzionalmente saturando il risultato
// Se limit == true, il risultato è limitato a [out_min, out_max]
static inline float mapf(float x, float in_min, float in_max, float out_min, float out_max, bool limit) {
    // Gestione intervallo inverso o uguale
    if (in_max == in_min) return out_min;
    float result = (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    if (limit) {
        if (out_min < out_max) {
            if (result < out_min) result = out_min;
            if (result > out_max) result = out_max;
        } else {
            if (result > out_min) result = out_min;
            if (result < out_max) result = out_max;
        }
    }
    return result;
}

#endif // SCLIB_MATH_H
