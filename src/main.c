#define _POSIX_C_SOURCE 200809L

#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct {
  const double *a;
  const double *b;
  double *c;
  int n;
  int row_start;
  int row_end;
} worker_arg_t;

static double elapsed_seconds(const struct timespec *start, const struct timespec *end) {
  double sec = (double)(end->tv_sec - start->tv_sec);
  double nsec = (double)(end->tv_nsec - start->tv_nsec) / 1e9;
  return sec + nsec;
}

static void fill_matrices(double *a, double *b, int n) {
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      a[(size_t)i * (size_t)n + (size_t)j] = (double)((i + j) % 100) / 100.0;
      b[(size_t)i * (size_t)n + (size_t)j] = (double)((i * 3 + j * 7) % 100) / 200.0;
    }
  }
}

static void multiply_rows(const double *a, const double *b, double *c, int n, int row_start, int row_end) {
  for (int i = row_start; i < row_end; i++) {
    for (int j = 0; j < n; j++) {
      double sum = 0.0;
      for (int k = 0; k < n; k++) {
        sum += a[(size_t)i * (size_t)n + (size_t)k] * b[(size_t)k * (size_t)n + (size_t)j];
      }
      c[(size_t)i * (size_t)n + (size_t)j] = sum;
    }
  }
}

static void multiply_sequential(const double *a, const double *b, double *c, int n) {
  multiply_rows(a, b, c, n, 0, n);
}

static void *worker_multiply(void *arg_ptr) {
  worker_arg_t *arg = (worker_arg_t *)arg_ptr;
  multiply_rows(arg->a, arg->b, arg->c, arg->n, arg->row_start, arg->row_end);
  return NULL;
}

static int multiply_parallel(const double *a, const double *b, double *c, int n, int thread_count) {
  pthread_t *threads = (pthread_t *)malloc((size_t)thread_count * sizeof(pthread_t));
  worker_arg_t *args = (worker_arg_t *)malloc((size_t)thread_count * sizeof(worker_arg_t));

  if (threads == NULL || args == NULL) {
    free(threads);
    free(args);
    return -1;
  }

  int base_rows = n / thread_count;
  int remainder = n % thread_count;
  int next_start = 0;

  for (int t = 0; t < thread_count; t++) {
    int rows_for_thread = base_rows + (t < remainder ? 1 : 0);

    args[t].a = a;
    args[t].b = b;
    args[t].c = c;
    args[t].n = n;
    args[t].row_start = next_start;
    args[t].row_end = next_start + rows_for_thread;

    if (pthread_create(&threads[t], NULL, worker_multiply, &args[t]) != 0) {
      for (int j = 0; j < t; j++) {
        pthread_join(threads[j], NULL);
      }
      free(threads);
      free(args);
      return -1;
    }

    next_start = args[t].row_end;
  }

  for (int t = 0; t < thread_count; t++) {
    pthread_join(threads[t], NULL);
  }

  free(threads);
  free(args);
  return 0;
}

static int matrices_are_close(const double *left, const double *right, size_t len, double tolerance, double *max_diff) {
  double local_max = 0.0;

  for (size_t i = 0; i < len; i++) {
    double diff = fabs(left[i] - right[i]);
    if (diff > local_max) {
      local_max = diff;
    }
    if (diff > tolerance) {
      if (max_diff != NULL) {
        *max_diff = local_max;
      }
      return 0;
    }
  }

  if (max_diff != NULL) {
    *max_diff = local_max;
  }
  return 1;
}

static double matrix_checksum(const double *matrix, size_t len) {
  double sum = 0.0;
  for (size_t i = 0; i < len; i++) {
    sum += matrix[i];
  }
  return sum;
}

int main(int argc, char **argv) {
  const int default_n = 1000;
  const int default_runs = 6;
  const double tolerance = 1e-6;

  if (argc < 2) {
    fprintf(stderr, "Uso: %s <threads> [n] [runs]\n", argv[0]);
    fprintf(stderr, "Exemplo: %s 4 1000 6\n", argv[0]);
    return 1;
  }

  int thread_count = atoi(argv[1]);
  int n = (argc >= 3) ? atoi(argv[2]) : default_n;
  int runs = (argc >= 4) ? atoi(argv[3]) : default_runs;

  if (thread_count <= 0 || n <= 0 || runs < 2) {
    fprintf(stderr, "Parâmetros inválidos: threads > 0, n > 0 e runs >= 2.\n");
    return 1;
  }

  if (thread_count > n) {
    thread_count = n;
  }

  size_t matrix_len = (size_t)n * (size_t)n;
  size_t matrix_bytes = matrix_len * sizeof(double);

  double *a = (double *)malloc(matrix_bytes);
  double *b = (double *)malloc(matrix_bytes);
  double *c_seq = (double *)malloc(matrix_bytes);
  double *c_par = (double *)malloc(matrix_bytes);

  if (a == NULL || b == NULL || c_seq == NULL || c_par == NULL) {
    fprintf(stderr, "Falha ao alocar memória para as matrizes.\n");
    free(a);
    free(b);
    free(c_seq);
    free(c_par);
    return 1;
  }

  fill_matrices(a, b, n);

  double total_seq = 0.0;
  double total_par = 0.0;

  for (int run = 0; run < runs; run++) {
    struct timespec t0, t1;

    clock_gettime(CLOCK_MONOTONIC, &t0);
    multiply_sequential(a, b, c_seq, n);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    double elapsed_seq = elapsed_seconds(&t0, &t1);

    clock_gettime(CLOCK_MONOTONIC, &t0);
    if (multiply_parallel(a, b, c_par, n, thread_count) != 0) {
      fprintf(stderr, "Erro ao executar versão paralela.\n");
      free(a);
      free(b);
      free(c_seq);
      free(c_par);
      return 1;
    }
    clock_gettime(CLOCK_MONOTONIC, &t1);
    double elapsed_par = elapsed_seconds(&t0, &t1);

    if (run > 0) {
      total_seq += elapsed_seq;
      total_par += elapsed_par;
    }
  }

  double average_seq = total_seq / (double)(runs - 1);
  double average_par = total_par / (double)(runs - 1);
  double speedup = average_seq / average_par;

  double max_diff = 0.0;
  int ok = matrices_are_close(c_seq, c_par, matrix_len, tolerance, &max_diff);

  double checksum_seq = matrix_checksum(c_seq, matrix_len);
  double checksum_par = matrix_checksum(c_par, matrix_len);

  printf("Problema: P1 - Multiplicacao de Matrizes\n");
  printf("Dimensao: %d x %d\n", n, n);
  printf("Threads (paralelo): %d\n", thread_count);
  printf("Execucoes totais: %d (descarta a primeira)\n", runs);
  printf("T_seq medio: %.6f s\n", average_seq);
  printf("T_par medio: %.6f s\n", average_par);
  printf("Speedup (T_seq/T_par): %.3fx\n", speedup);
  printf("Checksum seq: %.8f\n", checksum_seq);
  printf("Checksum par: %.8f\n", checksum_par);
  printf("Validacao seq vs par: %s (max_diff = %.10f, tol = %.1e)\n", ok ? "OK" : "FALHA", max_diff, tolerance);

  free(a);
  free(b);
  free(c_seq);
  free(c_par);
  return ok ? 0 : 2;
}
