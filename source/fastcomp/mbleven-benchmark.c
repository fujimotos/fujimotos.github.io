/*
 * mbleven-benchmark - A benchmark program for mbleven
 *
 * How to run a perf test:
 *
 *   $ cc -o mbleven-benchmark mbleven-benchmark.c
 *   $ ./mbleven-benchmark
 *
 * This program is placed in the public domain.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#define MIN(a,b) ((a) < (b) ? (a) : (b))

/*
 * The model table for mbleven algorithm.
 */
static const char *matrix[] = {
      "r",  NULL,  NULL,  NULL,  NULL,  NULL,  NULL, // 1
      "d",  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,
     "rr",  "id",  "di",  NULL,  NULL,  NULL,  NULL, // 2
     "rd",  "dr",  NULL,  NULL,  NULL,  NULL,  NULL,
     "dd",  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,
    "rrr", "idr", "ird", "rid", "rdi", "dri", "dir", // 3
    "rrd", "rdr", "drr", "idd", "did", "ddi",  NULL,
    "rdd", "drd", "ddr",  NULL,  NULL,  NULL,  NULL,
    "ddd",  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,
};

static const int matrix_row_index[3] = { 0, 2, 5 };

#define MATRIX_COLSIZE 7

/*
 * mbleven algorithm
 */
int check_model(char *s1, char *s2, int len1, int len2, const char *model)
{
    int i = 0, j = 0, c = 0;

    while (i < len1 && j < len2) {
        if (s1[i] != s2[j]) {
            switch (model[c]) {
                case 'd':
                    i++;
                    break;
                case 'i':
                    j++;
                    break;
                case 'r':
                    i++;
                    j++;
                    break;
                case '\0':
                    return c + 1;
            }
            c++;
        } else {
            i++;
            j++;
        }
    }
    return c + (len1 - i) + (len2 - j);
}

int mbleven(char *s1, char *s2, int k)
{
    const char *model;
    char *sx;
    int row, col;
    int res = k + 1;
    int dst;
    int len1, len2, lenx;

    len1 = strlen(s1);
    len2 = strlen(s2);
    if (len1 < len2) {
        sx = s1;
        s1 = s2;
        s2 = sx;
        lenx = len1;
        len1 = len2;
        len2 = lenx;
    }
    if (len1 - len2 > k)
        return k + 1;

    row = matrix_row_index[k - 1] + (len1 - len2);
    for (col = 0; col < MATRIX_COLSIZE; col++) {
        model = matrix[row * MATRIX_COLSIZE + col];
        if (model == NULL)
            break;
        dst = check_model(s1, s2, len1, len2, model);
        res = MIN(res, dst);
    }
    return res;
}

/*
 * Slightly optimized Wagner-Fischer algorithm
 */
int wagner_fischer(char *s1, char *s2, int k)
{
    int i, j, dia, tmp, *arr;
    int len1, len2;
    char chr;

    len1 = strlen(s1);
    len2 = strlen(s2);

    if (abs(len1 - len2) > k)
        return k + 1;

    arr = malloc((len2 + 1) * sizeof(int));
    if (arr == NULL)
        return -1;

    for (j = 0; j <= len2; j++)
        arr[j] = j;

    for (i = 1; i <= len1; i++) {
        chr = s1[i - 1];
        dia = i - 1;
        arr[0] = i;

        for (j = 1; j <= len2; j++) {
            tmp = arr[j];

            if (chr != s2[j - 1]) {
                arr[j] = MIN(arr[j], arr[j - 1]);
                arr[j] = MIN(arr[j], dia) + 1;
            } else {
                arr[j] = dia;
            }
            dia = tmp;
        }
    }
    dia = arr[len2];
    free(arr);
    if (dia > k)
        dia = k + 1;
    return dia;
}

/*
 * Speedtest utils
 */
void int_to_bin(int num, char *buf)
{
    const char bin[] = "01";
    while (num >> 1) {
        *buf++ = bin[num % 2];
        num = num >> 1;
    }
    *buf = '\0';
}

double now(void)
{
  struct timeval t;
  gettimeofday(&t, (struct timezone *) 0);
  return t.tv_sec + t.tv_usec / (double) (1000000);
}

void bench(char *name, int (*func)(char*, char*, int), int k)
{
    char s1[15];
    char s2[15];
    int i, j;
    int loop = 2048;
    double t0, dt;

    t0 = now();
    for (i = 1; i < loop; i++) {
        int_to_bin(i, s1);
        for (j = 1; j < loop; j++) {
            int_to_bin(j, s2);
            func(s1, s2, k);
        }
    }
    dt = now() - t0;
    printf("%-20s %2i %10.3f %15.0f\n", name, k, dt, (loop * loop) / dt);
}

int main()
{
    printf("%-20s %2s %10s %15s\n", "#", "k", "TIME[s]", "SPEED[calls/s]");
    bench("mbleven", mbleven, 1);
    bench("mbleven", mbleven, 2);
    bench("mbleven", mbleven, 3);
    bench("wagner_fischer", wagner_fischer, 1);
    bench("wagner_fischer", wagner_fischer, 2);
    bench("wagner_fischer", wagner_fischer, 3);
}
