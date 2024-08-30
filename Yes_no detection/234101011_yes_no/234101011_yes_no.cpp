// 234101011_yes_no.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "process.h"
#include "conio.h"
#include "math.h"
#define inputfile "NO.txt"			//put filename containg data of 'Yes' or 'No' to detect
#define noisefile "Noise.txt"       //put the filename containing ambient noise data
#define framesize 100				//put framesize to be considered
#define threshold 7					//put the threshold value taken for zcr of yes


FILE* OpenFile(char fname[], char mode[])
{
	int err;
	FILE *f1 = NULL;
	err = fopen_s(&f1, fname ,mode);
	if(err != NULL)
    {
        printf("\nCannot open the file\n");
        system("pause");
        exit(1);
    }  
	return f1;
}


long double GetData(char ch)
{
	char fname[] = noisefile;						
	int x,i=0;
	long double DCShift = 0;
	long double max=0;
	FILE * f1 = OpenFile(fname,"r");				//Open and read file "Noise.txt"
	switch(ch)
	{
	case 'D': //calculate DCShift
				
			 while(!feof(f1))
				if( fscanf_s( f1, "%d", &x) == 1)
					{ 
						DCShift += x;	
						i++;
					}
			
			DCShift /= i;
	 	 	return DCShift; 

	case 'A': //calculate Ambient noise
			while(!feof(f1))
				if( fscanf_s( f1, "%d", &x) == 1)
					{ 
						if(abs(x)>max)
							max = abs(x);
					}
			return max;
	}
}

long double NormFactor()
{
	char fname[] = inputfile;						
	int err,x;
	long double max = 0,min = 0,limit = 5000, scale;
	FILE * f1 = OpenFile(fname,"r");				//Open and read the file 
	while(!feof(f1))								//Find the min and max values
			{
					if( fscanf_s( f1, "%d", &x) == 1)
					{ 
						if(x>max)
							max = x;
						if(x<min)
							min = x;
					}
			}
	scale = (max - min) / 2;
	return (limit/scale);

}
int FindZCR(int* arr, int size)
{
int  Fzcr[2000],frame=0,z=0,p, zcrsum=0;
for(int j = 0; j<size; j+=framesize)
 {
	 if(arr[j] >= 0)	p = 1; 					// if the 1st value of frame is +ve then mark p = 1 else p = 0 								
	 else			    p = 0;						

	 for(int i = j+1; i<size && i<j+framesize; i++)	// increment z by 1 iff the value of p is -ve and value of arr[i] is +ve or vice versa
			{
				 if (p == 1 && arr[i] < 0)	 { p = 0; z++; }				
			else if(p == 0 && arr[i] >= 0)	 { p = 1; z++; }
			}
	 Fzcr[frame++] = z;
	 z = 0;
 }

for(int i=frame-10;i>=frame/2;i--)				//calculate average zcr
	zcrsum+=Fzcr[i];	

return (zcrsum*2)/(frame);
}

void IdentifyYesNo(long double DCShift,long double Ambient, long double Normalize)
{
	char fname[] = inputfile;						
	FILE * f1 = OpenFile(fname,"r");
	int arr[32000],x,i=0;
	
	while(!feof(f1))								//DCShift adjustment and store normalized data in an array	
		if(fscanf_s( f1, "%d", &x) == 1)
			arr[i++] = (x - DCShift) * Normalize;   
    
	int zcr = FindZCR(arr,i);						//Calculate zcr for each frame. 
	printf("ZCR : %d\nZCR threshold : %d\n",zcr,threshold);		
	
	if(zcr>threshold) printf("\nYES");
	else printf("\nNO");
}

int _tmain(int argc, _TCHAR* argv[])
{
	long double DCShift = GetData('D');
	long double Ambient = GetData('A');
	long double Normalize = NormFactor();
	Ambient *= Normalize; //Normalize ambient energy
	printf("DCShift : %lf \nAmbient Noise : %lf \nNormalization Factor : %lf\n",DCShift,Ambient,Normalize);
	IdentifyYesNo(DCShift,Ambient,Normalize);
	getch();
	return 0;
}


