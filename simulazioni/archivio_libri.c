#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <error.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <asm-generic/fcntl.h>
#define MAX 100

//sleep prima di kill
//segnale solo a un processo lo installo solo su quello e non nel padre, sennò lo ereditano tutti i figli e tocca dirgli di ingnorarlo
//exit dopo le perror delle execl
//sprintf per trasformare intero in stringa da mettere nelle exec
//nella grep non passare fd come stdin ma metti il file come argomento
//ciclo infinito con break su stringa fine e continue su errori di inserimento 
//apro cartella con open meglio prima del ciclo, perchè se è sbagliata non posso reinserirla e killo il programma
//controllo file (creando percorso con sprintf) nel ciclo con continue se non va a buon fine
//pause meglio prima di fare le ridirezioni

static volatile sig_atomic_t count = 0;

void handler() 
{ printf("Numero richieste: %d\n", count); exit(0); }

void handler_parti(){}

int main(int argc,char* argv[])
{
    char cognome[MAX];
    char genere[MAX];
    char path[MAX*2];
    int risultati;
    int p1p2[2];
    int pid1,pid2;
    if(argc!=2)
    {
        fprintf(stderr,"Errore: numero di argomenti sbagliato.\n");
        exit(1);
    }
    if(argv[1][0]!="/")
    {
        fprintf(stderr,"Errore: la directory deve essere assoluta.\n");
        exit(2);
    }
    //nome file: genere.txt
    //contenuto file: anno,titolo,cognome,casa

    if(open(argv[1],O_DIRECTORY,0777)<0)
    {
        fprintf(stderr,"Errore: la directory non è accessibile.\n");
        exit(3);
    }

    signal(SIGINT,handler);

    while(strcmp(cognome,"fine")==0 || strcmp(genere,"fine")==0)
    {
        printf("Inserisci il cognome dell'autore:\n");
        scanf("%s",cognome);

        printf("Inserisci il genere del libro:\n");
        scanf("%s",genere);

        printf("Inserisci il numero di risultati da mostrare:\n");
        scanf("%d",&risultati);

        if(risultati<=0)
        {
            fprintf(stderr,"Errore: il numero di risultati da mostrare deve essere maggiore di 0\n");
            continue;
        }
        sprintf(path,"%s/%s.txt",argv[1],genere);
        if(open(path,O_RDONLY,0777)<0)
        {
            fprintf(stderr,"Errore: apertura file non riuscita.\n");
            continue;
        }

        if(pipe(p1p2)<0)
        {
            fprintf(stderr,"Errore: creazione della pipe non riuscita.\n");
            exit(4);
        }

        if(pid1=fork()<0)
        {
            fprintf(stderr,"Errore: creazione del primo figlio non riuscita.\n");
            exit(5);
        }else if(pid1==0)
        {
            signal(SIGINT,SIG_DFL);
            signal(SIGUSR1,handler_parti);
            close(p1p2[0]);
            close(1);
            dup(p1p2[1]);
            close(p1p2[1]);
            pause();

            execlp("grep","grep",cognome,genere,NULL);
            perror("errore grep");
            exit(6);
        }

        if(pid2=fork()<0)
        {
            fprintf(stderr,"Errore: creazione del secondo figlio non riuscita.\n");
            exit(6);
        }else if(pid2==0)
        {
            signal(SIGINT,SIG_DFL);
            close(p1p2[1]);
            close(0);
            dup(p1p2[0]);
            close(p1p2[0]);
            char risultati_str[MAX];
            sprintf(risultati_str,"%d",risultati);
            execl("head","head","-n",risultati_str,NULL);
            perror("errore tail");
            exit(7);
        }

        close(p1p2[0]);
        close(p1p2[1]);

        sleep(1);
        kill(pid1,SIGUSR1);

        wait(NULL);
        wait(NULL);

        count++;
    }
    return 0;
}