//=======================================================================================
//
// SAMPLE SOURCE CODE - SUBJECT TO THE TERMS OF SAMPLE CODE LICENSE AGREEMENT,
// http://software.intel.com/en-us/articles/intel-sample-source-code-license-agreement/
//
// Copyright 2013 Intel Corporation
//
// THIS FILE IS PROVIDED "AS IS" WITH NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT
// NOT LIMITED TO ANY IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
// PURPOSE, NON-INFRINGEMENT OF INTELLECTUAL PROPERTY RIGHTS.
//
// ======================================================================================
 


#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include"timer.h"
#if (defined(_WIN32) && defined(__INTEL_COMPILER))
#include<cilk\cilk.h>
#elif (defined(__GNUC__) && defined(__INTEL_COMPILER))
#include<cilk/cilk.h>
#endif

#if defined(_WIN32)
#include <malloc.h>
#else
#include <mm_malloc.h>
#endif

#include "SepiaFilterCilkPlus.h"
#define ALIGNMENT 32 //Set to 16 bytes for SSE architectures and 32 bytes for Intel(R) AVX architectures
using namespace std;

//Processing API is having one Array Notation versions and another one in else block which is the plain AOS version

void process_image_AOS(rgb &indataset, rgb &outdataset){
	float temp;
	temp = (0.393f * indataset.red) + (0.769f * indataset.green) + (0.189f * indataset.blue);
	outdataset.red = temp>255?255:temp;
	temp = (0.349f * indataset.red) + (0.686f * indataset.green) + (0.168f * indataset.blue);
	outdataset.green = temp>255?255:temp;
	temp = (0.272f * indataset.red) + (0.534f * indataset.green) + (0.131f * indataset.blue);
	outdataset.blue = temp>255?255:temp;
	return;
}


//This API does the reading and writing from/to the .bmp file. Also invokes the image processing API from here
#if defined(_WIN32)
__declspec(noinline) 
#else
__attribute__ ((noinline))
#endif
int read_process_write(char* input, char *output, int choice) {

    FILE *fp,*out;
    bitmap_header* hp;
    int n;
	CUtilTimer timer;
	float *temp;
    // Making sure the AOS alignes to an address which is multiple of 16 to support vectorization 
#if defined(_WIN32)
    __declspec(align(ALIGNMENT)) rgb *indata, *outdata;
#else
	__attribute__((aligned(ALIGNMENT))) rgb *indata, *outdata;
#endif

    //Instantiating a file handle to open a input BMP file in binary mode
    fp = fopen(input, "rb");
    if(fp==NULL){
        cout<<"The file could not be opened. Program will be exiting\n";
	return 0;
    }


    //Allocating memory for storing the bitmap header information which will be retrived from input image file
    hp=(bitmap_header*)malloc(sizeof(bitmap_header));
    if(hp==NULL)
    {
	cout<<"Unable to allocate the memory for bitmap header\n";
        return 0;
    }

    //Reading from input file the bitmap header information which is inturn stored in memory allocated in the previous step
    n=fread(hp, sizeof(bitmap_header), 1, fp);
        if(n<1){
            cout<<"Read error from the file. No bytes were read from the file. Program exiting \n";
            return 0;        
        }

    if(hp->bitsperpixel != 24){
        cout<<"This is not a RGB image\n";
        return 0;
    }

	//Size of the image in terms of number of pixels
	int size_of_image = hp->width * hp->height;
    //Allocate memory for loading the bitmap data of the input image
#if defined(_WIN32)
    indata = (rgb *)_aligned_malloc((sizeof(rgb)*(size_of_image)), ALIGNMENT);
#else
	indata = (rgb *)_mm_malloc((sizeof(rgb)*(size_of_image)), ALIGNMENT);
#endif
    if(indata==NULL){
        cout<<"Unable to allocate the memory for bitmap date\n";
        return 0;
    }

    // Setting the File descriptor to the starting point in the input file where the bitmap data(payload) starts
    fseek(fp,sizeof(char)*hp->fileheader.dataoffset,SEEK_SET);

    // Reading the bitmap data from the input bmp file to the memory allocated in the previous step
    n=fread(indata, sizeof(rgb), (size_of_image), fp);
    if(n<1){
        cout<<"Read error from the file. No bytes were read from the file. Program exiting \n";
        return 0;
    }
	
	//Allocate memory for storing the bitmap data of the processed image
#if defined(_WIN32)
    outdata = (rgb *)_aligned_malloc((sizeof(rgb)*(size_of_image)), ALIGNMENT);
#else
	outdata = (rgb *)_mm_malloc((sizeof(rgb)*(size_of_image)), ALIGNMENT);
#endif
    if(outdata==NULL){
        cout<<"Unable to allocate the memory for bitmap date\n";
        return 0;
    }
    // Involing the image processing API which does some manipulation on the bitmap data read from the input .bmp file
		long long avg_time;
		avg_time = 0;
		for(int k=0; k<5; ++k) {

    timer.start();
				cilk_for(int i = 0; i < size_of_image; i++)
				{
					process_image_AOS(indata[i], outdata[i]);
				}
				timer.stop();

		avg_time += timer.get_ticks();
		}
		avg_time /= 5;

	// Opening an output file to which the processed result will be written
    out = fopen(output, "wb");
    if(out==NULL){
        cout<<"The file could not be opened. Program will be exiting\n";
        return 0;
    }

    // Writing the bitmap header which we copied from the input file to the output file. We need not make any changes because we haven't made any change to the image size or compression type.
    n=fwrite(hp,sizeof(char),sizeof(bitmap_header),out);
    if(n<1){
        cout<<"Write error to the file. No bytes were wrtten to the file. Program exiting \n";
        return 0;
    }

    //Setting the file descriptor to point to the location where the bitmap data is to be written 
    fseek(out,sizeof(char)*hp->fileheader.dataoffset,SEEK_SET);

    // Writing the bitmap data of the processed image to the output file
    n=fwrite(outdata,sizeof(rgb),(size_of_image),out);
    if(n<1){
        cout<<"Write error to the file. No bytes were wrtten to the file. Program exiting \n";
        return 0;
    }
	cout<<avg_time<<"\n";
    // Closing all file handles and also freeing all the dynamically allocated memory
    fclose(fp);
    fclose(out);
    free(hp);
#if defined(_WIN32)
    _aligned_free(indata);
    _aligned_free(outdata);
#else
	_mm_free(indata);
	_mm_free(outdata);
#endif
    return 0;
}
int main(int argc, char *argv[]){
		int choice = 2;
        read_process_write(argv[1], argv[2], choice);
        return 0;
}





