// =====================================================================
// DOOM 64 EXTRACTOR
// =====================================================================

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

typedef unsigned char byte;

void show_info(void)
{
    printf("\n");
    printf("     ################(ERICK194)################\n");
    printf("     #            DOOM 64 EXTRACTOR           #\n");
    printf("     #     CREATED BY ERICK VASQUEZ GARCIA    #\n");
    printf("     #              Doom 64.z64               #\n");
    printf("     #         (USA/EUR/JAP) VERSION          #\n");
    printf("     #                                        #\n");
    printf("     # USE:                                   #\n");
    printf("     # Doom64Extractor.exe -i \"RomImage.z64\"  #\n");
    printf("     ##########################################\n");
    printf("\n");
}

void do_error(const char *s, ...)
{
    va_list args;
    va_start(args, s);
    vfprintf(stderr, s, args);
    fprintf(stderr, "\n");
    va_end(args);
    exit(0);
}

byte header[65];
int region = -1;

int read_header(FILE *f, int offset)
{
    byte a;

    fseek(f, offset, SEEK_SET);

    for (int i = 0; i < 64; i++)
    {
        fread(&a, sizeof(byte), 1, f);
        header[i] = a;
    }

    if (header[0x3E] == 0x45) // Region USA
    {
        if (header[0x10] == 0xA8) // Original release
        {
            region = 0;
            printf("Region USA\n");
        }
        else if (header[0x10] == 0x42) // Revision 1 release
        {
            region = 3;
            printf("Region USA, Revision 1\n");
        }
        else
        {
            do_error("Error Region %d\n", region);
            exit(0);
        }
    }
    else if (header[0x3E] == 0x50) // Region EUR
    {
        region = 1;
        printf("Region EUR\n");
    }
    else if (header[0x3E] == 0x4A) // Region JAP
    {
        region = 2;
        printf("Region JAP\n");
    }
    else
    {
        do_error("Error Region %d\n", region);
        exit(0);
    }

    return 0;
}

byte name1[6] = {'D', 'o', 'o', 'm', '6', '4'}; // Usa, Eur
byte name2[6] = {'D', 'O', 'O', 'M', '6', '4'}; // Jap

int check_name(void)
{
    for (int i = 0; i < 6; i++)
    {
        if (region == 2) // Jap
        {
            if (header[0x20 + i] != name2[i])
            {
                return 0;
            }
        }
        else
        {
            if (header[0x20 + i] != name1[i])
            {
                return 0;
            }
        }
    }

    return 1;
}

int WAD_offset[4]   = {0x63D10 , 0x63F60 , 0x64580 , 0x63DC0};
int WAD_size[4]     = {0x5D18B0, 0x5D6CDC, 0x5D8478, 0x5D301C};

int WMD_offset[4]   = {0x6355C0, 0x63AC40, 0x63CA00, 0x636DE0};
int WMD_size[4]     = {0xB9E0  , 0xB9E0  , 0xB9E0  , 0xB9E0};

int WSD_offset[4]   = {0x640FA0, 0x646620, 0x6483E0, 0x6427C0};
int WSD_size[4]     = {0x142F8 , 0x142F8 , 0x142F8 , 0x142F8};

int WDD_offset[4]   = {0x6552A0, 0x65A920, 0x65C6E0, 0x656AC0};
int WDD_size[4]     = {0x1716C4, 0x1716C4, 0x1716C4, 0x1716C4};

int main(int argc, char *argv[])
{
    byte a;

    // Parse arguments
    char *romname;
    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            if (!strcmp(argv[i], "-i"))
            {
                i++;
                romname = argv[i];
            }
        }
    }

    show_info();
    if (argc == 1)
    {
        exit(0);
    }

    FILE *in, *out;
    in = fopen(romname, "rb");

    if (!in)
    {
        do_error("Cannot open %s\n", romname);
        exit(0);
    }

    read_header(in, 0);

    if (!check_name())
    {
        do_error("Not a Doom 64 Rom\n");
        exit(0);
    }

    // Extract the file DOOM64.WAD
    printf("Extracting DOOM64.WAD ");
    fseek(in, WAD_offset[region], SEEK_SET);
    out = fopen("DOOM64.WAD", "wb");
    for (int i = 0; i < WAD_size[region]; i++)
    {
        fread(&a, sizeof(byte), 1, in);
        fwrite(&a, sizeof(byte), 1, out);
    }
    fclose(out);
    printf("Ok\n");

    // Extract the file DOOM64.WMD
    printf("Extracting DOOM64.WMD ");
    fseek(in, WMD_offset[region], SEEK_SET);
    out = fopen("DOOM64.WMD", "wb");
    for (int i = 0; i < WMD_size[region]; i++)
    {
        fread(&a, sizeof(byte), 1, in);
        fwrite(&a, sizeof(byte), 1, out);
    }
    fclose(out);
    printf("Ok\n");

    // Extract the file DOOM64.WSD
    printf("Extracting DOOM64.WSD ");
    fseek(in, WSD_offset[region], SEEK_SET);
    out = fopen("DOOM64.WSD", "wb");
    for (int i = 0; i < WSD_size[region]; i++)
    {
        fread(&a, sizeof(byte), 1, in);
        fwrite(&a, sizeof(byte), 1, out);
    }
    fclose(out);
    printf("Ok\n");

    // Extract the file DOOM64.WDD
    printf("Extracting DOOM64.WDD ");
    fseek(in, WDD_offset[region], SEEK_SET);
    out = fopen("DOOM64.WDD", "wb");
    for (int i = 0; i < WDD_size[region]; i++)
    {
        fread(&a, sizeof(byte), 1, in);
        fwrite(&a, sizeof(byte), 1, out);
    }
    fclose(out);
    printf("Ok\n");

    fclose(in);

    printf("\nCompleted\n");
    return 0;
}
