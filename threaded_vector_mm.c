
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <emmintrin.h>
#define NUM_THREADS	9 //n*m


struct thread_data
{
   int	thread_id;
   int  n;
   int  m;
};

struct thread_data thread_data_array[NUM_THREADS];



void print128_float(__m128 var) {
    float *fval = (float*) &var;
    printf("%f %f %f %f\n", fval[3], fval[2], fval[1], fval[0]);
}
float* convert_128float_to_arr(__m128 var) {
  float *fval = (float*) &var;
  return fval;
}


/*
  Doing matrix multiplication does not require any mutexs

  1 2 3   11 12 13      [1*11+2*14+3*17] [1*12+2*15+3*18] [1*13+2*16+3*19]
  4 5 6 * 14 15 16 =    [4*11+5*14+6*17] [4*12+5*15+6*18] [4*13+5*16+6*19]
  7 8 9   17 18 19      [7*11+8*14+9*17] [7*12+8*15+9*18] [7*13+8*16+9*19]
*/

//Matrices and their dimensions are globally accessible
float n[3][3] = {{1,2,3},{4,5,6},{7,8,9}};
int n_w = 3;
int n_h = 3;
float m[3][3] = {{11,12,13},{14,15,16},{17,18,19}};
int m_w = 3;
int m_h = 3;


float result[3][3];



//Calculate the resultant (n,m) index in the matrix
void *Element(void *threadarg)
{
   int taskid, ni, mi;
   int i;
   struct thread_data *my_data;
   //sleep(1); //Wait so the print on console is in order
   my_data = (struct thread_data *) threadarg;

   //Calculate the (n,m) element of the resultant matrix
   int sum = 0;
   for(i = 0; i < 3; i++) {
     //row * column
     sum += n[my_data->n][i] * m[i][my_data->m];
   }
   printf("THREAD - threadID:%d n=%d m=%d sum=%d \n", my_data->thread_id, my_data->n, my_data->m, sum);
   //Save to the result array
   result[my_data->n][my_data->m] = sum;
   pthread_exit(NULL);
}


//Calculate the resultant (n,m) index in the matrix (USING VECTOR ARITHMITIC)
void *ElementVec(void *threadarg)
{

   float sumf;
   int taskid, ni, mi;
   struct thread_data *my_data;
   my_data = (struct thread_data *) threadarg;

   //Calculate the (n,m) element of the resultant matrix
   __m128 mul, a, b;
   a = _mm_loadu_ps(n[my_data->n]); //load n[0]
   b = _mm_set_ps(-1,(m[0][my_data->m]+6),(m[0][my_data->m]+3),m[0][my_data->m]);
   mul = _mm_mul_ps(a,b);

   float* ptr = convert_128float_to_arr(mul);
   sumf = ptr[2] + ptr[1] + ptr[0];
   //printf("threadID:%d Summing:%f %f %f Sum=%f\n", my_data->thread_id, ptr[2],ptr[1],ptr[0],sumf);
   printf("THREAD_VEC - threadID:%d n=%d m=%d sum=%f \n", my_data->thread_id, my_data->n, my_data->m, sumf);


   //Save to the result array
   result[my_data->n][my_data->m] = sumf;
   pthread_exit(NULL);
}




int main(int argc, char *argv[])
{
  pthread_t threads[NUM_THREADS];

  int threadId = 0;
  int i,j;
  for(i = 0; i< 3; i++) {
    for(j = 0; j < 3; j++) {
      thread_data_array[threadId].thread_id = threadId;
      thread_data_array[threadId].n = i;
      thread_data_array[threadId].m = j;
      printf("Creating thread %d\n", threadId);
      pthread_create(&threads[threadId], NULL, ElementVec, (void *)&thread_data_array[threadId]);
      threadId++;
    }
  }

  //Wait for all threads to complete
  for(i = 0; i < NUM_THREADS; i++) {
    pthread_join( threads[i], NULL);
  }

  //print the result array
  printf("\nResult:\n");
  for(i = 0; i< 3; i++) {
    for(j = 0; j < 3; j++) {
      printf("%f\t", result[i][j]);
    }
    printf("\n");
  }

  return 0;
}
