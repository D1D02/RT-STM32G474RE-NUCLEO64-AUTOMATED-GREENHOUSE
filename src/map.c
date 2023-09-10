#include "../lib/map.h"

int map(float x, float in_min, float in_max, int out_min, int out_max) {

    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
