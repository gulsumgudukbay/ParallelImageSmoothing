//
//  smooth-mpi-v1.c
//  Image_Smoothing
//
//  Created by Gulsum on 20/10/2017.
//  Copyright © 2017 Gulsum. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#define INT_MIN -2147483648

float* apply_filter( float* arr, float* kernel, int row_size, int col_size, int rank, int np, int kernel_dim, int *out_row_size)
{
    float sum;
    float o_row_size;
    
    if( rank == 0 || rank == np-1)
    {
        o_row_size = row_size - 1;
        *out_row_size = o_row_size;
    }
    else
    {
        o_row_size = row_size - 2;
        *out_row_size = o_row_size;
    }
    
    float* output_image = malloc( o_row_size * col_size * sizeof(float));
    int offset = (kernel_dim - 1) / 2;
    
    if( rank == 0)
    {
        for( int i = 0; i < row_size; i++)
        {
            for( int j = 0; j < col_size; j++)
            {
                sum = 0;
                for( int k1 = 0; k1 < kernel_dim; k1++)
                {
                    if( i + k1 < offset || i + k1 - offset >= row_size ) //out of vertical boundaries
                    {
                        continue;
                    }
                    
                    for( int k2 = 0; k2 < kernel_dim; k2++)
                    {
                        if( j + k2 < offset || j + k2 - offset >= col_size) //out of horizontal boundaries
                        {
                            continue;
                        }
                        
                        sum += arr[(i + k1 - offset) * col_size + j + k2 - offset] * kernel[k1 * kernel_dim + k2];
                    }
                }
                if( i != row_size - 1)
                    output_image[i * col_size + j] = sum;
            }
        }
    }
    else if( rank == np-1)
    {
        for( int i = 0; i < row_size; i++)
        {
            for( int j = 0; j < col_size; j++)
            {
                sum = 0;
                for( int k1 = 0; k1 < kernel_dim; k1++)
                {
                    if( i + k1 < offset || i + k1 - offset >= row_size ) //out of vertical boundaries
                    {
                        continue;
                    }
                    
                    for( int k2 = 0; k2 < kernel_dim; k2++)
                    {
                        if( j + k2 < offset || j + k2 - offset >= col_size) //out of horizontal boundaries
                        {
                            continue;
                        }
                        
                        sum += arr[(i + k1 - offset) * col_size + j + k2 - offset] * kernel[k1 * kernel_dim + k2];
                    }
                }
                if( i != 0)
                    output_image[(i-1) * col_size + j] = sum;
            }
        }
    }
    else
    {
        for( int i = 0; i < row_size; i++)
        {
            for( int j = 0; j < col_size; j++)
            {
                sum = 0;
                for( int k1 = 0; k1 < kernel_dim; k1++)
                {
                    if( i + k1 < offset || i + k1 - offset >= row_size) //out of boundaries
                    {
                        continue;
                    }
                    
                    for( int k2 = 0; k2 < kernel_dim; k2++)
                    {
                        if( j + k2 < offset || j + k2 - offset >= col_size) //out of boundaries
                        {
                            continue;
                        }
                        
                        sum += arr[(i + k1 - offset) * col_size + j + k2 - offset] * kernel[k1 * kernel_dim + k2];
                    }
                }
                if( i != 0 && i != row_size-1)
                output_image[(i-1) * col_size + j] = sum;
            }
        }
    }
    
    return output_image;
}


int main(int argc, char *argv[])
{
    int my_rank, my_size;
    
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &my_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    
    int read_integer, row_size, kernel_dim;
    FILE* input_image_file, *input_kernel_file, *output_image_file;
    
    char *input_image_file_name, *input_kernel_file_name;
    float *image;
    float *kernel;
    float *output_image;
    
    row_size = 0;
    kernel_dim = 3;
    
    
    if(my_rank == 0)
    {
        input_image_file_name = argv[1];
        input_kernel_file_name = argv[2];
        
        input_image_file = fopen( input_image_file_name, "r+");
        input_kernel_file = fopen( input_kernel_file_name, "r+");
        output_image_file = fopen ("output_image.txt", "w+");

        if( fscanf( input_image_file, "%d", &read_integer) != EOF)
        {
            row_size = read_integer;
        }
        
        image = malloc( row_size * row_size * sizeof(float));
        output_image = malloc( row_size * row_size * sizeof(float));
        kernel = malloc( kernel_dim * kernel_dim * sizeof(float));
        
       
        for(int i = 0; i < row_size; i++)
            for(int j = 0; j < row_size; j++)
                output_image[i * row_size + j] = 0;
        
        for( int i = 0; i < row_size; i++)
        {
            for( int j = 0; j < row_size; j++)
            {
                if( fscanf( input_image_file, "%d", &read_integer) != EOF)
                {
                    image[i * row_size + j] = read_integer;
                }
            }
        }

        int kernel_sum = 0;
        for( int i = 0; i < kernel_dim; i++)
        {
            for( int j = 0; j < kernel_dim; j++)
            {
                if( fscanf( input_kernel_file, "%d", &read_integer) != EOF)
                {
                    kernel[i * kernel_dim + j] = read_integer;
                    kernel_sum += kernel[i * kernel_dim + j];
                }
            }
        }
        
        for( int i = 0; i < kernel_dim; i++)
        {
            for( int j = 0; j < kernel_dim; j++)
            {
                kernel[i * kernel_dim + j] = kernel[i * kernel_dim + j] / kernel_sum;
            }
        }

        //send the kernel!
        for( int q = 0; q < my_size; q++)
        {
            if( q != my_rank)
            {
                MPI_Send((void *) kernel, kernel_dim * kernel_dim, MPI_FLOAT, q, 6, MPI_COMM_WORLD );
            }
        }
        
        //close the files
        fclose(input_image_file);
        fclose(input_kernel_file);
        
        int row_size_portion = row_size / my_size;
        int rem = row_size % my_size;

        for( int q = 1; q < my_size; q++)
        {
            
            MPI_Send((void *) &row_size, 1, MPI_INT, q, 8, MPI_COMM_WORLD );
            int sent_portion_size = 0;
            if( q < rem)
            {
                sent_portion_size = row_size_portion + 3;
                MPI_Send((void *) &sent_portion_size, 1, MPI_INT, q, 7, MPI_COMM_WORLD);
                
                MPI_Send((void *)(image + (q * row_size * (sent_portion_size+1) -1)), sent_portion_size * row_size, MPI_FLOAT, q, 9, MPI_COMM_WORLD);
                
                float* sent_local = (image + (q * row_size * (sent_portion_size+1) -1));
                char* filename_local = malloc( 20 * sizeof(char));
                sprintf(filename_local,"local_img_file_%d.txt", q);
                FILE* local_img = fopen( filename_local, "w+");
                for( int i = 0; i < sent_portion_size; i++)
                {
                    for(int j = 0; j < row_size; j++)
                    {
                        fprintf(local_img, "%d ", (int)sent_local[i*row_size+j]);
                    }
                    fprintf(local_img, "\n");
                }
                fclose(local_img);
                free(filename_local);
            }
            else
            {
                
                sent_portion_size = row_size_portion + 2;
                
                if( q == my_size-1)
                    sent_portion_size--;
                MPI_Send((void *)&sent_portion_size, 1, MPI_INT, q, 7, MPI_COMM_WORLD);
                
                MPI_Send((void *)(image + q * row_size * sent_portion_size + rem - 1), sent_portion_size * row_size, MPI_FLOAT, q, 9, MPI_COMM_WORLD);
                
                float* sent_local = (image + q * row_size * sent_portion_size + rem - 1);
                printf("SENT INDEX: %d\n", q*sent_portion_size + rem -1);
                char* filename_local = malloc( 20 * sizeof(char));
                sprintf(filename_local,"local_img_file_%d.txt", q);
                FILE* local_img = fopen( filename_local, "w+");
                for( int i = 0; i < sent_portion_size; i++)
                {
                    for(int j = 0; j < row_size; j++)
                    {
                        fprintf(local_img, "%d ", (int)sent_local[i*row_size+j]);
                    }
                    fprintf(local_img, "\n");
                }
                fclose(local_img);
                free(filename_local);
            }
            
        }
        
        //process own part
        int master_portion_size = 0;
        if( 0 <= rem)
        {
            master_portion_size = row_size_portion + 1;
        }
        else
        {
            master_portion_size = row_size_portion;
        }
        int master_out_size = 0;
        float* master_portion = apply_filter( image, kernel, master_portion_size, row_size, my_rank, my_size, kernel_dim, &master_out_size);
        for( int i = 0; i < master_out_size; i++)
        {
            for( int j = 0; j < row_size; j++)
            {
                output_image[i * row_size + j] = master_portion[i * row_size + j];
            }
        }
        
        int row_index_out = 0;
        int no_of_rows = 0;
        
        //merge!
        
        for( int q = 1; q < my_size; q++)
        {
            MPI_Recv((void *) &no_of_rows, 1, MPI_INT, q, 10, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            
            float *local_image = malloc(no_of_rows * row_size * sizeof(float));
            
            MPI_Recv((void *) local_image, no_of_rows * row_size, MPI_FLOAT, q, 11, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            

            for( int i = 0; i < no_of_rows; i++)
            {
                for(int j = 0; j < row_size; j++)
                {
                    output_image[(master_out_size + row_index_out) * row_size + j] = local_image[i * row_size + j];
                }
                row_index_out++;
            }
            
            free(local_image);
        }
        
        for( int i = 0; i < row_size; i++)
        {
            for( int j = 0; j < row_size; j++)
            {
                fprintf(output_image_file, "%f ", output_image[i * row_size +j]);
            }
            if( i != row_size-1)
            fprintf(output_image_file, "\n");
        }
        
        
        fclose(output_image_file);
        printf("Look at the output!\n");

        
    }
    else //CHILDREN
    {
        float *local_image, *kernel_local;
        int row_size_portion = 0;
        int col_size = 0;
        //receive kernel

        kernel_local = malloc( kernel_dim * kernel_dim * sizeof(float));
        
        MPI_Recv((void *) kernel_local, kernel_dim * kernel_dim, MPI_FLOAT, 0, 6, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        //receive col size
        MPI_Recv((void *) &col_size, 1, MPI_INT, 0, 8, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        //receive no of rows per process
        MPI_Recv((void *) &row_size_portion, 1, MPI_INT, 0, 7, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("row size portion received: %d\n", row_size_portion);
        
        local_image = malloc( row_size_portion * col_size * sizeof(float));

        MPI_Recv((void *) local_image, row_size_portion * col_size, MPI_FLOAT, 0, 9, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        int out_row_size = 0;

        float* local_output = apply_filter( local_image, kernel_local, row_size_portion, col_size, my_rank, my_size, kernel_dim, &out_row_size);

        MPI_Send((void *) &out_row_size, 1, MPI_INT, 0, 10, MPI_COMM_WORLD);

        MPI_Send((void *)local_output, out_row_size * col_size, MPI_FLOAT, 0, 11, MPI_COMM_WORLD);

        free(local_image);
        
    }
    
    
    //////////////////////////////
    
    MPI_Finalize();
    
    //free stuff
    /*
    
    free(image);
    free(kernel);
    free(output_image);
    
    */
    
    return 0;
}


