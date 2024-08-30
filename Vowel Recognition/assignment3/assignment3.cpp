// assignment3.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "process.h"
#include "conio.h"
#include "math.h"
#include "string.h"
#include "stdlib.h"
  
#define noisefile "Noise.txt"       //put the filename containing ambient noise data
#define N 320						//put framesize to be considered
#define p 12
#define pi 3.1415926535
#define threshold 1500
char inputfile[100],outputfile[100];
char var[100];
long double mat[100][12],ref[5][12];
long double dist[5] ={0};

FILE *filePointer; 
FILE *filewrite; 
 

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
long double GetDCShift()
{
	char fname[] = noisefile;						
	int x,i=0;
	long double DCShift = 0;
	long double max=0;
	FILE * f1 = OpenFile(fname,"r");				//Open and read file "Noise.txt"
	//calculate DCShift
				
			 while(!feof(f1))
				if( fscanf_s( f1, "%d", &x) == 1)
					{ 
						DCShift += x;	
						i++;
					}
			
			DCShift /= i;
	 	 	return DCShift; 
}
long double NormFactor()
{
	char fname[100];	
	strcpy(fname,inputfile);
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
void HammingWindow(long double s[])      //cap the frame with hamming window
{
	long double w[N];
	for(int m=0; m<=N-1; m++){
		w[m] = 0.54 - 0.46 * cos((double)(2*22*m)/(7*(N-1))) ;
		//printf("%Lf\n",w[m]);
	}

	for(int i = 1; i<=N; i++)
		s[i] *= w[i-1];
	
}
void Calc_Ri(long double s[],long double R[])		///returns Ri s of the given frame
{
	
	for(int k = 0; k<=p; k++)
	{
		long double temp = 0;
		for(int y = 1; y<= N-k; y++)
		{
			temp += s[y]*s[y+k];
		}
		R[k] = temp;
	}
	
}
void Calc_ai(long double s[], long double R[], long double a[])  // impl Levinsion Alg
{
	long double E[p+1],K[p+1];
	long double A[p+1][p+1];
	E[0] = R[0];
	for(int i=1; i<=p; i++)
	{
		long double temp = 0;
		for(int j = 1; j<= i-1; j++)
			temp += A[j][i-1] * R[i-j];
		K[i] = (R[i] - temp) / E[i-1];
		A[i][i] = K[i];
		for(int j = 1; j<= i-1; j++)
			A[j][i] = A[j][i-1] - K[i] * A[i-j][i-1];
		E[i] = (1 - K[i]*K[i]) * E[i-1];
	}
	for(int i = 1; i<=p; i++)
	{
		a[i] = A[i][p];
	}
	

}
void Calc_ci(long double a[], long double R[], long double c[])  //compute cepstral coeff
{
	long double energy = R[0];
	c[0] = log(energy*energy);
	for(int m = 1; m<=p; m++)
	{
		long double temp = 0;
		for(int k = 1; k<=m-1;k++)
			temp+= ((double)k/m) * c[k] * a[m-k];
		c[m] = a[m] +temp;
	}
	
}
void Calc_ci_RSW(long double a[], long double R[],long double RSW_c[])  //compute cepstral coeff with raised sine window
{
	long double energy = R[0],Cweight[p];
	RSW_c[0] = log(energy*energy);
	for(int m = 1; m<=p; m++)
	{
		long double temp = 0;
		for(int k = 1; k<=m-1;k++)
			temp+= ((double)k/m) * RSW_c[k] * a[m-k];
		RSW_c[m] = a[m] + temp;
	}
	for(int m = 0; m<p; m++)
		Cweight[m] = 1 + (p/2)*sin((double)(22*m)/(7*p));

	for(int i = 1; i<=p; i++)
		RSW_c[i] *= Cweight[i-1];
}
long double Calc_DistT(long double c[], long double cr[])  //returns tokhura distance
{
	/*printf("\n");
	for(int i = 0;i<=12; i++)
		printf("%lf ",c[i]);
	printf("\n");
	for(int i = 0;i<12; i++)
		printf("%lf ",cr[i]);*/

	long double d=0;
	long double TW[13] = {0,1.0,3.0,7.0,13.0,19.0,22.0,25.0,33.0,42.0,50.0,56.0,61.0};
	for(int i = 1; i<=p; i++)
	{
		//printf("\n %lf - %lf",c[i],cr[i-1]);
		d+= TW[i] * ((c[i] - cr[i-1])*(c[i] - cr[i-1]));
	}
	return d;
}
void fetch_frame(long double s[],long double DCShift, long double Normalize){
	int i = 1;
	long double value;
    if (filePointer == NULL) {
        printf("File could not be opened.\n");
        return;  
    }
	while (i<=320 && fscanf(filePointer, "%Lf", &value) != EOF) {

		s[i++] = (value-DCShift) * Normalize;
	}
}
void findSteadyPoint(){
	long double value;
    if (filePointer == NULL) {
        printf("File could not be opened.\n");
        return;  
    }
	while (fscanf(filePointer, "%Lf", &value) != EOF) {
			if(value>=threshold)
				break;
	}
}
void Write_Ci(long double c[])
{
	if (filewrite == NULL) {
        perror("Unable to open the output file");
        return;
	}

	for(int i = 1; i<=p; i++)
		fprintf(filewrite,"%Lf\t",c[i]);

	fprintf(filewrite,"\n");
}
void TrainModel(char str[])
{
	char out[100];
	sprintf(out, "%c",str[10]);
	strcat(out,".txt");
	strcpy(outputfile,out);
	printf("Training the model for vowel: %c\n",str[10]);
	for(int i = 1;i<=20; i++)
	{
	char num[20];
    sprintf(num, "%d", i);
	char filename[100];
    strcpy(filename, str);
    strcat(filename, num);
    strcat(filename, ".txt");
	strcpy(inputfile,filename);
	filePointer = fopen(inputfile, "r");
    filewrite = fopen(outputfile,"a");
	long double s[321],R[p+1],a[p+1],c[p+1],RSW_c[p+1];
	long double DCShift = GetDCShift();
	long double Normalize = NormFactor();
	findSteadyPoint();
	for(int i = 0;i<5; i++)
	{
	fetch_frame(s,DCShift,Normalize);	
	HammingWindow(s);
	Calc_Ri(s,R);
	Calc_ai(s,R,a);
	Calc_ci(a,R,c);
	Calc_ci_RSW(a,R,RSW_c);
	Write_Ci(RSW_c);
	}
	fclose(filewrite);
	}
	//now i have a.txt file with 100x12 values
	FILE* f = fopen(out,"r");
	long double value;
	if (f == NULL) {
        printf("Outputfile %s could not be opened.\n",out);
        return;  
    }
	int count=0, i=0;
	for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 12; j++) {
            if (fscanf(f, "%Lf", &mat[i][j]) != 1) {
                printf("Failed to readd data.\n");
                return ;
            }
        }
    }

//Now we will find the avg Ci for a
	for(int i = 0;i<5;i++)//frames
	{
		long double temp[12]={0}; //12 ci
		for(int j = 0; j<20; j++) //20 samples
		{
			for(int k=0;k<12;k++) //12 ci for each samp
			{
				temp[k] += mat[j*5+i][k];
			}
		}
		for(int z=0; z<12;z++)
			ref[i][z] = temp[z]/20;
	}
//put avg ci into ref file

	char vowel[100],op[100]="ref_";;
	sprintf(vowel, "%c",str[10]);
	strcat(op,vowel);
	strcat(op,".txt");   //op = ref_a.txt
	FILE* fp = fopen(op,"a");
	if (fp == NULL) {
        perror("Unable to open the output file");
        return;
	}

	for(int i = 0; i<5; i++)
		{
		for(int j=0;j<12;j++)
			fprintf(fp,"%Lf\t",ref[i][j]);
		fprintf(fp,"\n");
	}
	fclose(fp);
}
void TestModel(char str[])
{
	printf("\nVowel Recog Test for \'%c\'\nTESTFILE\t\tOUTPUT\tVERDICT",str[10]);
	for(int i = 21;i<=30; i++)  ////
	{
	char num[20];
    sprintf(num, "%d", i);
	char filename[100];
    strcpy(filename, str);
    strcat(filename, num);
    strcat(filename, ".txt"); //filename = 234101011_a_21.txt = inputfile
	strcpy(inputfile,filename);
	printf("\n%s\t",inputfile);
	filePointer = fopen(inputfile, "r");
	findSteadyPoint();
	long double RSW_c[5][p+1];
	long double s[321],R[p+1],a[p+1],c[p+1];
	long double DCShift = GetDCShift();
	long double Normalize = NormFactor();
	for(int y = 0;y<5; y++)
	{
	fetch_frame(s,DCShift,Normalize);	
	HammingWindow(s);
	Calc_Ri(s,R);
	Calc_ai(s,R,a);
	Calc_ci(a,R,c);
	Calc_ci_RSW(a,R,RSW_c[y]);
	}
	long double avgdist[5]={1000000000.00,1000000000.00,1000000000.00,1000000000.00,1000000000.00};
	char alp[5] = {'a','e','i','o','u'};
	for(int k=0;k<5;k++)///// change here
	{
	char vowel[100], op[100]="ref_";   //op is acnym for output file
	sprintf(vowel, "%c",alp[k]);
	strcat(op,vowel);
	strcat(op,".txt");  //op = ref_a.txt
	
	FILE* fp = fopen(op,"r");
		if (fp == NULL) {
        printf("Unable to open the ref file");
        return;
		}
	for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 12; j++) {
            if (fscanf(fp, "%Lf", &ref[i][j]) != 1) {
                printf("Failed to reaaad data.\n");
                return ;
				}
        }
	}
	fclose(fp);	
	for(int y = 0;y<5; y++)
	{
	dist[y] = Calc_DistT(RSW_c[y],ref[y]);
	}
	fclose(filePointer);
	long double tem=0;

	for(int u = 0;u<5; u++)
		tem += dist[u];
	avgdist[k] = tem/5;	
}
	int minDist=0;
	for(int i=1;i<5;i++){
				if(avgdist[i]<avgdist[minDist]){
			minDist=i;
		}
	}
	printf("\'%c\'\t", alp[minDist]);
	if(str[10]==alp[minDist]) printf("Correct");
	else printf("Wrong");
}
}
int _tmain(int argc, _TCHAR* argv[])
{
	
    while ((remove("a.txt") == 0)||(remove("e.txt") == 0)||(remove("i.txt") == 0)||(remove("o.txt") == 0)||(remove("u.txt") == 0)||
		(remove("ref_a.txt") == 0)||(remove("ref_e.txt") == 0)||(remove("ref_i.txt") == 0)||(remove("ref_o.txt") == 0)||(remove("ref_u.txt") == 0)); //clearing the old sample data
	
	char str_a[] = "234101011_a_";
	char str_e[] = "234101011_e_";
	char str_i[] = "234101011_i_";
	char str_o[] = "234101011_o_";
	char str_u[] = "234101011_u_";
	TrainModel(str_a);
	TrainModel(str_e);
	TrainModel(str_i);
	TrainModel(str_o);
	TrainModel(str_u);
	printf("\n!!!All models are trained successfully!!!.\n\nTesting the data against the models trained...\n\n");
	TestModel(str_a);
	TestModel(str_e);
	TestModel(str_i);
	TestModel(str_o);
	TestModel(str_u);
	getch();
	return 0;
}




