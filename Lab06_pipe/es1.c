#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#define MAX_PROC 10
#define DIM 100
/*
Creare un programma in C usando fork e pipe che generi N processi
figlio. Ognuno dei figli deve utilizzare una differente pipe per
comunicare con il padre, mandandogli un messaggio testuale
contente il suo PID.
Il programma presenta l'interfaccia:
visualizza_messaggio <num_fork>
dove <num_fork> è un intero che rappresenta il numero di figli che il
processo padre deve creare. Ciascun figlio deve mandare al padre un
messaggio del tipo "Hello world! sono il processo: <pid_figlio>". Il
processo padre riceve ciascun messaggio, li stampa sullo standard
output e infine termina.
*/
int main(int argc,char *argv[])
{
    int pid,status;
    char messaggio[DIM];

    //controllo argomenti
    if(argc!=2)
    {
        perror("Errore argomenti");
        exit(1);
    }

    //controllo argomento numero positivo
    for (int i=0; i<strlen(argv[1]); i++) {
        if (argv[1][i] < '0' || argv[1][i] > '9') {
            perror("Errore: il numero di fork deve essere un intero positivo");
            exit(1);
        }
    }

    //controllo numero nel range giusto
    int Nf=atoi(argv[1]);
    if(Nf>MAX_PROC)
    {
        perror("Errore: il numero di fork deve essere minore di 10");
        exit(1);
    }
    //credo Nf pipe 
    int pipefd[Nf][2];
    for (int i = 0; i < Nf; i++) {
        if (pipe(pipefd[i]) == -1) {
            perror("pipe");
            exit(1);
        }
    }

    //creo Nf processi figli
    for(int i=0; i<Nf;i++)
    {
        pid=fork();
        if(pid==0)
        {
            //può accedere a qualunque delle pipe ma usa solo il canale di scrittura della i-esima pipe
            close(pipefd[i][0]);
            sprintf(messaggio,"Hello world! sono il processo: %d\n",getpid());
            //scrivo un numero di byte pari alla lunghezzza del messaggio più uno cioè il terminatore
            write(pipefd[i][1],messaggio,strlen(messaggio)+1);
            close(pipefd[i][1]);
            exit(0);
        }else{
            printf("Fork numero %d\n",i+1);
        }
    }
    for(int i=0; i<Nf; i++)
    {
        close(pipefd[i][1]);
        //leggo dim caratteri e li metto su messaggio
        read(pipefd[i][0],messaggio,DIM);
        printf("%s\n",messaggio);
        close(pipefd[i][0]);
    }

    // Attende la terminazione di tutti i processi figli
    for (int i = 0; i < Nf; i++) {
        wait(&status);
    }

    return 0;
}