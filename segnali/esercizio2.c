#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include<stdbool.h>
#include <signal.h>

volatile sig_atomic_t counter=0;
volatile sig_atomic_t Nf=0;
volatile sig_atomic_t flagTerminato=false;

void sig1_handler(int signal)
{
    counter++;
    printf("Child %d ha incrementato counter che vale %d\n",getpid(),counter);
}

void sig2_handler(int signal)
{
    counter--;
    printf("Child %d ha decrementato counter che vale %d\n",getpid(),counter);
}

void sigInt_handler(int signal)
{
    fflush(stdout);
    printf("Processo figlio %d ha counter pari a %d\n",getpid(),counter);
    exit(0);
}

void sigintparent(int signal)
{
    printf("Padre ha ricevuto ctrl-c attende i figli\n");
    int status;
    for(int i=0; i<Nf; i++)
    {
        wait(&status);
    }
    //metto variabile flag e la pongo a true qui, nel while in fondo al main cambio condizione mettendo il flag==false
    flagTerminato=true;
    //exit(0);
}

int main(int argc,char* argv[])
{
    struct sigaction sigint,sigusr1,sigusr2,sigintparent;
    if(argc!=2)
    {
        fprintf(stderr,"Argomenti errati\n");
        exit(2);
    }

    //si può aggiungere controllo con valore di ritorno atoi <=0 che indica errore di conversione
    Nf=atoi(argv[1]);
    //qui meglio usare una malloc e allora dinamicamente l'array
    //int *pids=(int*)malloc(Nf*sizeof(int));
    int pids[Nf];
    signal(SIGINT,sigInt_handler);

    for(int i=0; i<Nf; i++)
    {
        int pid=fork();
        if(pid==0)
        {
            //figlio sospende finchè non riceve segnale, reinstalla i segnali e poi si risospende
            sigemptyset(&sigint.sa_mask);
            sigint.sa_flags=0;
            sigint.sa_handler=sigInt_handler;
            if(sigaction(SIGINT,&sigint,NULL)==-1)
            {
               perror("Errore sigaction");
               exit(3);
            }

            sigemptyset(&sigusr1.sa_mask);
            sigusr1.sa_flags=0;
            sigusr1.sa_handler=sig1_handler;
            if(sigaction(SIGUSR1,&sigusr1,NULL)==-1)
            {
                perror("Errore sigaction");
                exit(3);
            }
            
            sigemptyset(&sigusr2.sa_mask);
            sigusr2.sa_flags=0;
            sigusr2.sa_handler=sig2_handler;
            if(sigaction(SIGUSR2,&sigusr2,NULL)==-1)
            {
                perror("Errore sigaction");
                exit(3);
            }
            //signal(SIGUSR1,sig1_handler);
            //signal(SIGUSR2,sig2_handler);
            while(1){
               pause();//si sospende finchè non riceve un segnale 
            }
        }else if(pid>0)
        {
            pids[i]=pid;
        }
    }

    sigemptyset(&sigintparent.sa_mask);
    sigintparent.sa_flags=0;
    sigintparent.sa_handler=sigInt_handler;
    if(sigaction(SIGINT,&sigintparent,NULL)==-1)
    {
        perror("Errore sigaction");
        exit(3);
    }

    while(!flagTerminato)
    {
        sleep(2);
        int figlio=rand()%Nf;
        int sig=rand()%2;
        if(sig==0)
        {
            kill(pids[figlio],SIGUSR1);
        }
        else
        {
            kill(pids[figlio],SIGUSR2);
        }
    }
    return 0;
}