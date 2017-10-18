//
//  main.c
//  mpi_hello
//
//  Created by Gulsum on 17/10/2017.
//  Copyright © 2017 Gulsum. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <mpi.h>

const int MAX_STRING = 100;

int main(int argc, char *argv[]) {
    
    int my_rank, p, q;
    char greeting[MAX_STRING];
    MPI_Status status;
    
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
        
    if(my_rank == 0)
    {
        for( q = 1; q < p; q++)
        {
            sprintf( greeting, "Process %d of %d got the message!\n", q, p );
            MPI_Send((void*)&greeting, sizeof(greeting), MPI_CHAR, q, 9, MPI_COMM_WORLD );
        }
    }
    else
    {
        MPI_Recv((void*)&greeting, sizeof(greeting), MPI_CHAR, 0, 9, MPI_COMM_WORLD, &status);
        printf("%s", greeting);
    }
    
    MPI_Finalize();
    return 0;
}
