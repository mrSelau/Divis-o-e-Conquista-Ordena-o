#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h> 
#include <sys/timeb.h>
#include <mpi.h>
#include <sys/time.h>

#define VETOR_SIZE 100000 

void bs(int n, int * vetor){
    int c=0, d, troca, trocou =1;

    while ((c < (n-1)) & trocou ){ 
        trocou = 0;
        for (d = 0 ; d < n - c - 1; d++)
            if (vetor[d] > vetor[d+1])
                {
                troca      = vetor[d]; 
                vetor[d]   = vetor[d+1];
                vetor[d+1] = troca;
                trocou = 1;
                }
        c++;
        }
}

int *interleaving(int vetor[], int tam){
  int *vetor_auxiliar;
  int i1, i2, i_aux;

  vetor_auxiliar = (int *)malloc(sizeof(int) * tam);

  i1 = 0;
  i2 = tam / 2;

  for (i_aux = 0; i_aux < tam; i_aux++){
    if (((vetor[i1] <= vetor[i2]) && (i1 < (tam / 2)))
        || (i2 == tam))
      vetor_auxiliar[i_aux] = vetor[i1++];
    else
      vetor_auxiliar[i_aux] = vetor[i2++];
  }

  return vetor_auxiliar;
}

int cmpfunc (const void * a, const void * b) {
   return ( *(int*)a - *(int*)b );
}

int main(int argc, char** argv){
  MPI_Status status;
  int my_rank;       // Identificador deste processo
  int proc_n;        // Numero de processos disparados pelo usuario na linha de comando (np)      
  int mid;
  int *vetor_aux;
  int tam_vetor = VETOR_SIZE;
  int nivel,pai;
  int delta = 25000;
  double t1,t2;

  MPI_Init(&argc , &argv); // funcao que inicializa o MPI, todo o codigo paralelo estah abaixo

  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); // pega pega o numero do processo atual (rank)
  MPI_Comm_size(MPI_COMM_WORLD, &proc_n);  // pega informacao do numero de processos (quantidade total)

  int *vetor = (int *)malloc(sizeof(int)*tam_vetor);
  // recebo vetor

  if ( my_rank != 0 ){
    MPI_Recv(vetor, VETOR_SIZE, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD,&status);
    pai = status.MPI_SOURCE; //conferir
    MPI_Get_count(&status, MPI_INT, &tam_vetor);
  }
  else{
        t1 = MPI_Wtime();
      int i = 0;
      for(; i < tam_vetor; i++){// sou a raiz e portanto gero o vetor - ordem reversa
          vetor[i] = tam_vetor - i;
      }
      mid = tam_vetor / 2;
  }
  // dividir ou conquistar?

  if ( tam_vetor <= delta ){
    printf("%d\n", my_rank);
	qsort(vetor, tam_vetor, sizeof(int), cmpfunc);
    vetor_aux = vetor;
  }  // conquisto
  else{
      // dividir
      // quebrar em duas partes e mandar para os filhos
      printf("%d\n", my_rank);
      MPI_Send(&vetor[0], tam_vetor/2, MPI_INT, (2*my_rank)+1, 0, MPI_COMM_WORLD);
      MPI_Send(&vetor[tam_vetor/2], tam_vetor/2, MPI_INT, (2*my_rank)+2, 0,MPI_COMM_WORLD);
      // receber dos filhos

      MPI_Recv(&vetor[0], tam_vetor/2, MPI_INT, (2*my_rank)+1, 0, MPI_COMM_WORLD,
       &status);
      MPI_Recv(&vetor[tam_vetor/2], tam_vetor/2, MPI_INT, (2*my_rank)+2, 0, MPI_COMM_WORLD,
       &status);

      vetor_aux = interleaving(vetor, tam_vetor);// intercalo vetor inteiro
  }

  // mando para o pai

  if ( my_rank !=0 ){
     MPI_Send(vetor_aux, tam_vetor, MPI_INT, pai, 0,
         MPI_COMM_WORLD);
  }
  else{
  	int i = 0;
  	for(;i<tam_vetor;i++){
  		printf("%d\n", vetor[i]);
  	}
        t2 = MPI_Wtime(); // termina a contagem do tempo
        printf("\nTempo de execucao: %f\n\n", t2-t1);
  }

  MPI_Finalize();
  free(vetor_aux);
  return 1;
}

