#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

unsigned short crc16( char *ptr, int count)
{
   uint16_t crc;
   char i;
   crc = 0;
   while (--count >= 0)
   {
      crc = crc ^  ( ((int)*ptr)  << 8  ) ;
      ptr=ptr+1;
      i = 8;
      do
      {
         if (crc & 0x8000)
            crc = (crc << 1) ^ 0x1021;
         else
            crc = crc << 1;
      } while(--i);
   }
   return (crc);
}

int main(int argc, char* argv[]) {

    char *filename = argv[1];

    FILE *in = fopen(filename, "rb");
    if (!in) {
        perror("File not opened");
        exit(EXIT_FAILURE);
    }
    fseek(in, 0L, SEEK_END);
    int size = ftell(in);
    rewind(in);
    char *data = malloc(size);
    if (!data) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    fread(data, sizeof(char), size, in);
    
    uint16_t crc = crc16(data, size);
    printf(" 0x%x ", crc);
    exit(0);
}
