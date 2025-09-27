#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define N 288
#define VALID_MIN 0
#define VALID_MAX 1000
#define ANOMALY_RATIO 0.05      // ~5%
#define HIST_BINS 10
#define WINDOW_RADIUS 2         //окно медианы: i-2..i+2 (всего 5 значений макс)

//Утилиты

int is_correct(int v) {
    return (v >= VALID_MIN && v <= VALID_MAX);
}

int clamp(int v, int lo, int hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

//простая вставочная сортировка для маленьких буферов и N
void insertion_sort_int(int a[], int len) {
    for (int i = 1; i < len; ++i) {
        int key = a[i];
        int j = i - 1;
        while (j >= 0 && a[j] > key) {
            a[j + 1] = a[j];
            --j;
        }
        a[j + 1] = key;
    }
}

/* медиана массива int -> double */
double median_of_ints(int a[], int len) {
    int tmp[N]; /* достаточно, т.к. len <= N */
    for (int i = 0; i < len; ++i) tmp[i] = a[i];
    insertion_sort_int(tmp, len);
    if (len % 2 == 1) {
        return (double)tmp[len/2];
    } else {
        int m1 = tmp[len/2 - 1];
        int m2 = tmp[len/2];
        return ((double)m1 + (double)m2) / 2.0;
    }
}

/* ---------- Генерация данных ---------- */

int rand_in_range(int lo, int hi) { /* включительно */
    return lo + (rand() % (hi - lo + 1));
}

int make_valid() {
    return rand_in_range(VALID_MIN, VALID_MAX);
}

/* аномалия либо ниже минимума, либо выше максимума */
int make_anomaly() {
    int side = rand() % 2; /* 0 -> ниже минимума; 1 -> выше максимума */
    int mag = 1 + (rand() % 200); /* величина выхода за пределы */
    if (side == 0) return VALID_MIN - mag;
    else           return VALID_MAX + mag;
}

/* Заполнение массива: для каждой позиции случайно решаем, будет ли аномалия */
void generate_data(int a[], int n, double anomaly_ratio) {
    for (int i = 0; i < n; ++i) {
        /* используем rand() для вероятности аномалии */
        double r = (double)rand() / (double)RAND_MAX;
        if (r < anomaly_ratio) a[i] = make_anomaly();
        else                   a[i] = make_valid();
    }
}

/* ---------- Статистика ---------- */

double mean_all(const int a[], int n) {
    long long sum = 0;
    for (int i = 0; i < n; ++i) sum += a[i];
    return (double)sum / (double)n;
}

/* медиана по всем значениям (включая аномалии) */
double median_all(const int a[], int n) {
    int tmp[N];
    for (int i = 0; i < n; ++i) tmp[i] = a[i];
    insertion_sort_int(tmp, n);
    if (n % 2 == 1) return (double)tmp[n/2];
    else return ((double)tmp[n/2 - 1] + (double)tmp[n/2]) / 2.0;
}

/* мин/макс среди корректных значений */
int min_correct(const int a[], int n, int *found) {
    int ok = 0;
    int mn = 0;
    for (int i = 0; i < n; ++i) {
        if (is_correct(a[i])) {
            if (!ok) { mn = a[i]; ok = 1; }
            else if (a[i] < mn) mn = a[i];
        }
    }
    *found = ok;
    return mn;
}

int max_correct(const int a[], int n, int *found) {
    int ok = 0;
    int mx = 0;
    for (int i = 0; i < n; ++i) {
        if (is_correct(a[i])) {
            if (!ok) { mx = a[i]; ok = 1; }
            else if (a[i] > mx) mx = a[i];
        }
    }
    *found = ok;
    return mx;
}

/* глобальное среднее только корректных значений, округляем до int */
int mean_correct_int(const int a[], int n, int *found) {
    long long sum = 0;
    int cnt = 0;
    for (int i = 0; i < n; ++i) {
        if (is_correct(a[i])) { sum += a[i]; cnt++; }
    }
    *found = (cnt > 0);
    if (cnt == 0) return 0;
    double m = (double)sum / (double)cnt;
    return (int)lrint(m);
}

int count_anomalies(const int a[], int n) {
    int c = 0;
    for (int i = 0; i < n; ++i) if (!is_correct(a[i])) c++;
    return c;
}

/* ---------- Фильтрация (замена аномалий) ---------- */

void filter_anomalies_median_window(const int src[], int dst[], int n) {
    /* скопируем валидные, аномалии заменим */
    int found_mean_ok = 0;
    int global_mean_ok = 0;
    int global_mean = mean_correct_int(src, n, &global_mean_ok);

    for (int i = 0; i < n; ++i) {
        if (is_correct(src[i])) {
            dst[i] = src[i];
        } else {
            /* собираем корректных соседей в окне */
            int buf[2*WINDOW_RADIUS + 1];
            int bsz = 0;
            int left = (i - WINDOW_RADIUS < 0) ? 0 : i - WINDOW_RADIUS;
            int right = (i + WINDOW_RADIUS >= n) ? (n - 1) : i + WINDOW_RADIUS;

            for (int j = left; j <= right; ++j) {
                if (j == i) continue; /* сам аномальный элемент не берём */
                if (is_correct(src[j])) {
                    buf[bsz++] = src[j];
                }
            }

            if (bsz > 0) {
                insertion_sort_int(buf, bsz);
                /* медиана по соседям */
                int med;
                if (bsz % 2 == 1) med = buf[bsz/2];
                else med = (buf[bsz/2 - 1] + buf[bsz/2]) / 2;
                dst[i] = med;
            } else if (global_mean_ok) {
                /* нет ни одного корректного соседа – fallback: глобальное матожидание корректных */
                dst[i] = global_mean;
                found_mean_ok = 1;
            } else {
                /* крайний случай: вообще нет корректных значений в массиве */
                dst[i] = VALID_MIN; /* безопасная заглушка */
            }
        }
    }
    (void)found_mean_ok; /* переменная оставлена на случай расширения отчёта */
}

/* ---------- Визуализация ---------- */

void print_first_50_with_flags(const int a[], int n, const char *title) {
    printf("\n%s (первые 50 значений):\n", title);
    printf("индекс : значение [* аномалия]\n");
    int limit = (n < 50) ? n : 50;
    for (int i = 0; i < limit; ++i) {
        printf("%4d : %6d %s\n", i, a[i], is_correct(a[i]) ? "" : "*");
    }
}

/* простая «полосатая» мини-диаграмма значений для первых 50 элементов */
void ascii_miniplot_first_50(const int a[], int n, const char *title) {
    int limit = (n < 50) ? n : 50;
    /* найдём min/max для шкалы среди отображаемых */
    int mn = a[0], mx = a[0];
    for (int i = 1; i < limit; ++i) {
        if (a[i] < mn) mn = a[i];
        if (a[i] > mx) mx = a[i];
    }
    /* чтобы аномалии тоже отобразились, не обрезаем по корректному диапазону */
    int width = 50; /* ширина полосы */
    int span = mx - mn;
    if (span == 0) span = 1;

    printf("\n%s (мини-график первых 50):\n", title);
    for (int i = 0; i < limit; ++i) {
        int bar = (int)llround((long double)(a[i] - mn) * (width - 1) / (long double)span);
        if (bar < 0) bar = 0;
        if (bar >= width) bar = width - 1;
        printf("%3d |", i);
        for (int k = 0; k < width; ++k) {
            if (k == bar) {
                printf("%c", is_correct(a[i]) ? '|' : '*'); /* аномалии отмечаем '*' на позиции */
            } else {
                printf(".");
            }
        }
        printf("| %d\n", a[i]);
    }
}

/* гистограмма распределения: 10 корзин по корректному диапазону + 2 для аномалий */
void histogram_counts(const int a[], int n, int bins[HIST_BINS + 2]) {
    for (int i = 0; i < HIST_BINS + 2; ++i) bins[i] = 0;
    int range = VALID_MAX - VALID_MIN + 1;
    int step = range / HIST_BINS;
    if (step < 1) step = 1;

    for (int i = 0; i < n; ++i) {
        if (!is_correct(a[i])) {
            if (a[i] < VALID_MIN) bins[HIST_BINS]++;      /* аномалии ниже */
            else bins[HIST_BINS + 1]++;                   /* аномалии выше */
        } else {
            int idx = (a[i] - VALID_MIN) / step;
            if (idx >= HIST_BINS) idx = HIST_BINS - 1;
            bins[idx]++;
        }
    }
}

void print_histogram(const int bins[HIST_BINS + 2], const char *title) {
    /* нормируем столбики к высоте 40 */
    int maxc = 1;
    for (int i = 0; i < HIST_BINS + 2; ++i) if (bins[i] > maxc) maxc = bins[i];
    int height = 12;
    printf("\n%s (гистограмма, нормирована):\n", title);

    for (int row = height; row >= 1; --row) {
        for (int i = 0; i < HIST_BINS + 2; ++i) {
            int bar = (bins[i] * height + maxc - 1) / maxc;
            if (bar >= row) printf("█ ");
            else            printf("  ");
        }
        printf("\n");
    }
    for (int i = 0; i < HIST_BINS; ++i) printf("— ");
    printf("L H\n"); /* L=ниже минимума, H=выше максимума */
}

/* ---------- Главная программа ---------- */

int main(void) {
    /* сидируем rand() временем, чтобы при каждом запуске получались разные позиции/кол-во аномалий */
    srand((unsigned)time(NULL));

    int data[N];
    int filtered[N];

    generate_data(data, N, ANOMALY_RATIO);

    /* статистика до фильтрации */
    int anomalies_before = count_anomalies(data, N);

    double mean_before = mean_all(data, N);
    double median_before = median_all(data, N);
    int found_min_ok = 0, found_max_ok = 0;
    int min_ok_before = min_correct(data, N, &found_min_ok);
    int max_ok_before = max_correct(data, N, &found_max_ok);

    /* фильтрация */
    filter_anomalies_median_window(data, filtered, N);

    /* статистика после фильтрации */
    int anomalies_after = count_anomalies(filtered, N);

    double mean_after = mean_all(filtered, N);
    double median_after = median_all(filtered, N);
    int f2_min_ok = 0, f2_max_ok = 0;
    int min_ok_after = min_correct(filtered, N, &f2_min_ok);
    int max_ok_after = max_correct(filtered, N, &f2_max_ok);

    /* ------- Вывод ------- */
    printf("Параметры генерации:\n");
    printf("  N=%d, корректный диапазон: [%d..%d], доля аномалий ~%.1f%%\n",
           N, VALID_MIN, VALID_MAX, ANOMALY_RATIO*100.0);
    printf("  Метод замены аномалий: медиана локального окна (радиус %d),\n", WINDOW_RADIUS);
    printf("  fallback: глобальное матожидание корректных значений.\n");

    /* первые 50 значений и мини-графики */
    print_first_50_with_flags(data, N, "Исходные данные");
    ascii_miniplot_first_50(data, N, "Исходные данные");

    print_first_50_with_flags(filtered, N, "После фильтрации");
    ascii_miniplot_first_50(filtered, N, "После фильтрации");

    /* гистограммы распределений */
    int bins_before[HIST_BINS + 2], bins_after[HIST_BINS + 2];
    histogram_counts(data, N, bins_before);
    histogram_counts(filtered, N, bins_after);

    print_histogram(bins_before, "Распределение до фильтрации");
    print_histogram(bins_after, "Распределение после фильтрации");

    /* сводка по статистике */
    printf("\nСтатистические показатели (до фильтрации):\n");
    printf("  Среднее арифметическое: %.3f\n", mean_before);
    printf("  Медиана:                %.3f\n", median_before);
    if (found_min_ok) printf("  Минимальное корректное: %d\n", min_ok_before);
    else              printf("  Минимальное корректное: НЕТ корректных значений\n");
    if (found_max_ok) printf("  Максимальное корректное: %d\n", max_ok_before);
    else              printf("  Максимальное корректное: НЕТ корректных значений\n");

    printf("\nСтатистические показатели (после фильтрации):\n");
    printf("  Среднее арифметическое: %.3f\n", mean_after);
    printf("  Медиана:                %.3f\n", median_after);
    if (f2_min_ok) printf("  Минимальное корректное: %d\n", min_ok_after);
    else           printf("  Минимальное корректное: НЕТ корректных значений\n");
    if (f2_max_ok) printf("  Максимальное корректное: %d\n", max_ok_after);
    else           printf("  Максимальное корректное: НЕТ корректных значений\n");

    printf("\nАномалии:\n");
    printf("  Обнаружено (до): %d из %d (%.2f%%)\n", anomalies_before, N, 100.0*anomalies_before/N);
    printf("  Осталось (после): %d\n", anomalies_after);
    printf("  Исправлено: %d\n", anomalies_before - anomalies_after);

    return 0;
}
