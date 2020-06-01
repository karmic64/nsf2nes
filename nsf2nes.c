/* nsf2nes - written by <karmic.c64@gmail.com> */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define lo(i) (i&0xff)
#define hi(i) ((i&0xff00)>>8)

extern uint8_t driver[];
extern uint8_t driverend[];

asm("driver:  .incbin \"drv.bin\"\n\t"
    "driverend:\n\t"
    );

int main(int argc, char *argv[])
{
    const uint8_t driversize = driverend-driver;
    
    if (argc < 2)
    {
        puts("nsf2nes conversion utility by <karmic.c64@gmail.com>");
        puts("Usage: nsf2nes [nsffile]...");
        return EXIT_FAILURE;
    }
    
    for (int arg = 1; arg < argc; arg++)
    {
        if (arg > 1) putchar('\n');
        printf("Reading %s\n", argv[arg]);
        
        FILE* infile = fopen(argv[arg], "rb");
        if (!infile)
        {
            perror("Could not open file");
            continue;
        }
        fseek(infile, 0, SEEK_END);
        size_t insize = ftell(infile);
        if (insize < 0x81)
        {
            fclose(infile);
            puts("File too short to be an NSF");
            continue;
        }
        uint8_t *inbuf = malloc(insize);
        rewind(infile);
        fread(inbuf, 1, insize, infile);
        fclose(infile);
        
        if (memcmp(inbuf, "NESM\x1a", sizeof("NESM\x1a")-1))
        {
            puts("Not an NSF");
            continue;
        }
        int errcnt = 0;
        if (*(inbuf+5) != 1)
        {
            puts("Non-NSF1 NSFs are not supported");
            errcnt++;
        }
        if (*(inbuf+0x7b))
        {
            puts("Expansion sound NSFs are not supported");
            errcnt++;
        }
        if (*(uint32_t*)(inbuf+0x70) | *(uint32_t*)(inbuf+0x74))
        {
            puts("Bankswitched NSFs are not supported");
            errcnt++;
        }
        if (errcnt)
            continue;
        
        uint16_t load = *(inbuf+8) | (*(inbuf+9) << 8);
        uint32_t end = load + (insize-0x80)-1;
        printf("NSF load range: $%04X-$%04X\n", load, end);
        if (load < 0x8000 || load >= 0xfffa)
        {
            puts("Illegal load address");
            continue;
        }
        if (end >= 0x10000)
        {
            puts("NSF data is too long");
            continue;
        }
        else if ((load < (0x8000 | driversize)) && (end >= 0xff00))
        {
            puts("No free space for driver");
            continue;
        }
        else if (end >= 0xfffa)
            puts("Warning: NSF data passes $FFFA. This will be overwritten!");
        uint16_t init = *(inbuf+10) | (*(inbuf+11) << 8);
        uint16_t play = *(inbuf+12) | (*(inbuf+13) << 8);
        printf("NSF init/play: $%04X/$%04X\n", init, play);
        if (init < load || init > end)
        {
            puts("Illegal init address");
            continue;
        }
        uint8_t songs = *(inbuf+6);
        uint8_t defsong = *(inbuf+7);
        printf("NSF songs/default: %u/%u\n", songs, defsong);
        if (!songs)
        {
            puts("Illegal amount of songs");
            continue;
        }
        if (!defsong || defsong > songs)
        {
            puts("Illegal default song");
            continue;
        }
        
        uint8_t driverpage;
        int romsize;
        if ( ((load >= (0x8000 | driversize)) && end < 0xbffa) ||
             ((load >= (0xc000 | driversize)) /*&& end < 0xfffa*/) )
        { /* 16k rom, driver in first page */
            romsize = 0;
            driverpage = 0xc0;
        }
        else if ( (/*load >= 0x8000 &&*/ end < 0xbf00) ||
                  (load >= 0xc000  &&  end < 0xff00) )
        { /* 16k rom, driver in page after data */
            romsize = 0;
            driverpage = (hi(end) + 1) | 0xc0;
        }
        else if (load >= (0x8000 | driversize))
        { /* 32k rom, driver in first page */
            romsize = 1;
            driverpage = 0x80;
        }
        else
        { /* 32k rom, driver in page after data */
            romsize = 1;
            driverpage = hi(end) + 1;
        }
        
        uint16_t bufsize = romsize ? 0x8010 : 0x4010;
        uint8_t *outbuf = malloc(bufsize);
        if (romsize) /* NSF data */
            memcpy(outbuf + 0x10 + (load&0x7fff), inbuf+0x80, insize-0x80);
        else
            memcpy(outbuf + 0x10 + (load&0x3fff), inbuf+0x80, insize-0x80);
        free(inbuf);
        *(outbuf+0) = 'N'; /* iNES header */
        *(outbuf+1) = 'E';
        *(outbuf+2) = 'S';
        *(outbuf+3) = 0x1a;
        *(outbuf+4) = romsize+1;
        for (uint8_t *p = outbuf+5; p < outbuf+0x10; p++)
            *p = 0;
        *(outbuf+bufsize-6) = 0x00; /* nmi = $xx00, reset = $xx04 */
        *(outbuf+bufsize-5) = driverpage;
        *(outbuf+bufsize-4) = 0x04;
        *(outbuf+bufsize-3) = driverpage;
        
        uint8_t *driverdest = outbuf + 0x10 + ((driverpage & (romsize ? 0x7f : 0x3f)) << 8);
        for (int i = 0; i < driversize; i++)
        {
            switch (driver[i])
            {
                case 0x41: /* 0x41 - driver page */
                    driverdest[i] = driverpage;
                    break;
                case 0x42: /* 0x4242 - init */
                    if (driver[i+1] == 0x42)
                    {
                        driverdest[i++] = lo(init);
                        driverdest[i] = hi(init);
                    }
                    else driverdest[i] = driver[i];
                    break;
                case 0x43: /* 0x4343 - play */
                    if (driver[i+1] == 0x43)
                    {
                        driverdest[i++] = lo(play);
                        driverdest[i] = hi(play);
                    }
                    else driverdest[i] = driver[i];
                    break;
                case 0x44: /* 0x44 - max tune */
                    driverdest[i] = songs;
                    break;
                case 0x45: /* 0x45 - default tune */
                    driverdest[i] = defsong-1;
                    break;
                default:
                    driverdest[i] = driver[i];
                    break;
            }
            
        }
        
        FILE* outfile = fopen(strcat(argv[arg], ".nes"), "wb");
        fwrite(outbuf, 1, bufsize, outfile);
        fclose(outfile);
    }
    
    
}