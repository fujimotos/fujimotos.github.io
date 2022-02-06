/*
 * This is a perf program to compare the Wagner-Fischer algorithm
 * and Myers' bit-parallel algorithm.
 *
 * You can run this program as follows:
 *
 *   $ cc -O2 -o benchmark benchmark.c
 *   $ ./benchmark
 *
 * License: public domain.
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <assert.h>

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))

/*
 * The Wagner-Fischer algorithm with Ukkonen Optimization
 */
int8_t wagner_fischer(char *s1, int8_t len1, char *s2, int8_t len2)
{
    int8_t Mx, mx, dia, top, left;
    int8_t i, j, m, M;
    int8_t buf[4096];
    char *p1;

    if (len1 < len2)
        return wagner_fischer(s2, len2, s1, len1);
    if (len2 == 0)
        return len1;
    if (len2 == 1)
        return len1 - (memchr(s1, s2[0], len1) != NULL);
    if (len2 == 2) {
        p1 = memchr(s1, s2[0], len1 - 1);
        if (p1) {
            i = len1 + s1 - p1 - 1;
            return len1 - (memchr(p1+1, s2[1], i) != NULL) - 1;
        } else {
            return len1 - (memchr(s1+1, s2[1], len1 - 1) != NULL);
        }
    }

    Mx = (len2 - 1) / 2;
    mx = 1 - Mx - (len1 - len2);

    for (j = 0; j <= Mx; j++)
        buf[j] = j;

    for (i = 1; i <= len1; i++) {
        buf[0] = i - 1;

        m = MAX(mx, 1);
        M = MIN(Mx, len2);
        mx++;
        Mx++;

        dia = buf[m - 1];
        top = buf[m];

        if (s1[i - 1] != s2[m - 1])
            dia = MIN(dia, top) + 1;

        buf[m] = dia;
        left = dia;
        dia = top;

        for (j = m + 1; j <= M; j++) {
            top = buf[j];

            if (s1[i - 1] != s2[j - 1])
                dia = MIN(MIN(dia, top), left) + 1;
            buf[j] = dia;
            left = dia;
            dia = top;
        }

        if (len2 == M)
            continue;

        if (s1[i - 1] != s2[M])
            dia = MIN(dia, left) + 1;
        buf[M + 1] = dia;
    }
    return buf[len2];
}

/*
 * Myers' bit-parallel algorithm
 *
 * G. Myers. "A fast bit-vector algorithm for approximate string
 * matching based on dynamic programming." Journal of the ACM, 1999.
 */

int8_t myers1999(char *s1, int8_t len1, char *s2, int8_t len2)
{
    uint64_t Peq[256];
    uint64_t Eq, Xv, Xh, Ph, Mh, Pv, Mv, Last;
    int8_t i;
    int8_t Score = len2;

    memset(Peq, 0, sizeof(Peq));

    for (i = 0; i < len2; i++)
        Peq[s2[i]] |= (uint64_t) 1 << i;

    Mv = 0;
    Pv = (uint64_t) -1;
    Last = (uint64_t) 1 << (len2 - 1);

    for (i = 0; i < len1; i++) {
        Eq = Peq[s1[i]];

        Xv = Eq | Mv;
        Xh = (((Eq & Pv) + Pv) ^ Pv) | Eq;

        Ph = Mv | ~ (Xh | Pv);
        Mh = Pv & Xh;

        if (Ph & Last) Score++;
        if (Mh & Last) Score--;

        Ph = (Ph << 1) | 1;
        Mh = (Mh << 1);

        Pv = Mh | ~ (Xv | Ph);
        Mv = Ph & Xv;
    }
    return Score;
}

/*
 * Test Utils
 */
void randomhex(char *buf, int len)
{
    int i;
    const static char hex[] = "0123456789abcdef";
    srand(time(NULL));
    for (i = 0; i < len; i++)
        buf[i] = hex[rand() & 0x0f];
}

double now(void)
{
    struct timeval t;
    gettimeofday(&t, (struct timezone *) 0);
    return t.tv_sec + t.tv_usec / (double) (1000000);
}

/*
 * Benchmark Testing
 */
char *dataset;
#define DATASIZE 131072  // 2048 strings (of length 64)
#define STR_A(n) (dataset + ((n) * 64))
#define STR_B(n) (dataset + (((n) + 1024) * 64))

int bench_init(void)
{
    dataset = malloc(DATASIZE);
    if (dataset == NULL)
        return -1;

    randomhex(dataset, DATASIZE);
    return 0;
}

void bench_finish(void)
{
    free(dataset);
}

double bench_run(int len, int8_t (func)(char*, int8_t, char*, int8_t))
{
    int i, j;
    double t0;

    t0 = now();

    for (i = 0; i < 1024; i++)
        for (j = 0; j < 1024; j++)
            func(STR_A(i), len, STR_B(j),  len);

    return now() - t0;
}

int main(void)
{
    int len;
    double time_wf, time_my;

    if (bench_init())
        return -1;

    printf("%2s %-15s %-15s\n", "#", "WAGNER_FISCHER", "MYERS1999");

    for (len = 2; len < 65; len += 2) {
        time_wf = bench_run(len, wagner_fischer);
        time_my = bench_run(len, myers1999);
        printf("%2i %10.3f %10.3f\n", len, time_wf, time_my);
    }

    bench_finish();
    return 0;
}
