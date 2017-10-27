//
//  smooth-mpi-v2.c
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
#include <math.h>

int log_base_2( int a)
{
    return (int)(log10((double)a)/log10((double)2));
}

float* apply_filter( float* arr, float* kernel, int row_size, int col_size, int rank, int np, int kernel_dim, int out_row_size)
{
    float sum;
    
    float* output_image = malloc( out_row_size * out_row_size * sizeof(float));
    int offset = (kernel_dim - 1) / 2;
    
    
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
            
            
            //////////////
            int sqrt_np = sqrt(np);
            if( rank == 0) //first row first column
            {
                if( i != row_size - 1 && j != col_size - 1 )
                    output_image[i * out_row_size + j] = sum;
                if(i == 0 && j == 0)
                    printf( "Hello from case 1\n");
            }
            else if( (rank % sqrt_np == 0) && (rank / sqrt_np > 0) && (rank / sqrt_np < sqrt_np-1) ) //first col middle rows
            {
                printf( "Here\n");

                if( i != 0 && i != row_size - 1 && j != col_size - 1 )
                    output_image[(i-1) * out_row_size + j] = sum;
                if(i == 0 && j == 0)
                    printf( "Hello from case 2\n");
            }
            else if( (rank % sqrt_np == 0) && (rank/ sqrt_np == sqrt_np-1) ) // first col last row
            {
                if( i != 0 && i != row_size - 1 && j != col_size - 1 )
                    output_image[(i-1) * out_row_size + j] = sum;
                if(i == 0 && j == 0)
                    printf( "Hello from case 3\n");
            }
            else if( (rank % sqrt_np > 0) && (rank / sqrt_np == 0) && (rank % sqrt_np < sqrt_np-1)  ) // first row middle columns
            {
                if( j != 0 && j != col_size-1)
                    output_image[i * out_row_size + j-1] = sum;
                if(i == 0 && j == 0)
                    printf( "Hello from case 4\n");
            }
            else if( (rank % sqrt_np > 0 ) && (rank % sqrt_np < sqrt_np - 1) && (rank / sqrt_np > 0) && (rank / sqrt_np < sqrt_np - 1) ) // middle columns middle rows
            {
                if( j != 0 && j != col_size-1 && i != 0 && i != row_size-1)
                    output_image[(i-1) * out_row_size + j-1] = sum;
                if(i == 0 && j == 0)
                    printf( "Hello from case 5\n");
            }
            else if( (rank % sqrt_np > 0) && ( rank % sqrt_np < sqrt_np-1) && (rank / sqrt_np == sqrt_np-1) ) // last row middle columns
            {
                if( j != 0 && j != col_size-1 && i != row_size-1)
                    output_image[i * out_row_size + j-1] = sum;
                if(i == 0 && j == 0)
                    printf( "Hello from case 6\n");
            }
            else if( (rank % sqrt_np == sqrt_np - 1) && (rank / sqrt_np == 0)) //first row last col
            {
                if( i != row_size-1 && j != 0)
                    output_image[i * out_row_size + j-1] = sum;
                if(i == 0 && j == 0)
                    printf( "Hello from case 7\n");
            }
            else if( (rank % sqrt_np == sqrt_np - 1) && ( rank / sqrt_np > 0) && ( rank / sqrt_np < sqrt_np - 1) ) //last col middle rows
            {
               if( j != 0 && i != row_size-1 && i != 0)
                   output_image[(i-1) * out_row_size + j-1] = sum;
                if(i == 0 && j == 0)
                    printf( "Hello from case 8\n");
            }
            else if (( rank % sqrt_np == sqrt_np-1) && ( rank / sqrt_np == sqrt_np-1) ) //last row last column
            {
                if( j != 0 && i != row_size-1)
                    output_image[i * out_row_size + j-1] = sum;
                if(i == 0 && j == 0)
                    printf( "Hello from case 9\n");
            }
            ///////////////
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
    int n = atoi(argv[3]);
    
    if(my_rank == 0)
    {
        input_image_file_name = argv[1];
        input_kernel_file_name = argv[2];
        
        input_image_file = fopen( input_image_file_name, "r+");
        input_kernel_file = fopen( input_kernel_file_name, "r+");
        output_image_file = fopen ("smooth-mpi-v2-output.txt", "w+");
        
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
        
        int sqrt_np = (int)sqrt(my_size);
        int no_of_rows_per_process = row_size / sqrt_np;
        int no_of_cols_per_process = row_size / sqrt_np;
        float *sent_portion;

        for( int q = 1; q < my_size; q++)
        {
            MPI_Send((void *) &no_of_rows_per_process, 1, MPI_INT, q, 3, MPI_COMM_WORLD );

            if( (q % sqrt_np == 0) && (q / sqrt_np > 0) && (q / sqrt_np < sqrt_np-1) ) //first col middle rows
            {
                int sent_no_of_cols_per_process = no_of_cols_per_process + 1;
                int sent_no_of_rows_per_process = no_of_rows_per_process + 2;

                MPI_Send((void *) &sent_no_of_cols_per_process, 1, MPI_INT, q, 8, MPI_COMM_WORLD );
                MPI_Send((void *) &sent_no_of_rows_per_process, 1, MPI_INT, q, 7, MPI_COMM_WORLD);
                
                sent_portion = malloc( sent_no_of_cols_per_process * sent_no_of_rows_per_process * sizeof(float));
                
                for( int i = 0; i < sent_no_of_rows_per_process; i++)
                {
                    for( int j = 0; j < sent_no_of_cols_per_process; j++)
                    {
                        sent_portion[i * sent_no_of_cols_per_process + j] = image[(i + no_of_rows_per_process * ((q/sqrt_np))-1) * sent_no_of_cols_per_process + j];
                    }
                }
                MPI_Send((void *)sent_portion, sent_no_of_cols_per_process * sent_no_of_rows_per_process, MPI_FLOAT, q, 9, MPI_COMM_WORLD);

            }
            else if( (q % sqrt_np == 0) && (q / sqrt_np == sqrt_np-1) ) // first col last row
            {
                int sent_no_of_cols_per_process = no_of_cols_per_process + 1;
                int sent_no_of_rows_per_process = no_of_rows_per_process + 1;
                
                MPI_Send((void *) &sent_no_of_cols_per_process, 1, MPI_INT, q, 8, MPI_COMM_WORLD );
                MPI_Send((void *) &sent_no_of_rows_per_process, 1, MPI_INT, q, 7, MPI_COMM_WORLD);
                
                sent_portion = malloc( sent_no_of_cols_per_process * sent_no_of_rows_per_process * sizeof(float));
                
                for( int i = 0; i < sent_no_of_rows_per_process; i++)
                {
                    for( int j = 0; j < sent_no_of_cols_per_process; j++)
                    {
                        sent_portion[i * sent_no_of_cols_per_process + j] = image[(i + no_of_rows_per_process * (q/sqrt_np) - 1) * sent_no_of_cols_per_process + j];
                    }
                }
                
                MPI_Send((void *)sent_portion, sent_no_of_cols_per_process * sent_no_of_rows_per_process, MPI_FLOAT, q, 9, MPI_COMM_WORLD);
            }
            else if( (q % sqrt_np > 0) && (q / sqrt_np == 0) && (q % sqrt_np < sqrt_np-1)  ) // first row middle columns
            {
                int sent_no_of_cols_per_process = no_of_cols_per_process + 2;
                int sent_no_of_rows_per_process = no_of_rows_per_process + 1;
                
                MPI_Send((void *) &sent_no_of_cols_per_process, 1, MPI_INT, q, 8, MPI_COMM_WORLD );
                MPI_Send((void *) &sent_no_of_rows_per_process, 1, MPI_INT, q, 7, MPI_COMM_WORLD);
                
                sent_portion = malloc( sent_no_of_cols_per_process * sent_no_of_rows_per_process * sizeof(float));
                
                for( int i = 0; i < sent_no_of_rows_per_process; i++)
                {
                    for( int j = 0; j < sent_no_of_cols_per_process; j++)
                    {
                        sent_portion[i * sent_no_of_cols_per_process + j] = image[i * sent_no_of_cols_per_process + q * no_of_cols_per_process - 1 + j];
                    }
                }
                
                MPI_Send((void *)sent_portion, sent_no_of_cols_per_process * sent_no_of_rows_per_process, MPI_FLOAT, q, 9, MPI_COMM_WORLD);
            }
            else if( (q % sqrt_np > 0 ) && (q % sqrt_np < sqrt_np - 1) && (q / sqrt_np > 0) && (q / sqrt_np < sqrt_np - 1) ) // middle columns middle rows
            {
                int sent_no_of_cols_per_process = no_of_cols_per_process + 2;
                int sent_no_of_rows_per_process = no_of_rows_per_process + 2;
                
                MPI_Send((void *) &sent_no_of_cols_per_process, 1, MPI_INT, q, 8, MPI_COMM_WORLD );
                MPI_Send((void *) &sent_no_of_rows_per_process, 1, MPI_INT, q, 7, MPI_COMM_WORLD);
                
                sent_portion = malloc( sent_no_of_cols_per_process * sent_no_of_rows_per_process * sizeof(float));
                
                for( int i = 0; i < sent_no_of_rows_per_process; i++)
                {
                    for( int j = 0; j < sent_no_of_cols_per_process; j++)
                    {
                        sent_portion[i * sent_no_of_cols_per_process + j] = image[(i + no_of_rows_per_process * (q/sqrt_np) - 1) * sent_no_of_cols_per_process + (q % sqrt_np) * no_of_cols_per_process - 1 + j];
                    }
                }
                
                MPI_Send((void *)sent_portion, sent_no_of_cols_per_process * sent_no_of_rows_per_process, MPI_FLOAT, q, 9, MPI_COMM_WORLD);
            }
            else if( (q % sqrt_np > 0) && ( q % sqrt_np < sqrt_np-1) && (q / sqrt_np == sqrt_np-1) ) // last row middle columns
            {
                int sent_no_of_cols_per_process = no_of_cols_per_process + 2;
                int sent_no_of_rows_per_process = no_of_rows_per_process + 1;
                
                MPI_Send((void *) &sent_no_of_cols_per_process, 1, MPI_INT, q, 8, MPI_COMM_WORLD );
                MPI_Send((void *) &sent_no_of_rows_per_process, 1, MPI_INT, q, 7, MPI_COMM_WORLD);
                
                sent_portion = malloc( sent_no_of_cols_per_process * sent_no_of_rows_per_process * sizeof(float));
                
                for( int i = 0; i < sent_no_of_rows_per_process; i++)
                {
                    for( int j = 0; j < sent_no_of_cols_per_process; j++)
                    {
                        sent_portion[i * sent_no_of_cols_per_process + j] = image[(i + no_of_rows_per_process * (q/sqrt_np) - 1) * sent_no_of_cols_per_process + (q % sqrt_np) * no_of_cols_per_process - 1 + j];
                    }
                }
                
                MPI_Send((void *)sent_portion, sent_no_of_cols_per_process * sent_no_of_rows_per_process, MPI_FLOAT, q, 9, MPI_COMM_WORLD);
            }
            else if( (q % sqrt_np == sqrt_np - 1) && (q / sqrt_np == 0)) //first row last col
            {
                int sent_no_of_cols_per_process = no_of_cols_per_process + 1;
                int sent_no_of_rows_per_process = no_of_rows_per_process + 1;
                
                MPI_Send((void *) &sent_no_of_cols_per_process, 1, MPI_INT, q, 8, MPI_COMM_WORLD );
                MPI_Send((void *) &sent_no_of_rows_per_process, 1, MPI_INT, q, 7, MPI_COMM_WORLD);
                
                sent_portion = malloc( sent_no_of_cols_per_process * sent_no_of_rows_per_process * sizeof(float));
                
                for( int i = 0; i < sent_no_of_rows_per_process; i++)
                {
                    for( int j = 0; j < sent_no_of_cols_per_process; j++)
                    {
                        sent_portion[i * sent_no_of_cols_per_process + j] = image[i * sent_no_of_cols_per_process + (q % sqrt_np) * no_of_cols_per_process - 1 + j];
                    }
                }
                
                MPI_Send((void *)sent_portion, sent_no_of_cols_per_process * sent_no_of_rows_per_process, MPI_FLOAT, q, 9, MPI_COMM_WORLD);
            }
            else if( (q % sqrt_np == sqrt_np - 1) && ( q / sqrt_np > 0) && ( q / sqrt_np < sqrt_np - 1) ) //last col middle rows
            {
                int sent_no_of_cols_per_process = no_of_cols_per_process + 1;
                int sent_no_of_rows_per_process = no_of_rows_per_process + 2;
                
                MPI_Send((void *) &sent_no_of_cols_per_process, 1, MPI_INT, q, 8, MPI_COMM_WORLD );
                MPI_Send((void *) &sent_no_of_rows_per_process, 1, MPI_INT, q, 7, MPI_COMM_WORLD);
                
                sent_portion = malloc( sent_no_of_cols_per_process * sent_no_of_rows_per_process * sizeof(float));
                
                for( int i = 0; i < sent_no_of_rows_per_process; i++)
                {
                    for( int j = 0; j < sent_no_of_cols_per_process; j++)
                    {
                        sent_portion[i * sent_no_of_cols_per_process + j] = image[(i + no_of_rows_per_process * ((q/sqrt_np))-1) * sent_no_of_cols_per_process + (q % sqrt_np) * no_of_cols_per_process - 1 + j];
                    }
                }
                
                MPI_Send((void *)sent_portion, sent_no_of_cols_per_process * sent_no_of_rows_per_process, MPI_FLOAT, q, 9, MPI_COMM_WORLD);
            }
            else if (( q % sqrt_np == sqrt_np-1) && ( q / sqrt_np == sqrt_np-1) ) //last row last column
            {
                int sent_no_of_cols_per_process = no_of_cols_per_process + 1;
                int sent_no_of_rows_per_process = no_of_rows_per_process + 1;
                
                MPI_Send((void *) &sent_no_of_cols_per_process, 1, MPI_INT, q, 8, MPI_COMM_WORLD );
                MPI_Send((void *) &sent_no_of_rows_per_process, 1, MPI_INT, q, 7, MPI_COMM_WORLD);
                
                sent_portion = malloc( sent_no_of_cols_per_process * sent_no_of_rows_per_process * sizeof(float));
                
                for( int i = 0; i < sent_no_of_rows_per_process; i++)
                {
                    for( int j = 0; j < sent_no_of_cols_per_process; j++)
                    {
                        sent_portion[i * sent_no_of_cols_per_process + j] = image[(i + no_of_rows_per_process * ((q/sqrt_np))-1) * sent_no_of_cols_per_process + (q % sqrt_np) * no_of_cols_per_process - 1 + j];
                    }
                }
                
                MPI_Send((void *)sent_portion, sent_no_of_cols_per_process * sent_no_of_rows_per_process, MPI_FLOAT, q, 9, MPI_COMM_WORLD);

            }
        
            
        }
        
        //process own part
        int master_row_size = no_of_rows_per_process + 1;
        int master_col_size = no_of_cols_per_process + 1;
        
        int master_row_out = no_of_rows_per_process;
        int master_col_out = no_of_cols_per_process;
        
        float* master_portion = apply_filter( image, kernel, master_row_size, master_col_size, my_rank, my_size, kernel_dim, master_row_out);
        for( int i = 0; i < master_row_out; i++)
        {
            for( int j = 0; j < master_col_out; j++)
            {
                output_image[i * row_size + j] = master_portion[i * row_size + j];
            }
        }
        
        
        int row_index_out = 0;
        int col_index_out = 0;

        //merge!
        
        for( int q = 1; q < my_size; q++)
        {
            float *local_image = malloc(no_of_rows_per_process * no_of_rows_per_process * sizeof(float));
            
            MPI_Recv((void *) local_image, no_of_rows_per_process * no_of_rows_per_process, MPI_FLOAT, q, 11, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            
            printf("Received local img arr from: %d\n", my_rank);
            
            for( int i = 0; i < no_of_rows_per_process; i++)
            {
                for(int j = 0; j < no_of_rows_per_process; j++)
                {
                    output_image[(no_of_cols_per_process + row_index_out) * row_size + col_index_out] = local_image[i * no_of_rows_per_process + j];
                    col_index_out++;
                }
                row_index_out++;
            }
            
            free(local_image);
        }
        printf("after merge\n");

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
        
        //free stuff
        free(image);
        free(kernel);
        free(output_image);
        
        
    }
    else //CHILDREN
    {
        float *local_image, *kernel_local;
        int row_size_portion = 0;
        int col_size = 0;
        int out_row_size = 0;
        //receive kernel
        
        kernel_local = malloc( kernel_dim * kernel_dim * sizeof(float));
        MPI_Recv((void *) &out_row_size, 1, MPI_INT, 0, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        MPI_Recv((void *) kernel_local, kernel_dim * kernel_dim, MPI_FLOAT, 0, 6, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        //receive col size
        MPI_Recv((void *) &col_size, 1, MPI_INT, 0, 8, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        //receive no of rows per process
        MPI_Recv((void *) &row_size_portion, 1, MPI_INT, 0, 7, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        local_image = malloc( row_size_portion * col_size * sizeof(float));
        
        MPI_Recv((void *) local_image, row_size_portion * col_size, MPI_FLOAT, 0, 9, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("appllied filter %d\n", my_rank);

        float* local_output = apply_filter( local_image, kernel_local, row_size_portion, col_size, my_rank, my_size, kernel_dim, out_row_size);

        
        MPI_Send((void *)local_output, out_row_size * out_row_size, MPI_FLOAT, 0, 11, MPI_COMM_WORLD);

        free(local_image);
        
    }
    
    
    //////////////////////////////
    
    MPI_Finalize();
    
    
    
    
    return 0;
}



