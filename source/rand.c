
#define RAND_MAX      32767
static unsigned long next = 1;

void srand(int seed) {
    next = seed;
}

int rand(void) {
    next = next * 1103515245 + 12345;
    return (int)((next/65536) % ((unsigned long)RAND_MAX + 1));
}
