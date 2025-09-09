#ifndef FUN_H
#define FUN_H

#include "param.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void clearBuffers(Buffer buf);                            // Function to clear both in and out buffers
int fillOutBuffer(Buffer buf,int byteCount);              // Function to fill out-buffer with random data
void printBuffer(const char* name,Buffer buf,int index);  // Function to print contents of both buffers

#endif