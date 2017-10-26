//
//  max-mpi-v1.c
//
//  Created by Gulsum on 18/10/2017.
//  Copyright © 2017 Gulsum. All rights reserved.
//


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#define INT_MIN -2147483648

int main(int argc, char *argv[])
{
    double timeStart = MPI_Wtime();
    int my_rank, my_size;
    
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &my_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    
    int size, max;
    int *data;
    
    max = INT_MIN;
    
    if(my_rank == 0)
    {
        //file reading stuff
        FILE* input_file;
        char *input_file_name;
        int read_integer, index;
        
        size = 0;
        index = 0;
        input_file_name = argv[1];
        input_file = fopen( input_file_name, "r+");
        
        while( fscanf( input_file, "%d", &read_integer) != EOF)
        {
            size++;
        }
        
        int portion_size = size / my_size;
        int remainder = size % my_size;
        
        fseek(input_file, 0, SEEK_SET);
        data = ( int * )malloc( size * sizeof(int));
        
        while( fscanf( input_file, "%d", &read_integer) != EOF)
        {
            data[index] = read_integer;
            index++;
        }
        
        for( int q = 0; q < my_size; q++)
        {
            if( q != my_rank)
            {
                if( q < remainder )
                {
                    int rows = portion_size + 1;
                    MPI_Send((void *) &rows, 1, MPI_INT, q, 7, MPI_COMM_WORLD );
                    MPI_Send((void*)(data + (q * rows)), rows, MPI_INT, q, 9, MPI_COMM_WORLD );
                }
                else
                {
                    int rows = portion_size;
                    MPI_Send((void *) &rows, 1, MPI_INT, q, 7, MPI_COMM_WORLD );
                    MPI_Send((void*)(data + (q * portion_size + remainder)), rows, MPI_INT, q, 9, MPI_COMM_WORLD );
                }
            }
            
        }
        
        int master_portion_size = 0;
        int master_index = 0;
        if( my_rank < remainder)
        {
            master_index = my_rank * (portion_size + 1);
            master_portion_size = portion_size + 1;
        }
        else
        {
            master_index = my_rank * portion_size + remainder;
            master_portion_size = portion_size;
        }
        
        for( int i = master_index; i < master_portion_size; i++)
        {
            if( max < data[i])
                max = data[i];
        }
        
        int cur_max = max;
        
        for( int q = 0; q < my_size; q++)
        {
            if( q != my_rank)
            {
                MPI_Recv((void *) &cur_max, 1, MPI_INT, q, 8, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                if( max < cur_max)
                    max = cur_max;
            }
        }
        printf("The maximum is %d\n", max);
        double timeEnd = MPI_Wtime();
        double timeElapsed = (timeEnd - timeStart) * 1000;
        printf("Time spent: %lf ms\n", timeElapsed);
    }
    else
    {
        int portion_size_child;
        MPI_Recv((void *) &portion_size_child, 1, MPI_INT, 0, 7, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        data = ( int * )malloc( portion_size_child * sizeof(int));
        int child_max = INT_MIN;
        MPI_Recv((void*)data, portion_size_child, MPI_INT, 0, 9, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        for( int i = 0; i < portion_size_child; i++)
        {
            if( child_max < data[i])
                child_max = data[i];
        }
        MPI_Send((void *) &child_max, 1, MPI_INT, 0, 8, MPI_COMM_WORLD );
        
    }
    
    
    MPI_Finalize();
    free(data);


    return 0;
}

