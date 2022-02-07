#include <mpi.h>
#include <stdio.h>
#include <malloc.h>

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

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);
    int N = 5;
    int world_size;
    int world_rank;
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    printf("%d\n", world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Get_processor_name(processor_name, &name_len);
    MPI_Status status;

    if (world_rank == 0) {
        int partSize = N / world_size;
        int shift = N % world_size;

        int from = partSize + shift;
        int to = from + partSize;
        int toForRoot = from;

        for (int i = world_rank + 1; i < world_size; ++i) {
            int buff[2];
            buff[0] = from;
            buff[1] = to;
            MPI_Send(&buff, 2, MPI_INT, i, 0, MPI_COMM_WORLD);
            from = to;
            to = to + partSize;
        }
        printf("toForRoot %d \n", toForRoot);
        printf("partSize %d \n", partSize);
        printf("shift %d \n", shift);
        printf("world_size %d \n", world_size);

        int *M = malloc(N * sizeof(int));
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
        for (int i = world_rank+1; i < world_size; ++i) {
            MPI_Recv(&buffer, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &status);
            k += buffer;
        }
        printf("Answer: %d", k);
    } else{
        int n;
        MPI_Probe(world_rank, 0, MPI_COMM_WORLD, &status);
        MPI_Get_count(&status, MPI_DOUBLE, &n);

        int *buff = malloc(n * sizeof(int));
        MPI_Recv(buff, n, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);

        int *M = malloc(N * sizeof(int));
        int k = 0;
        for (int i = buff[0]; i < buff[1]; i++) {
            int p = 1;
            M[0] = i;
            M[p] = 0;
            while (p >= 1) {
                M[p] = M[p] + 1;
                printf("Iteration %d from processor %s, rank %d out of %d processors\n", p,
                       processor_name, world_rank, world_size);
                if (p == N - 1) {
                    if (M[p] > N) while (M[p] > N) p--;
                    else {
                        if (!checkCoordinate(M, p, N)) {
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
        MPI_Send(&k, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    printf("Hello world from processor %s, rank %d out of %d processors\n",
           processor_name, world_rank, world_size);

    MPI_Finalize();
}