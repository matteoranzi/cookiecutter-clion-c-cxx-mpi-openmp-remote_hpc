//
// Created by Matteo Ranzi on 02/11/25.
//

#include "benchmark/benchmark.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <unistd.h>


#include "debug/print_debug.h"
#include "debug/unique_print_debug.h"

#define MAX_LOG_FILENAME_LENGTH 1024
#define BENCHMARK_FILE_NAME "/benchmark.log"

static void handle_mpi_error_and_exit(const int my_rank, const int status, const char *message) {
    char error_string[MPI_MAX_ERROR_STRING];
    int error_string_len;
    MPI_Error_string(status, error_string, &error_string_len);
    PRINT_DEBUG_ERROR(my_rank, "%s: %s\n", message, error_string);
    exit(1);
}


static void create_benchmark_filename(char *benchmark_log_filename, const char *logs_dir) {
    strncpy(benchmark_log_filename, logs_dir, MAX_LOG_FILENAME_LENGTH);
    strncat(benchmark_log_filename, BENCHMARK_FILE_NAME, MAX_LOG_FILENAME_LENGTH);
}
static void open_benchmark_file(const int my_rank, const char *filename, MPI_File *mpi_file) {
    if (my_rank == 0) {
        remove(filename);
    }

    const int ret = MPI_File_open(MPI_COMM_WORLD, filename,MPI_MODE_CREATE | MPI_MODE_WRONLY | MPI_MODE_APPEND,MPI_INFO_NULL, mpi_file);
    if (ret != MPI_SUCCESS) {
        handle_mpi_error_and_exit(my_rank, ret, "[BENCHMARK] Cannot open benchmark log file");
    }
}

static void benchmark_unique_log(const int my_rank, const MPI_File *file, const char *format, ...) {
    if (my_rank == 0) {
        char buffer[4096];
        va_list args;
        va_start(args, format);
        const int len = vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);

        if (len > 0) {
            const int ret = MPI_File_write(*file, buffer, len, MPI_CHAR, MPI_STATUS_IGNORE);

            if (ret != MPI_SUCCESS) {
                handle_mpi_error_and_exit(my_rank, ret, "[BENCHMARK] Error writing to benchmark file");
            }
        }
    }
}
/*static void benchmark_all_log(const int my_rank, const MPI_File *file, const char *format, ...) {
    char buffer[4096];
    va_list args;
    va_start(args, format);
    const int len = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    if (len > 0) {
        const int ret = MPI_File_write_all(*file, buffer, len, MPI_CHAR, MPI_STATUS_IGNORE);

        if (ret != MPI_SUCCESS) {
            handle_mpi_error_and_exit(my_rank, ret, "[BENCHMARK] Error writing to benchmark file");
        }
    }
}*/

static void write_benchmark_log_header(const int my_rank, const MPI_File *file, const char *benchmark_name) {
    // print date and time
    if (my_rank == 0) {
        const time_t t = time(NULL);
        const struct tm tm = *localtime(&t);

        benchmark_unique_log(my_rank, file, "***\n");
        benchmark_unique_log(my_rank, file, "Benchmark started at: %d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
        benchmark_unique_log(my_rank, file, "Benchmark Name: %s\n", benchmark_name);
    }

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // print processor name for each process
    if (my_rank == 0) {
        char node_name[MPI_MAX_PROCESSOR_NAME];
        int name_len;
        MPI_Get_processor_name(node_name, &name_len);

        /* print own name */
        benchmark_unique_log(my_rank, file, "Rank: %d > node: %s\n", 0, node_name);

        /* receive and print names from other ranks */
        for (int r = 1; r < world_size; r++) {
            char recv_name[MPI_MAX_PROCESSOR_NAME];
            MPI_Recv(recv_name, MPI_MAX_PROCESSOR_NAME, MPI_CHAR, r, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            benchmark_unique_log(my_rank, file, "Rank: %d > node: %s\n", r, recv_name);
        }
    } else {
        char node_name[MPI_MAX_PROCESSOR_NAME];
        int name_len;
        MPI_Get_processor_name(node_name, &name_len);
        /* send null-terminated name to rank 0 */
        MPI_Send(node_name, name_len + 1, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
    }

    benchmark_unique_log(my_rank, file, "***\n");
}

void benchmark_init(const int my_rank, const char *logs_dir, MPI_File *benchmark_log_file) {
    char benchmark_log_filename[MAX_LOG_FILENAME_LENGTH];

    create_benchmark_filename(benchmark_log_filename, logs_dir);
    open_benchmark_file(my_rank, benchmark_log_filename, benchmark_log_file);

    UNIQUE_PRINT_DEBUG_INFO(my_rank, "[BENCHMARK] Output log to: %s\n", benchmark_log_filename);
}
void benchmark_finalize(const int my_rank, MPI_File *benchmark_log_file) {
    MPI_File_close(benchmark_log_file);
}

void benchmark_run(const int my_rank, const char* benchmark_name, const Application app, const MPI_File *benchmark_log_file) {
    write_benchmark_log_header(my_rank, benchmark_log_file, benchmark_name);

    MPI_Barrier(MPI_COMM_WORLD);
    const double start_time = MPI_Wtime();
    app(my_rank);
    const double end_time = MPI_Wtime();

    benchmark_unique_log(my_rank, benchmark_log_file, "%.20lf\n", end_time - start_time);
}
