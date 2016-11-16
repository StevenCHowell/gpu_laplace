#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <omp.h>
#include "constants.h"

using namespace std;

#pragma acc routine seq
unsigned char mandelbrot(int Px, int Py);

int main() {
  
  size_t bytes=WIDTH*HEIGHT*sizeof(unsigned int);
  unsigned char *image=(unsigned char*)malloc(bytes);
  int num_blocks = 8;
  const int y_block = HEIGHT/num_blocks;
  const int block_size = y_block*WIDTH;
  FILE *fp=fopen("image.pgm","wb");
  fprintf(fp,"P5\n%s\n%d %d\n%d\n","#comment",WIDTH,HEIGHT,MAX_COLOR);
  double st = omp_get_wtime();

#pragma acc data create(image[WIDTH*HEIGHT]) 
  for(int n=0;n<num_blocks;n++) {
#pragma acc update device(image[n*block_size:block_size]) async(n%3+1)
#pragma acc parallel loop async(n%3+1)
    for(int y=n*y_block;y<(n+1)*y_block;y++) {
      for(int x=0;x<WIDTH;x++) {
	image[y*WIDTH+x]=mandelbrot(x,y);
      }
    }
#pragma acc update self(image[n*block_size:block_size]) async(n%3+1)
  }
#pragma acc wait

  double et = omp_get_wtime();
  printf("Time: %lf seconds.\n", (et-st));
  fwrite(image,sizeof(unsigned char),WIDTH*HEIGHT,fp);
  fclose(fp);
  free(image);
  return 0;
}
