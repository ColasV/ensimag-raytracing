#include <mpi.h>
#include <stdio.h>

int main(int argc, char **argv) {
    int taille, rang, hostlen;
    char hostname[MPI_MAX_PROCESSOR_NAME]={};
    double message=rang+123;
    int TAG = 123456;
    MPI_Status status;
    MPI_Init(& argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &taille);
    MPI_Comm_rank(MPI_COMM_WORLD, &rang);
    MPI_Get_processor_name(hostname, &hostlen);
    if (rang == 0) {
        MPI_Send(& message, 1 , MPI_DOUBLE,
                (rang +1)%taille, TAG, MPI_COMM_WORLD);
        MPI_Recv(& message, 1, MPI_DOUBLE,
                (taille -1), TAG, MPI_COMM_WORLD, & status);
        printf("Message %g reçu ! %d parmi %d sur %s!\n",
                message, rang, taille, hostname);
    } else {
        MPI_Recv(& message, 1, MPI_DOUBLE,
                (rang -1), TAG, MPI_COMM_WORLD, & status);
        printf("Message %g reçu ! %d parmi %d sur %s!\n",
                message, rang, taille, hostname);
        MPI_Send(& message, 1 , MPI_DOUBLE,
                (rang +1)%taille, TAG, MPI_COMM_WORLD);
    }
    MPI_Finalize();
}
