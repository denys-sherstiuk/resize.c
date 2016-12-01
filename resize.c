#include <stdio.h>
#include <stdlib.h>

#include "bmp.h"

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        fprintf(stderr, "Usage: ./resize coefficient infile outfile \n");
        return 1;
    }

    char *infile = argv[2];
    char *outfile = argv[3];
    
    int coefficient = atoi(argv[1]);
    if (coefficient < 1 | coefficient > 100)
    {
        fprintf(stderr, "Usage: ./resize 1 <= coefficient < 100 infile outfile \n");
        return 1;
    }

    FILE* inptr = fopen(infile, "r");
    if (inptr == NULL)
    {
        fprintf(stderr, "Could not open %s. \n", infile);
        return 2;
    }

    FILE* outptr = fopen(outfile, "w");
    if (outptr == NULL)
    {
        fclose(inptr);
        fprintf(stderr, "Could not create %s. \n", outfile);
        return 3;
    }

    BITMAPFILEHEADER bf;
    fread(&bf, sizeof(BITMAPFILEHEADER), 1, inptr);

    BITMAPINFOHEADER bi;
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, inptr);

    if (bf.bfType != 0x4d42 || bf.bfOffBits != 54 || bi.biSize != 40 || 
        bi.biBitCount != 24 || bi.biCompression != 0)
    {
        fclose(outptr);
        fclose(inptr);
        fprintf(stderr, "Unsupported file format. \n");
        return 4;
    }

    int padding =  (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;
    int padding_out = (4 - (bi.biWidth*coefficient* sizeof(RGBTRIPLE)) % 4) % 4;
    
    bf.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    
    fseek(outptr, bf.bfSize, SEEK_SET);

    for (int i = 0, biHeight = abs(bi.biHeight); i < biHeight; i++)
    {
        for (int z = 0; z < coefficient; z++)
        {
            if (z != 0)
            {
                fseek(inptr, -bi.biWidth*sizeof(RGBTRIPLE), SEEK_CUR);
            }
            
            for (int j = 0; j < bi.biWidth; j++)
            {
                RGBTRIPLE triple;

                fread(&triple, sizeof(RGBTRIPLE), 1, inptr);

                for (int g = 0; g < coefficient; g++)
                {
                fwrite (&triple, sizeof(RGBTRIPLE), 1, outptr);
            
                bf.bfSize += sizeof(RGBTRIPLE);
                }
            }
            for (int k = 0; k < padding_out; k++)
            {
                fputc(0x00, outptr);
                bf.bfSize+=sizeof(char);
            }
        }
        
        fseek(inptr, padding, SEEK_CUR);
    }

    bi.biWidth*=coefficient;
    bi.biHeight*=coefficient;
    bi.biSizeImage = (bi.biWidth * sizeof(RGBTRIPLE) + padding_out) * abs(bi.biHeight); 
   
    fseek(outptr, 0, SEEK_SET); 
    fwrite(&bf, sizeof(BITMAPFILEHEADER), 1, outptr);
    fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, outptr);

    fclose(inptr);

    fclose(outptr);

    return 0;
}
