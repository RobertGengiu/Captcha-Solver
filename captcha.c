#include "bmp_header.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define null 0


//////////////////// Structura care va retine un pixel ////////////////////////
typedef struct 
{
	unsigned char r;
	unsigned char g;
	unsigned char b;
	
}Pixel;
///////////////////////////////////////////////////////////////////////////////


/////////////////// Structura care va retine o cifra //////////////////////////
typedef struct 
{
	int value;					// Valoarea (0-9)
	int startX;					// Coordonata pe Ox
	int startY;					// Coordonata pe Oy
	
}Number;
///////////////////////////////////////////////////////////////////////////////


///////////////// Structura ce retine o imagine ///////////////////////////////
typedef struct
{
	Pixel mat[100][100];		// Matricea cu pixeli	
	int width;					// Latimea imaginii
	int height;					// Inaltimea imaginii

}Image;
///////////////////////////////////////////////////////////////////////////////


////////////////// Functie ce aloca o imagine /////////////////////////////////
Image* imageAlloc(signed int width, signed int height)
{	
	Image* image = (Image*)malloc(sizeof(Image));
	if(!image)
	{
		printf("Eroare de alocare\n");
		return null;
	}

	image->width = width;
	image->height = height;
	
	return image;
}
///////////////////////////////////////////////////////////////////////////////


/////////// Functie ce verifica daca doi pixeli au aceeasi culoare ////////////
int isNotColor(Pixel ref, Pixel pixel)
{
	if(ref.r != pixel.r || ref.g != pixel.g || ref.b != pixel.b)
		return 1;
	return 0;
}
///////////////////////////////////////////////////////////////////////////////


//////////////////// Functie ce verifica daca pixelul este alb ////////////////
int isWhite(Pixel pixel)
{
	if(pixel.r == 255 && pixel.g == 255 && pixel.b == 255)
		return 1;
	else
		return 0;
}
///////////////////////////////////////////////////////////////////////////////


////////////////// Functie ce creaza copia unei imagini ///////////////////////
Image* copyImage(Image *source)
{
	Image *destiantion = imageAlloc(source->width, source->height);
	int i, j;

	if(!destiantion)
	{
		printf("Eroare de alocare\n");
		return null;
	}

	for(i = 0; i < destiantion->height; i++)
	{
		for (j = 0; j < destiantion->width; j++)
		{
			destiantion->mat[i][j].r = source->mat[i][j].r;
			destiantion->mat[i][j].g = source->mat[i][j].g;
			destiantion->mat[i][j].b = source->mat[i][j].b;
		}
	}

	return destiantion;
}
///////////////////////////////////////////////////////////////////////////////


///////////////// Functie ce citeste o imagine dintr-un fisier ////////////////
void readImage(Image *image, FILE *in)
{
	int i, j;
	int padSize = (4 - 3 * image->width % 4) % 4;

	for(i  = image->height - 1 ; i >= 0; i--)
	{
		for(j = 0; j < image->width; j++)
		{
			fread(&image->mat[i][j], 3, 1, in);
		}

		fseek(in, padSize, SEEK_CUR);
	}
}
///////////////////////////////////////////////////////////////////////////////


//////////////// Functie ce scrie o imagine intr-un fisier ////////////////////
void writeImage(Image *image, struct bmp_fileheader fh, 
	struct bmp_infoheader ih, FILE *out)
{
	int i, j;
	int padSize = (4 - 3 * image->width % 4) % 4;
	unsigned char bmppad[3] = {0,0,0};

	fwrite(&fh, 14, 1, out);
	fwrite(&ih, 40, 1, out);
	fseek(out, fh.imageDataOffset, SEEK_SET);
	
	
	for(i = image->height - 1 ; i >= 0; i--)
	{
		for(j = 0; j < image->width; j++)
		{
			fwrite(&image->mat[i][j], 3, 1, out);
		}

		fwrite(bmppad, padSize, 1, out);
	}
}
///////////////////////////////////////////////////////////////////////////////


/////////// Functie ce schimba culoarea cifrelor in culoarea dorita ///////////
void changeColors(Image *image, Pixel reference)
{
	int i, j;

	for (i = 0; i < image->height; i++)
	{
		for(j = 0; j < image->width; j++)
		{
			if (!isWhite(image->mat[i][j]))
			{
				image->mat[i][j].r = reference.r;
				image->mat[i][j].g = reference.g;
				image->mat[i][j].b = reference.b;
			}		
		}
	}
}
///////////////////////////////////////////////////////////////////////////////


/////////////////////// Functie ce identifica o cifra /////////////////////////
int resolve(int points[5][5], int numberOfPoints)
{
	if(numberOfPoints == 16 && !points[2][2])
		return 0;

	if(numberOfPoints == 5 && points[0][3] && points[2][3] && points[4][3] &&
	 !points[0][4] )
		return 1;

	if(numberOfPoints == 17)
	{
		if(points[1][4] && points[3][0])
			return 2;

		if(points[1][4] && points[3][4] )
			return 3;

		else
			return 5;
	}

	if(numberOfPoints == 11 && points[1][0] && points[0][4] && points[1][4] && 
		points[2][3] && points[3][4])
		return 4;

	if(numberOfPoints == 9 && points[0][1] && points[0][2] && points[2][4] &&
		points[3][4] && points[4][4])
		return 7;

	if(numberOfPoints == 19)
		return 8;

	if(numberOfPoints == 18)
	{
		if(points[1][4] == 1)
			return 9;
		else
			return 6;
	}

	return -1;
}
///////////////////////////////////////////////////////////////////////////////


/////////////////// Functie ce preia o submatrice de 5 x 5 ////////////////////
int takeInput(Image *image, int x, int y)
{
	int numberOfPoints = 0;
	int points[5][5] = {{}};		// Matrice goala
	int i, j;

	for (i = x; i < x + 5; i++)
	{
		for (j = y; j < y + 5; j++)
		{	
			if (!isWhite(image->mat[i][j]) )
			{
				points[i-x][j-y] = 1;
				numberOfPoints++;
			}	
		}
	}
	if(numberOfPoints >= 5)
	{
		return resolve(points, numberOfPoints);
	}	

	return -1;
}
///////////////////////////////////////////////////////////////////////////////


/////////////////// Functia ce cauta cifre ////////////////////////////////////
Number* searchPattern(Image *image, int *n)
{
	int i, j;
	int counter = 0, result = 0;
	Number *listOfNumbers = (Number*)malloc(sizeof(Number) * 30);

	if(!listOfNumbers)
	{
		printf("Eroare de alocare\n");
		return null;
	}
	
	for(j = 0; j < image->width-4; j++)
	{
		for (i = 0; i < image->height-4; i++)
		{
			result = takeInput(image, i, j);
			if(result != -1)
			{
				listOfNumbers[counter].value = result;
				listOfNumbers[counter].startY  = i;
				
				if(result != 1)
				{
					
					listOfNumbers[counter].startX = j;
				}
				else
				{
					listOfNumbers[counter].startX = j - 1;
				}

				counter++;				
			}
		}
	}

	*n = counter;
	return listOfNumbers;
}
///////////////////////////////////////////////////////////////////////////////


///////////////// Muta o cifra cu o pozitie ///////////////////////////////////
void switchNumbers(int x_1, int y_1, int x_2, int y_2, Image *image)
{
	int i, j;

	for(i = 0; i < 5; i++)
    {
		for(j = 0; j < 5; j++)
		{
			image->mat[x_1 + i][y_1 + j].r = image->mat[x_2 + i][y_2 + j].r;
			image->mat[x_1 + i][y_1 + j].g = image->mat[x_2 + i][y_2 + j].g;
			image->mat[x_1 + i][y_1 + j].b = image->mat[x_2 + i][y_2 + j].b;
		}
	}

}
///////////////////////////////////////////////////////////////////////////////


////////////////////// Elimina o cifra ////////////////////////////////////////
void clear(int x_1, int y_1, Image *image)
{
	int i, j;
	for(i = y_1 - 1; i < y_1 + 5; i++)
    {
		for(j = x_1 - 1; j < x_1 + 5; j++)
		{
			image->mat[j][i].r = 255;
			image->mat[j][i].g = 255;
			image->mat[j][i].b = 255;
		}
	}

}
///////////////////////////////////////////////////////////////////////////////


//////////////////// Functie ce sterge cifre dintr-o lista ////////////////////
void removeN(Image *image, Number *lstOfNum, int n, int lstForRem[10], int m)
{
	int i, j, k;
	for(i = 0 ; i < m; i++)
	{
		etichet:
		for(j = 0; j < n; j++)
		{	
			if(lstForRem[i] == lstOfNum[j].value)
			{
				for(k = j ; k < n - 1; k++)
				{
					switchNumbers(lstOfNum[k].startY, lstOfNum[k].startX, 
						lstOfNum[k+1].startY, lstOfNum[k+1].startX, image);
						lstOfNum[k].value = lstOfNum[k+1].value;
				}

				clear(lstOfNum[n-1].startY, lstOfNum[n-1].startX, image);
				n--;
				goto etichet;
			}		
		} 	
	}
}
///////////////////////////////////////////////////////////////////////////////


////////////////////////////// Functia main ///////////////////////////////////
int main()
{	
	////////////// Alocam memorie pentru numele fisierelor ////////////////////
	char *fileIN_task1 = (char*)malloc(sizeof(char)*100);
	char *fileOUT_task1 = (char*)malloc(sizeof(char)*100);
	char *fileOUT_task2 = (char*)malloc(sizeof(char)*100);
	char *fileOUT_task3 = (char*)malloc(sizeof(char)*100);
	char *fileIN_B = (char*)malloc(sizeof(char)*100);
	///////////////////////////////////////////////////////////////////////////


	Pixel main_color;					// Culoarea dorita pentru cifre
	int listForRemoving[10];			// Lista cu cifrele de sters 

	FILE *fp = fopen("input.txt", "rt");

	if(!fp)
	{
		printf("Fisierul de input nu se poate deschide\n");
		return -1;
	}

	fscanf(fp, "%s", fileIN_task1);
	fscanf(fp, "%hhu %hhu %hhu", &main_color.r, &main_color.g, &main_color.b);

	int m, i = 0; 

	while(fscanf(fp, "%d ", &m) == 1)
		listForRemoving[i++] = m;
	

	fscanf(fp, "%s", fileIN_B);

	fclose(fp);

	struct bmp_fileheader fileheader;
	struct bmp_infoheader infoheader;

	FILE *file_1 = fopen(fileIN_task1, "rb");

	if(!file_1)
	{
		printf("Fisierul bmp nu se poate deschide sau lipseste\n");
		return -1;
	}

	fread(&fileheader, 14, 1, file_1);
	fread(&infoheader, 40, 1, file_1);


	/////////////////////////////////// Taskul 1 //////////////////////////////
	Image *image = imageAlloc(infoheader.width, infoheader.height);
	fseek(file_1, fileheader.imageDataOffset, SEEK_SET);
	readImage(image, file_1);
	Image *copy = copyImage(image);

	fclose(file_1);
	
	strncpy(fileOUT_task1, fileIN_task1, strlen(fileIN_task1)-4);
	strcat(fileOUT_task1, "_task1.bmp\0");

	FILE *file_2 = fopen(fileOUT_task1, "wb");

	changeColors(image, main_color);
	writeImage(image, fileheader, infoheader, file_2);
	
	fclose(file_2);
	///////////////////////////////////////////////////////////////////////////


	///////////////////////////// Taskul 2 ////////////////////////////////////
	int n;
	Number *numbers = searchPattern(image, &n);

	strncpy(fileOUT_task2, fileIN_task1, strlen(fileIN_task1)-4);
	strcat(fileOUT_task2, "_task2.txt\0");

	FILE *file_3 = fopen(fileOUT_task2, "wt");

	int a;
	
	for( a = 0; a < n; a++)
	{
		fprintf(file_3, "%d", numbers[a].value);
	}

	fclose(file_3);
	///////////////////////////////////////////////////////////////////////////


	///////////////////////////// Taskul 3 ////////////////////////////////////
	strncpy(fileOUT_task3, fileIN_task1, strlen(fileIN_task1)-4);
	strcat(fileOUT_task3, "_task3.bmp\0");

	FILE *file_4 = fopen(fileOUT_task3, "wb");

	removeN(copy, numbers, n, listForRemoving, i);
	writeImage(copy, fileheader, infoheader, file_4);

	fclose(file_4);
	///////////////////////////////////////////////////////////////////////////


	free(numbers);
	free(fileIN_task1);
	free(fileOUT_task1);
	free(fileOUT_task2);
	free(fileOUT_task3);
	free(fileIN_B);

	return(0);

}
