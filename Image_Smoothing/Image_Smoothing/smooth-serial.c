//
//  smooth-serial
//  ImageSmoothing
//
//  Created by Gulsum on 19/10/2017.
//  Copyright © 2017 Gulsum. All rights reserved.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define INT_MIN -2147483648

int main(int argc, char *argv[]) {
    
    int read_integer, row_size, index, kernel_dim;
    FILE* input_image_file, *input_kernel_file, *output_image_file;
    
    char *input_image_file_name, *input_kernel_file_name;
    float **image;
    float **kernel;
    float **output_image;
    
    row_size = 0;
    kernel_dim = 3;
    index = 0;
    
    input_image_file_name = argv[1];
    input_kernel_file_name = argv[2];
    
    input_image_file = fopen( input_image_file_name, "r+");
    input_kernel_file = fopen( input_kernel_file_name, "r+");
    output_image_file = fopen ("output_image.txt", "w+");
    
    
    if( fscanf( input_image_file, "%d", &read_integer) != EOF)
    {
        row_size = read_integer;
    }
    
    image = malloc( row_size * sizeof(float*));
    
    for( int i = 0; i < row_size; i++)
    {
        image[i] = malloc( row_size * sizeof(float));
    }
    
    output_image = malloc( row_size * sizeof(float*));
    
    for( int i = 0; i < row_size; i++)
    {
        output_image[i] = malloc( row_size * sizeof(float));
    }
    
    
    kernel = malloc( kernel_dim * sizeof(float*));
    
    for( int i = 0; i < kernel_dim; i++)
    {
        kernel[i] = malloc( kernel_dim * sizeof(float));
    }
    
    for(int i = 0; i < row_size; i++)
        for(int j = 0; j < row_size; j++)
            output_image[i][j] = 0;
    
    for( int i = 0; i < row_size; i++)
    {
        for( int j = 0; j < row_size; j++)
        {
            if( fscanf( input_image_file, "%d", &read_integer) != EOF)
            {
                image[i][j] = read_integer;
            }
        }
    }
    
    float kernel_sum = 0;
    for( int i = 0; i < kernel_dim; i++)
    {
        for( int j = 0; j < kernel_dim; j++)
        {
            if( fscanf( input_kernel_file, "%d", &read_integer) != EOF)
            {
                kernel[i][j] = read_integer;
                kernel_sum += kernel[i][j];
            }
        }
    }
    
    for( int i = 0; i < kernel_dim; i++)
    {
        for( int j = 0; j < kernel_dim; j++)
        {
            kernel[i][j] = kernel[i][j] / kernel_sum;
            printf("%f\n", kernel[i][j]);
        }
    }
    
    float sum;
    
    int offset = (kernel_dim - 1) / 2;
    for( int i = 0; i < row_size; i++)
    {
        for( int j = 0; j < row_size; j++)
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
                    if( j + k2 < offset || j + k2 - offset >= row_size) //out of boundaries
                    {
                        continue;
                    }
                    
                    sum += image[i + k1 - offset][j + k2 - offset] * kernel[k1][k2];
                }
            }
            output_image[i][j] = sum;
            
        }
    }
    
    for( int i = 0; i < row_size; i++)
    {
        for( int j = 0; j < row_size; j++)
        {
            fprintf(output_image_file, "%f ", output_image[i][j]);
        }
        fprintf(output_image_file, "\n");
    }
    
    //free stuff
    for( int i = 0; i < row_size; i++)
    {
        free(image[i]);
        free(output_image[i]);
    }
    
    for( int i = 0; i < kernel_dim; i++)
    {
        free(kernel[i]);
    }
    
    free(image);
    free(kernel);
    free(output_image);
    
    fclose(input_image_file);
    fclose(input_kernel_file);
    fclose(output_image_file);
    printf("Look at the output!\n");
    return 0;
}


