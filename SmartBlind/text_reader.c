#include<stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>


char* readFile (){
  FILE *f = fopen("/tmp/arduino.txt", "rb");
  if (f != NULL) {
    fseek(f, 0, SEEK_END);
    long pos = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *bytes = (char*)malloc(pos);
    fread(bytes, pos, 1, f);
    fclose(f);
  
    return bytes;
  } else {
    return NULL; 
  }
}

