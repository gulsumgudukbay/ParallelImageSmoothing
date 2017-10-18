//
//  max-serial.c
//
//  Created by Gulsum on 18/10/2017.
//  Copyright © 2017 Gulsum. All rights reserved.
//

#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    
    int read_integer, size, max, index;
    FILE* input_file;
    char *input_file_name;
    int *data;
    
    size = 0;
    index = 0;
    max = INT_MIN;
    input_file_name = argv[1];
    input_file = fopen( input_file_name, "r+");
    
    while( fscanf( input_file, "%d", &read_integer) != EOF)
    {
        size++;
    }
    
    data = ( int * )malloc( size * sizeof(int));
    
    while( fscanf( input_file, "%d", &read_integer) != EOF)
    {
        data[index] = read_integer;
    }
    
    for( int i = 0; i < size; i++)
        if( max < data[i])
            max = data[i];
    
    printf("The maximum is %d\n", max);
    return 0;
}