// ===================================================================================================================
// DOOM 64 EXTRACTOR
// ===================================================================================================================
// how to compile: g++ -o dm64ex dm64ex.cpp -static
// ===================================================================================================================

#include <cstring>
#include <stdio.h>
#include <vector>
#include <cstdarg>
#include <string>

typedef unsigned char byte;

void ShowInfo(void)
{
	printf("\n     ################");
    	printf("(ERICK194)");
    	printf("################\n");
	printf("     #            DOOM 64 EXTRACTOR           #\n");
	printf("     #     CREATED BY ERICK VASQUEZ GARCIA    #\n");
	printf("     #              Doom 64.z64               #\n");
	printf("     #         (USA/EUP/JAP) VERSION          #\n");
	printf("     #                                        #\n");
	printf("     # USE:                                   #\n");
	printf("     # Doom64Extractor.exe -i \"RomImage.z64\"  #\n");
	printf("     ##########################################\n");
	printf("\n");
}

void Error(const char *s,...)
{
     va_list args;
     va_start(args,s);
     vfprintf(stderr,s,args);
     fprintf(stderr,"\n");
     va_end(args);
     exit(0);
}

byte header[65];
int region = -1;

int ReadHeader(FILE *f, int offset)
{
	byte a;

	fseek(f, offset, SEEK_SET);

	for(int i = 0; i < 64; i++)
	{
		fread (&a, sizeof(byte), 1, f);
		header[i] = a;
	}

	if(header[0x3E] == 0x45) // Region USA
	{
		region = 0;
		printf("Region USA\n");
	}
	else if(header[0x3E] == 0x50) // Region EUP
	{
		region = 1;
		printf("Region EUP\n");
	}
	else if(header[0x3E] == 0x4A) // Region JAP
	{
		region = 2;
		printf("Region JAP\n");
	}
	else
	{
		Error("Error Region %d\n", region);
		exit(0);
	}

	return 0;
}

byte name1[6] = {'D','o','o','m','6','4'};// Usa, Eup
byte name2[6] = {'D','O','O','M','6','4'};// Jap

int CheckName(void)
{
	int i; 
	int rtn;

	rtn = 1;
	for(i = 0; i < 6; i++)
	{
		if(region == 2) // Jap
		{
			if(header[0x20 + i] != name2[i])
			{
				rtn = 0;
				break;
			}
		}
		else 
		{
			if(header[0x20 + i] != name1[i])
			{
				rtn = 0;
				break;
			}
		}
	}

    return rtn;
}

int WAD_offset[3]	= {0x63D10 , 0x63F60 , 0x64580};
int WAD_size[3]		= {0x5D18B0, 0x5D6CDC, 0x5D8478};

int WMD_offset[3]	= {0x6355C0, 0x63AC40, 0x63CA00};
int WMD_size[3]		= {0xB9E0  , 0xB9E0  , 0xB9E0};

int WSD_offset[3]	= {0x640FA0, 0x646620, 0x6483E0};
int WSD_size[3]		= {0x142F8 , 0x142F8 , 0x142F8};

int WDD_offset[3]	= {0x6552A0, 0x65A920, 0x65C6E0};
int WDD_size[3]		= {0x1716C4, 0x1716C4, 0x1716C4};

int main(int argc, char *argv[])
{
	int i;
	byte a;

	// Parse arguments
	char *romname;
	for ( int i=1; i<argc; i++)
	{
		if ( argv[i][0] == '-' )
		{
			if(!strcmp(argv[i],"-i"))
			{
				i++;
				romname = argv[i];
			}
		}
	}

	ShowInfo();
	if ( argc == 1 )
	{
		exit(0);
	}

	FILE *in, *out;
    in = fopen(romname,"rb");
	//in = fopen("Doom 64 (Europe).z64","rb");
	//in = fopen("Doom 64 (Japan).z64","rb");

    if(!in)
    {
		Error("Cannot open %s\n", romname);
		exit(0);
    }

	ReadHeader(in, 0);
	
	//printf("CheckName :: %d\n", CheckName());
	if(CheckName() == 0)
	{
		Error("Not a Doom 64 Rom\n");
		exit(0);
	}

	// Extract the file DOOM64.WAD
	printf("Extracting DOOM64.WAD ");
	fseek(in, WAD_offset[region], SEEK_SET);
	out = fopen("DOOM64.WAD","wb");
	for(i = 0; i < WAD_size[region]; i++)
	{
		fread (&a, sizeof(byte), 1, in);
		fwrite (&a, sizeof(byte), 1, out);
	}
	fclose(out);
	printf("Ok\n");


	// Extract the file DOOM64.WMD
	printf("Extracting DOOM64.WMD ");
	fseek(in, WMD_offset[region], SEEK_SET);
	out = fopen("DOOM64.WMD","wb");
	for(i = 0; i < WMD_size[region]; i++)
	{
		fread (&a, sizeof(byte), 1, in);
		fwrite (&a, sizeof(byte), 1, out);
	}
	fclose(out);
	printf("Ok\n");

	// Extract the file DOOM64.WSD
	printf("Extracting DOOM64.WSD ");
	fseek(in, WSD_offset[region], SEEK_SET);
	out = fopen("DOOM64.WSD","wb");
	for(i = 0; i < WSD_size[region]; i++)
	{
		fread (&a, sizeof(byte), 1, in);
		fwrite (&a, sizeof(byte), 1, out);
	}
	fclose(out);
	printf("Ok\n");

	// Extract the file DOOM64.WDD
	printf("Extracting DOOM64.WDD ");
	fseek(in, WDD_offset[region], SEEK_SET);
	out = fopen("DOOM64.WDD","wb");
	for(i = 0; i < WDD_size[region]; i++)
	{
		fread (&a, sizeof(byte), 1, in);
		fwrite (&a, sizeof(byte), 1, out);
	}
	fclose(out);
	printf("Ok\n");

	fclose(in);

	printf("\nCompleted\n");
	return 0;
}
