#include <mpi.h>
#include <stdio.h>
#include <malloc.h>
#include <time.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>

int isStrike(int x1, int y1, int x2, int y2, int N) {
    if ((x1 == x2) || (y1 == y2)) return 1;

    // Главная диагональ
    int tx, ty; // дополнительные переменные

    // Влево-вверх
    tx = x1 - 1;
    ty = y1 - 1;
    while ((tx >= 1) && (ty >= 1)) {
        if ((tx == x2) && (ty == y2)) return 1;
        tx--;
        ty--;
    }

    // Вправо-вниз
    tx = x1 + 1;
    ty = y1 + 1;
    while ((tx <= N) && (ty <= N)) {
        if ((tx == x2) && (ty == y2)) return 1;
        tx++;
        ty++;
    }

    // Дополнительная диагональ
    // Вправо-вверх
    tx = x1 + 1;
    ty = y1 - 1;
    while ((tx <= N) && (ty >= 1)) {
        if ((tx == x2) && (ty == y2)) return 1;
        tx++;
        ty--;
    }

    // Влево-вниз
    tx = x1 - 1;
    ty = y1 + 1;
    while ((tx >= 1) && (ty <= N)) {
        if ((tx == x2) && (ty == y2)) return 1;
        tx--;
        ty++;
    }
    return 0;
}

int checkCoordinate(const int M[], int p, int N) {
    int px, py, x, y;
    int i;

    px = M[p];
    py = p;

    for (i = 0; i <= p - 1; i++) {
        x = M[i];
        y = i;
        if (isStrike(x, y, px, py, N))
            return 1;
    }
    return 0;
}

void print_answer (double seconds, int res)
{
    std::ofstream out ("out.txt", std::ios::out);
    out<<"время работы = "<<seconds<<"\n";
    out<<"количество вариантов= "<<res<<"\n";
}

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);
    int N = 5;
    int world_size;
    int world_rank;
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    clock_t start, end;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    printf("%d\n", world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Get_processor_name(processor_name, &name_len);
    MPI_Status status;

    if (world_rank == 0) {
        start = clock();
        int partSize = N / world_size;
        int shift = N % world_size;

        int from = partSize + shift;
        int to = from + partSize;
        int toForRoot = from;

        for (int i = world_rank + 1; i < world_size; ++i) {
            printf("world_rankTo%d\n", i);
            int buff[2];
            buff[0] = from;
            buff[1] = to;
            MPI_Send(buff, 2, MPI_INT, i, 0, MPI_COMM_WORLD);
            from = to;
            to = to + partSize;
        }
        printf("toForRoot %d \n", toForRoot);
        printf("partSize %d \n", partSize);
        printf("shift %d \n", shift);
        printf("world_size %d \n", world_size);

        int *M = (int*)malloc(N * sizeof(int));
        int k = 0;
        for (int i = 1; i < toForRoot; i++) {
            int p = 1;
            M[0] = i;
            M[p] = 0;
            while (p >= 1) {
                M[p] = M[p] + 1;
                if (p == N - 1) {
                    if (M[p] > N) while (M[p] > N) p--;
                    else {
                        if (!checkCoordinate(M, p, N)) {
                            char a1[5];
                            for(int i = 0; i < N; i++) a1[i] = M[i] + '0';
                            printf("%s\n", a1);
                            k++;
                            p--;
                        }
                    }
                } else {
                    if (M[p] > N) while (M[p] > N) p--;
                    else {
                        if (!checkCoordinate(M, p, N)) {
                            p++;
                            M[p] = 0;
                        }
                    }
                }
            }
        }
        free(M);
        int buffer;
        printf("Answer %d from processor %s, rank %d out of %d processors\n", k,
               processor_name, world_rank, world_size);
        for (int i = world_rank+1; i < world_size; ++i) {
            MPI_Recv(&buffer, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &status);
            k += buffer;
        }
        end = clock();
        double seconds = (double)(end - start) / CLOCKS_PER_SEC;
        printf("Time %lf\n", seconds);
        printf("Answer: %d\n", k);
        print_answer(seconds, k);

    } else{
        int *buff = (int*)malloc(2 * sizeof(int));
        MPI_Recv(buff, 2, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);

        int *M = (int*)malloc(N * sizeof(int));
        int k = 0;
        for (int i = buff[0]; i < buff[1]; i++) {
            int p = 1;
            M[0] = i;
            M[p] = 0;
            while (p >= 1) {
                M[p] = M[p] + 1;
                if (p == N - 1) {
                    if (M[p] > N) while (M[p] > N) p--;
                    else {
                        if (!checkCoordinate(M, p, N)) {
                            char a1[5];
                            for(int i = 0; i < N; i++) a1[i] = M[i] + '0';
                            printf("%s", a1);
                            printf("\n");
                            k++;
                            p--;
                        }
                    }
                } else {
                    if (M[p] > N) while (M[p] > N) p--;
                    else {
                        if (!checkCoordinate(M, p, N)) {
                            p++;
                            M[p] = 0;
                        }
                    }
                }
            }
        }
        free(M);
        free(buff);
        printf("Answer %d from processor %s, rank %d out of %d processors\n", k,
               processor_name, world_rank, world_size);
        MPI_Send(&k, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }
    MPI_Finalize();
}