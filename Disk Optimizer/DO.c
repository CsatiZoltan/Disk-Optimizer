/* 1D offline bin-packing algorithm

Further description goes here ...
Bibliography:
[1] Johnson,D.S., Demers,A., Ullman,J.D., Garey,M.R. and Graham,R.L.:
Worst-Case Performance Bounds for Simple One-Dimensional Packing Algorithms,
SIAM J. Comput., 3(4), 299–325., 1974

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>


/* Function prototypes */

struct settings processInputs(struct settings defaultSettings, int argc, char *argv[]);
void printHelp();
int nlines(char *inputFileName);
void loadData(struct item *inputItems, int N, char *inputFileName);
void sortDescend(struct item inputStructure[], int nItem);
void freeSpace(struct bin binArray[], double frSpc[], double binSize, int nBin);
void processNegativeValues(double frSpc[], struct item, double output[], double binSize, int nBin);
int  minimum(double output[], int nBin);
int saveData(struct bin *binArray, char *outputFileName, int nBins, char version[]);
void printBinStructure(struct bin *binArray, int nBins, int nItems);
void printItemStructure(struct item *itemArray, int nItems);

/* Fundamental structure declarations */
struct bin{
	/* One bin structure corresponds to one DVD */
	struct item *itemArray; /* every bin contains several items */
	double binSize;         /* (constant) bin capacity */
	double usedSpace;       /* how much space is used up */
	int nextIndex;			/* index of the array of items we can put the next item */
};

struct item{
	/* One item structure corresponds to one file on a DVD */
	char *tag;      /* name of the item */
	double itemSize; /* size of the item */
};

struct settings{
	/* Input arguments settings with default values */
	char input[100];
	char *output;
	int append;
	char *version;
	int errorOccured;
};
/* struct settings defaultSettings = { "C:\\Users\\Zozo\\Downloads\\input.txt", "C:\\out.txt", 0, "1.0", 0 }; // TODO update it when the version changes */
static char version[] = "1.0"; /* update it when the version changes */


int main(int argc, char *argv[])
{
	/* =========== Preprocessing =========== */

	/* Process input data */
	char inputFileName[200];   /* input location */
	char outputFileName[200];  /* output location */
	int append = 0; /*  */
	int outputGiven = 0; /* user gave the output file location */
	int i; /* iteration index */
	for (i = 1; i < argc; i++)  /* Skip argv[0] (program name). */
	{
		if (strcmp(argv[i], "-a") == 0) /* append the output file instead of overwriting */
			append = 1;
		else if (strcmp(argv[i], "-h") == 0)
			printHelp();
		else if (strcmp(argv[i], "-v") == 0) /* program version */
			printf("version: %s\n", version);
		else if (strcmp(argv[i], "-o") == 0) /* requested output location */
		{
			if (argc > i + 2) /* at least two arguments (the output and the input location) follows it */
			{
				i++;
				strcpy(outputFileName, argv[i]);
				outputGiven = 1;
			}
			else if (argc > i + 1) /* only the output location follows it */
			{
				printf("The input file location is missing.\n");
				printHelp();
				return -1;
			}
			else /* both the output and the input are missing */
			{
				printf("The output and input file locations are missing.\n");
				printHelp();
				return -2;
			}
		}
		else if (i == argc - 1) /* last parameter (input file location) */
			strcpy(inputFileName, argv[i]);
		else
		{
			printf("Unknown option\n");
			printHelp();
			return -3;
		}
	}

	/* If the user has not given the output file, save it to the same directory as the input file */
	if (!outputGiven)
	{
		strcpy(outputFileName, inputFileName); /* start with the input location */
		char *start = outputFileName; /* mark the beginning of the output file name */
		int inputLength = strlen(inputFileName);
		char *end = outputFileName + inputLength; /* mark the end of the output file name */
		while (*end-- != '\\' & end > start) /* find the directory of input (i.e. until the last \) */
		{
			printf("%c", *end);
		}
		if (end == start) /* if \\ is not found, the input is wrong */
		{
			printf("Input file must contain directory or drive\n");
			return -4;
		}
		strcpy(end += 2, "output.txt"); /* append with the default "output.txt" file name */
	}

	int N; /* number of elements of the structure array */
	N = nlines(inputFileName);
	if (N == -5)
		return -5;
	struct item *givenStructure = calloc(N, sizeof(struct item));
	loadData(givenStructure, N, inputFileName);

	/* Sort input array in descending order */
	sortDescend(givenStructure, N);

	/* Exclude elements that do not fit into a bin */
	double binSize = 4.5; /* (normally from the user) */
	int startIndex = 0;
	for (i = 0; givenStructure[i].itemSize > binSize; i++)
		startIndex++;
	int nItems; /* number of items */
	nItems = N - startIndex; /* number of remaining items */

	/* Determine the required number of bins */
	double totalSum = 0;
	for (i = startIndex; i < N; i++)
		totalSum += givenStructure[i].itemSize;
	double optimum = ceil(totalSum / binSize); /* if we could slice the items */
	int nBins = (int)ceil(11/9.0 * optimum); /* number of bins (see literature [1]) */

	/* Dynamically allocate the nested structures (use calloc so that zero-filling is done) */
	struct bin *binArray = calloc(nBins, sizeof(struct bin)); /* create the bins */
	for (int j = 0; j < nBins; j++) /* dynamically allocate members of the array of bin structures */
	{
		binArray[j].itemArray = calloc(nItems, sizeof(struct item)); /* create the structures holding the items*/
	}


	/* =========== Offline Best Fit Algorithm =========== */

	double *frSpc = calloc(nBins, sizeof(double)); /* free space in binArray (i.e. binSize - usedSpace) */
	double *out = calloc(nBins, sizeof(double)); /* similar as *frSpc, but for the processed values */
	int index;

	/* Main loop */
	for (i = startIndex; i<N; i++) /* loop through the items */
	{
		freeSpace(binArray, frSpc, binSize, nBins);
		processNegativeValues(frSpc, givenStructure[i], out, binSize, nBins);
		index = minimum(out, nBins);
		int whereToPut = binArray[index].nextIndex; /* where to put the current item */
		binArray[index].itemArray[whereToPut] = givenStructure[i];
		binArray[index].nextIndex++;
		binArray[index].usedSpace += givenStructure[i].itemSize;
	}


	/* ========== Create output ========== */

	saveData(binArray, outputFileName, nBins, version);
	getchar();

	/* Release the dynamically allocated space */
	free(frSpc);
	free(out);
	free(givenStructure);
	for (int j = 0; j < nBins; j++)
		free(binArray[j].itemArray);
	free(binArray);
} /* end of main */


/* Function definitions */

struct settings processInputs(struct settings defaultSettings, int argc, char *argv[])
{
	/* Copy the input to the output */
	struct settings processed = defaultSettings;
	processed.output = calloc(100, sizeof(char));
	/* Modify the output structure according to the user inputs */
	/* char output[100] = "D:\\output.txt"; */
	int c; /* first character of the current input argument string */
	while (--argc > 0 & **++argv == '-')
	{
		while (c = *(++(*argv))){
			if (c == 'o') /* requested output location */
			{
				char temp[100];
				strcpy(temp, *++argv);
				defaultSettings.output = temp;
				argc -= 2;
				++argv;
				printf("");
			}
			else if (c == 'a') /* append the output file instead of overwriting */
				defaultSettings.append = 1;
			else if (c == 'v')
				printf("version: %s\n", defaultSettings.version); /* program version */
			else if (c == 'h')
				printHelp();
			else
			{
				printf("Unknown option.\n");
				printHelp();
			}
		}
	}
	printf("output: %s\n", defaultSettings.output);
	if (argc != 1){
		printHelp();
		processed.errorOccured = 1;
	}
	return processed;
}

void printHelp()
{
	printf("\nUsage:   OfflineBestFit [OPTIONS] input\n\n");
	printf("\t OPTIONS\n");
	printf("\t -a: append the output file instead of overwriting it\n");
	printf("\t -h: print this help message\n");
	printf("\t -o: OUTPUT: save results to the OUTPUT location\n");
	printf("\t -v: program version\n\n");
}


int nlines(char *inputFileName)
/* Number of lines in the file */
{
	/* Handle the input file */
	FILE *inputStream;
	inputStream = fopen(inputFileName, "r");
	if (inputStream == NULL){
		printf("Could not open the file %s for reading.\n", inputFileName);
		return -5;
	}
	/* Count the lines */
	int N = 0; /* number of lines */
	int nChars = -1; /* number of characters (exclude EOF character) */
	char currentChar = ' '; /* last character read from the file stream */
	char lastChar; /* number of characters */
	while (!feof(inputStream)){
		lastChar = currentChar;
		currentChar = fgetc(inputStream);
		nChars++;
		if (currentChar == '\n')
			N++;
	}
	if (lastChar != '\n' /* the file does not end with '\n' ... */
		&& nChars != 0)  /* ... and is not empty */
		N++;
	fclose(inputStream);
	return N;
}


void loadData(struct item *inputItems, int N, char *inputFileName)
/* Read the file content to the input structure */
{
	/* Handle the input file */
	FILE *inputStream;
	inputStream = fopen(inputFileName, "r");
	/*Determine the number of items by counting the rows of the input file */

	/* Process data row-by-row */
	const int filmNameLength = 255; /* maximum length of the name of a film */
	for (int k = 0; k < N; k++){
		inputItems[k].tag = calloc(filmNameLength, sizeof(char));
	}
	rewind(inputStream); /* go back to the start of the file */
	char row[300]; /* contains one row of the file */
	const int rowSize = 300;/* maximum number of characters in a row */
	char title[255]; double size; /* temporary buffers */
	int i = 0;
	while (fgets(row, rowSize, inputStream)){
		sscanf(row, "%s %lf", title, &size);
		strcpy(inputItems[i].tag, title);
		inputItems[i].itemSize = size;
		i++;
	}
	fclose(inputStream);
}


void sortDescend(struct item inputStructure[], int nItems)
/* Sort input array in descending order (naive sort) */
{
	int i, j;
	struct item temp;
	for (i = 0; i<nItems; i++)
		for (j = i; j<nItems; j++)
			if (inputStructure[i].itemSize < inputStructure[j].itemSize)
			{
				temp = inputStructure[i];
				inputStructure[i] = inputStructure[j];
				inputStructure[j] = temp;
			}
}


void freeSpace(struct bin binArray[], double frSpc[], double binSize, int nBins)
/* Calculate the free space in the bins at a specific time */
{
	int i;
	for (i = 0; i<nBins; i++)
		frSpc[i] = binSize - binArray[i].usedSpace;
}


void processNegativeValues(double frSpc[], struct item currentItem, double output[], double binSize, int nBins)
/* To make the non-negative minimum finding easy, make the negative entries large enough */
{
	int i;
	for (i = 0; i<nBins; i++)
	{
		output[i] = frSpc[i] - currentItem.itemSize;
		if (output[i] < 0)
			output[i] = binSize + 1;
	}
}


int minimum(double output[], int nBins)
/* Find the index of the minimum value we get when the specific item is placed in the right bin */
{
	int i, index = 0;
	double Old = output[0];
	double New;
	for (i = 1; i<nBins; i++)
	{
		New = output[i];
		if (New < Old)
		{
			Old = New;
			index = i;
		}
	}
	return index;
}


int saveData(struct bin *binArray, char *outputFileName, int nBins, char version[])
/* Write the processed structure into file */
{
	/* Handle the output file */
	FILE *outputStream; /* TODO create a file with a number after it when there is (are) file(s) already in the directory */
	outputStream = fopen(outputFileName, "w+");
	if (outputStream == NULL){
		printf("Could not open the file for writing.");
		return -6;
	}
	/* Print header text (time, author, program version, etc.) */
	fprintf(outputStream, "========== Created with OfflineBestFit ==========\n");
	fprintf(outputStream, "=                                               =\n");
	fprintf(outputStream, "=   Date:                                       =\n"); /* TODO: modify using time.h to measure the current time */
	fprintf(outputStream, "=   Author: Zoltan Csati                        =\n");
	fprintf(outputStream, "=   Version: %s                                =\n", version);
	fprintf(outputStream, "=                                               =\n");
	fprintf(outputStream, "=================================================\n\n");
	/* Write formatted output to file */
	for (int j = 0; j < nBins && binArray[j].usedSpace > 0; j++)
	{
		fprintf(outputStream, "\nDisk %d\n\n", j + 1);
		for (int i = 0; i < binArray[j].nextIndex; i++)
		{
			fprintf(outputStream, "   Tag: %s\n   Size: %lf\n",
				binArray[j].itemArray[i].tag, binArray[j].itemArray[i].itemSize);
		}
	}
	fclose(outputStream);
	printf("Results were successfully written to %s\n", outputFileName);
	return 1;
}


void printBinStructure(struct bin *binArray, int nBins, int nItems){
	/* Print the members of the bin structure array for debugging purposes */
	for (int j = 0; j<nBins; j++)
	{
		printf("\nbinArray[%d]\n\n", j);
		for (int i = 0; i<nItems; i++)
		{
			printf("itemArray[%d]\n   Size: %f\n   Tag: %s\n",
				i, binArray[j].itemArray[i].itemSize, binArray[j].itemArray[i].tag);
		}
	}
}


void printItemStructure(struct item *itemArray, int nItems){
	/* Print the members of the item structure array for debugging purposes */
	for (int j = 0; j<nItems; j++)
	{
		printf("\nitemArray[%d]\n   Tag: %s,   Size: %f\n",
			j, itemArray[j].tag, itemArray[j].itemSize);
	}
}