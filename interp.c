#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>



uint32_t hex2int(char *hex) {
    uint32_t val = 0;
    while (*hex) {
        // get current character then increment
        uint8_t byte = *hex++; 
        // transform hex character to the 4bit equivalent number, using the ascii table indexes
        if (byte >= '0' && byte <= '9') byte = byte - '0';
        else if (byte >= 'a' && byte <='f') byte = byte - 'a' + 10;
        else if (byte >= 'A' && byte <='F') byte = byte - 'A' + 10;    
        // shift 4 to make space for new digit, and add the 4 bits of the new digit 
        val = (val << 4) | (byte & 0xF);
    }
    return val;
}

uint8_t srec_get_byte_count(char *line)
{
char count_s[3] = {'A','B',0};
  char *count_p = &line[2];
  memcpy(count_s,count_p,2);

  uint8_t count = hex2int(count_s);
}

int srec_checksum(char *line)
{
   
    uint8_t count = srec_get_byte_count(line);
    char *start = &line[2];
    char *end = start+2*(count);;

    char temp[3] = {'A','B',0};

    uint16_t sum =0;

    //Without last two bytes
    for(;start < end;start+=2)
    {
        memcpy(temp,start,2);
        uint8_t data = hex2int(temp);
        sum+= data;
    }
    uint8_t checksum_calc = (sum&0xFF)^0xFF;
   
    memcpy(temp,end,2);
    uint8_t checksum = hex2int(temp);
    if(checksum != checksum_calc)
    {
        printf("\nChecksum incorrect");
        return -1;
    }
    return 0;
}

void srec_parse_header(char *line)
{
  uint8_t count = srec_get_byte_count(line);
  printf("\n\t Header length: %d(0x%02X)",count);

  char header_address[4] = {'0','0','0','0'};
  char *address_p = &line[4];
  if(memcmp(header_address,address_p,4) != 0)
  {
      printf("\nIncorrect address header, should be 0000");
  }

  int byte_count = count -4;
  for(int i = 0; i <= 2*byte_count; i+=2)
  {
      char character[3] = {'A','B',0};
      memcpy(character,&line[8+i],2);
      char hex_val = hex2int(character);
      printf("%c",hex_val);
  }
}

void srec_s1_decode(char *line)
{
    uint8_t count = srec_get_byte_count(line)-3;
    printf("\nData length: %dBytes",count);

    char address16bit[4] = {'0','0','0','0'};
    memcpy(address16bit,&line[4],4);
    uint16_t address = hex2int(address16bit);

    printf("\nAddress: %d(0x%04X)",address,address);

    char *data_start = &line[8];

    printf(" Data: ");

    for(int i = 0; i < 2*count; i+=2)
    {
      char byte_s[3] = {'A','B',0};
      memcpy(byte_s,data_start+i,2);
      uint8_t hex_val = hex2int(byte_s);
      printf("0x%02X,",hex_val);
    }
}

void srec_parse_line(char *line)
{
    

    //Start record should be present
     if(line[0] != 'S') 
     {
         printf("\nWrong Srec line, not started with S");
         return ;
     }
    
    srec_checksum(line);


    switch(line[1])
    {
        case '0' :  printf("\nHeader: "); srec_parse_header(line);break;

        case '1' :  srec_s1_decode(line); break;

        case '9' :  break;

        default: printf("\nNot implemented record type: %c",line[1]); return;
    }




}

int main(void)
{
    printf("Hello");
    FILE *fp = fopen("FilamentSensorproto2.abs.s19", "r");
   if(fp == NULL) 
   {
        perror("Unable to open file!");
        exit(1);
   }
 
   char *line = NULL;
   size_t len  = 0;

   while(getline(&line, &len, fp) != -1) 
   { 
       srec_parse_line(line);
   }
    return 0;
}