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
        int remainder = size & my_size;
        
        for( int q = 0; q < my_size; q++)
        {
            MPI_Send((void *) &portion_size, 1, MPI_INT, q, 7, MPI_COMM_WORLD );
        }
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
                    MPI_Send((void*)(data + (q * portion_size)), portion_size + 1, MPI_INT, q, 9, MPI_COMM_WORLD );
                }
                else
                {
                    MPI_Send((void*)(data + (q * portion_size)), portion_size, MPI_INT, q, 9, MPI_COMM_WORLD );
                }
            }
            
        }
        
        for( int i = my_rank; i < my_rank + portion_size; i++)
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
        
    }
    else
    {
        MPI_Recv((void *) &size, 1, MPI_INT, 0, 7, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("size received: %d\n", size);
        data = ( int * )malloc( size * sizeof(int));
        int child_max = INT_MIN;
        MPI_Recv((void*)data, sizeof(data)/my_size, MPI_INT, 0, 9, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        for( int i = 0; i < size; i++)
        {
            if( child_max < data[i])
                child_max = data[i];
        }
        MPI_Send((void *) &child_max, 1, MPI_INT, 0, 8, MPI_COMM_WORLD );
        
    }
    
  
    printf("The maximum is %d\n", max);
    
    MPI_Finalize();
    return 0;
}

