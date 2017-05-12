
/*
// Projeto SO - exercicio 1, version 1
// Sistemas Operativos, DEI/IST/ULisboa 2016-17
*/

#include "commandlinereader.h"
#include "contas.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

#define COMANDO_DEBITAR "debitar"
#define COMANDO_CREDITAR "creditar"
#define COMANDO_LER_SALDO "lerSaldo"
#define COMANDO_SIMULAR "simular"
#define COMANDO_SAIR "sair"
#define COMANDO_ARG_SAIR_AGORA "agora"

#define MAXARGS 3
#define BUFFER_SIZE 100

#define MAXFILHOS 20

int main (int argc, char** argv) {
  
  char *args[MAXARGS + 1];
  char buffer[BUFFER_SIZE];
  
  int numFilhos = 0;
  pid_t pidFilhos[MAXFILHOS];
  
  inicializarContas();
  
  /* Associa o signal SIGUSR1 'a funcao trataSignal.
     Esta associacao e' herdada pelos processos filho que venham a ser criados.
     Alternativa: cada processo filho fazer esta associacao no inicio da
     funcao simular; o que se perderia com essa solucao?
     Nota: este aspeto e' de nivel muito avancado, logo
     so' foi exigido a projetos com nota maxima  
  */
  if (signal(SIGUSR1, trataSignal) == SIG_ERR) {
    perror("Erro ao definir signal.");
    exit(EXIT_FAILURE);
  }
  
  printf("Bem-vinda/o ao i-banco\n\n");
  
  while (1) {
    int numargs;
    
    numargs = readLineArguments(args, MAXARGS+1, buffer, BUFFER_SIZE);
    
    /* EOF (end of file) do stdin ou comando "sair" */
    if (numargs < 0 ||
	(numargs > 0 && (strcmp(args[0], COMANDO_SAIR) == 0))) {
      
      printf("i-banco vai terminar.\n--\n");
      
      if (numargs > 1 && (strcmp(args[1], COMANDO_ARG_SAIR_AGORA) == 0)) {
	int i;
	for (i=0; i<numFilhos; i++)
	  kill(pidFilhos[i], SIGUSR1);
      }
      /* Uma alternativa igualmente correta: kill(0, SIGUSR1); */
      
      /* Espera pela terminacao de cada filho */
      while (numFilhos > 0) {
	int status;
	pid_t childpid;
	
	childpid = wait(&status);
	if (childpid < 0) {
	  if (errno == EINTR) {
	    /* Este codigo de erro significa que chegou signal que interrompeu a espera
	       pela terminacao de filho; logo voltamos a esperar */
	    continue;
	  }
	  else {
	    perror("Erro inesperado ao esperar por processo filho.");
	    exit (EXIT_FAILURE);
	  }
	}
	
	numFilhos --;
	if (WIFEXITED(status))
	  printf("FILHO TERMINADO: (PID=%d; terminou normalmente)\n", childpid);
	else
	  printf("FILHO TERMINADO: (PID=%d; terminou abruptamente)\n", childpid);
      }
      
      printf("--\ni-banco terminou.\n");
      exit(EXIT_SUCCESS);
    }
    
    else if (numargs == 0)
      /* Nenhum argumento; ignora e volta a pedir */
      continue;
    
    /* Debitar */
    else if (strcmp(args[0], COMANDO_DEBITAR) == 0) {
      int idConta, valor;
      if (numargs < 3) {
	printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_DEBITAR);
	continue;
      }
      
      idConta = atoi(args[1]);
      valor = atoi(args[2]);
      
      if (debitar (idConta, valor) < 0)
	printf("%s(%d, %d): OK\n\n", COMANDO_DEBITAR, idConta, valor);
      else
	printf("%s(%d, %d): OK\n\n", COMANDO_DEBITAR, idConta, valor);
    }
    
    /* Creditar */
    else if (strcmp(args[0], COMANDO_CREDITAR) == 0) {
      int idConta, valor;
      if (numargs < 3) {
	printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_CREDITAR);
	continue;
      }
      
      idConta = atoi(args[1]);
      valor = atoi(args[2]);
      
      if (creditar (idConta, valor) < 0)
	printf("%s(%d, %d): Erro\n\n", COMANDO_CREDITAR, idConta, valor);
      else
	printf("%s(%d, %d): OK\n\n", COMANDO_CREDITAR, idConta, valor);
    }
    
    /* Ler Saldo */
    else if (strcmp(args[0], COMANDO_LER_SALDO) == 0) {
      int idConta, saldo;
      
      if (numargs < 2) {
	printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_LER_SALDO);
	continue;
      }
      idConta = atoi(args[1]);
      saldo = lerSaldo (idConta);
      if (saldo < 0)
	printf("%s(%d): Erro.\n\n", COMANDO_LER_SALDO, idConta);
      else
	printf("%s(%d): O saldo da conta é %d.\n\n", COMANDO_LER_SALDO, idConta, saldo);
    }
    
    /* Simular */
    else if (strcmp(args[0], COMANDO_SIMULAR) == 0) {
      
      int numAnos;
      int pid;
      
      if (numFilhos >= MAXFILHOS) {
	printf("%s: Atingido o numero maximo de processos filho a criar.\n", COMANDO_SIMULAR);
	continue;
      }
      if (numargs < 2) {
	printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_SIMULAR);
	continue;
      }
      
      numAnos = atoi(args[1]);
      
      pid = fork();
      if (pid < 0) {
	perror("Failed to create new process.");
	exit(EXIT_FAILURE);
      }
      
      if (pid > 0) { 	 
	pidFilhos[numFilhos] = pid;
	numFilhos ++;
	printf("%s(%d): Simulacao iniciada em background.\n\n", COMANDO_SIMULAR, numAnos);
	continue;
      }
      else {
	simular(numAnos);
	exit(EXIT_SUCCESS);
      }
    }
    
    else
      printf("Comando desconhecido. Tente de novo.\n");
    
  } 
}
