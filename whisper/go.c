// generate /tmp/mel80 80-mel data to feed to whisper_test/doit
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <strings.h>

#define WINRES 32.0
#define FFT 128
#define FF2 65
#define N 64 // 10ms
#define NW  128        /* analysis window size   (279)            */

// number of top-N bins to use......
#define TOPN 5 // 5 or 4 is good, 3 OK, even 2 sometimes OK

#define CHANS80 80

//---------------------------------------------------------------------------

void main()
{
  FILE *fp,  *fp_out;
  
  int i,j,k,m,n,posn=0,size,z,bot,top;

  double x,y;
  static float f[FFT],f2[FFT],f3[201];
  int window[FFT],w[FFT],prev=0;
  static short s[FFT];
  float mel[CHANS80],tot;
  static float mel2[CHANS80][3000];  // normally 3000
  float wts[CHANS80][201];
  static int fi[FFT];
  int b[3000][5],bin;
  float e[3000][5],max,max3;
  int div,re,im;
  double mag;
  char c[200];
  
  printf("decoding /tmp/a.raw to /tmp/mel80.....\n");
  
  fp=fopen("wts","r"); z=fread(wts,CHANS80*201,4,fp); fclose(fp);
  
  fp=fopen("f2.txt","r"); fp_out=fopen("/tmp/mel80","w");
  size=0;
  //z=fscanf(fp,"%s\n",c);
  while(!feof(fp)) {
    z=fscanf(fp,"%i,",&div);
    printf("%i:  ",div);
    for (n=0; n<5; n++) {
       z=fscanf(fp,"%i,%i,%i,",&bin, &re, &im);
       //bin++; // helps?
       //printf("%i,%i,%i,",bin,re,im);
       printf("%i,",bin);
       mag=re*re+im*im;
       mag/=(WINRES*WINRES); //  undo window scaling
       mag/=(32768.0*32768.0);
       mag*=(div*div);
       if (mag!=0) mag=log(mag); else mag=-99999999;
       b[size][n]=bin; e[size][n]=mag;
    }
    printf("\n");
    z=fscanf(fp,"\n"); 
    //for (n=0; n<5; n++) printf("%i:%.2f, ",b[size][n],e[size][n]); printf("\n");
    size++;
  }
  printf("size=%i\n",size);
  //exit(0);
  
  //----------------------------------------------------------
  // now generate mel2[][] from the 'posn' frames....
  max=-999999;
  for (posn=0; posn<size; posn++) {
    for (n=0; n<FF2; n++) f2[n]=-100.0;
   
    //printf("%3i:  \n",posn); 
    for (i=0; i<TOPN; i++) {  
        top=b[posn][i]; max3=e[posn][i];
	//printf("%i: %f,",top,max3);
        f2[top]=max3; // default actual energy
        f[top]=-999;
        if (top>0)    if (f2[top-1]<f2[top]-2) f2[top-1]=f2[top]-2;
        if (top<FF2-1) if (f2[top+1]<f2[top]-2) f2[top+1]=f2[top]-2;
        if (top>10)   if (f2[top+2]<f2[top]-3) f2[top+2]=f2[top]-3;
        if (top>10)  if (f2[top-2]<f2[top]-3) f2[top-2]=f2[top]-3;
        if (top>20)   if (f2[top+3]<f2[top]-4) f2[top+4]=f2[top]-4;
        if (top>20)   if (f2[top-3]<f2[top]-4) f2[top-3]=f2[top]-4;
        if (top>30)   f2[top+4]=f2[top]-3;
        if (top>30)   f2[top-4]=f2[top]-3;
    }
    for (n=0; n<FF2; n++) f[n]=f2[n];
    for (i=0; i<FF2; i++) f[i]=exp(f[i]);
    for (i=0; i<201; i++) f3[i]=0; // sanity
      for (n=0; n<81; n++)  {  // need to strech the FFT bins slightly....
       i=n*FF2/81;
       f3[n]=f[i];
    }
    // now estimate the 80 Mel bins...
    for (n=0; n<CHANS80; n++) {
       tot=0.0; for (i=0; i<201; i++) tot+=(f3[i])*wts[n][i];
       mel[n]=tot;
    }
 
    for (n=0; n<CHANS80; n++) {
       if (mel[n]<1e-10) mel[n]=1e-10;
       mel[n]=log10f(mel[n]);  
       if (mel[n]>max) max=mel[n];
    }
    for (n=0; n<CHANS80; n++) mel2[n][posn]=mel[n];   
  }
   
  // tidy-up across all (up to 3000) frames now...
  for (i=0; i<size; i++) for (n=0; n<CHANS80; n++){  
    //if (mel2[n][i]<(max-8.0)) mel2[n][i]=max-8.0; // default, too high
    //if (mel2[n][i]<(max-6.0)) mel2[n][i]=max-6.0;
    //if (mel2[n][i]<(max-5.0)) mel2[n][i]=max-5.0;
    if (mel2[n][i]<(max-4.5)) mel2[n][i]=max-4.5; // best
    mel2[n][i]=(mel2[n][i]+4.0)/4.0;
  }
  // pad out the remaining space to 30 seconds...
  for (i=size; i<3000; i++) for (n=0; n<CHANS80; n++) mel2[n][i]=(max-8.0+4.0)/4.0;
  fwrite(mel2,80*3000,4,fp_out);
  fclose(fp); fclose(fp_out);
 
}
