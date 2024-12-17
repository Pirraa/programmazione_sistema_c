#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <bits/fcntl-linux.h>
#define MAX 100

//nome file data di prenotazione
//contenuto file lunghezza,passeggeri,orario,codice taxi

static volatile sig_atomic_t count=0;

void handler()
{
    printf("Numero richieste: %d\n",count);
    exit(0);
}

void handler2(){}

int main(int argc,char *argv[])
{
    int pid1,pid2,pid3,p1p2[2],p2p3[2],fd,status,risultati;
    char codice[MAX],data[9],path[MAX*2];

    if(argc!=2)
    {
        printf("Uso: %s <file>\n",argv[0]);
        exit(1);
    }

    if(argv[1][0]=='/')
    {
        fprintf(stderr,"errore %s non è directory relativa",argv[1]);
        exit(2);
    }

    if((fd=open(argv[1],O_DIRECTORY))<0)
    {
        fprintf(stderr,"la directory non esiste o non hai i permessi");
        exit(3);
    }

    close(fd);

    signal(SIGINT,handler);

    while(1)
    {
        printf("Inserisci codice taxi: \n");
        scanf("%s",codice);

        printf("Inserisci la data: \n");
        scanf("%s",data);

        sprintf(path,"%s/%s.txt",argv[1],data);
        if((fd=open(path,O_RDONLY,0777))<0)
        {
            fprintf(stderr,"il file non esiste o non hai i permessi\n");
            continue;
        }
        close(fd);

        printf("Inserisci il numero n di risultati: \n");
        scanf("%d",&risultati);

        if(risultati<0)
        {
            fprintf(stderr,"i risultati devono essere >0\n");
            continue;
        }

        if(pipe(p1p2)<0)
        {
            perror("Errore prima pipe\n");
            exit(4);
        }

        if((pid1=fork())<0)
        {
            perror("Errore prima fork\n");
            exit(5);
        }

        if(pid1==0)
        {
            signal(SIGINT,SIG_DFL);
            signal(SIGUSR1,handler2);
            pause();

            close(p1p2[0]);
            close(1);
            dup(p1p2[1]);
            close(p1p2[1]);
            
            execlp("grep","grep",codice,path,NULL);
            perror("Errore prima execl\n");
            exit(6);
        }

        close(p1p2[1]);

        if(pipe(p2p3)<0)
        {
            perror("Errore seconda pipe\n");
            exit(7);
        }

        if((pid2=fork())<0)
        {
            perror("Errore seconda fork\n");
            exit(8);
        }

        if(pid2==0)
        {
            signal(SIGINT,SIG_DFL);
            close(0);
            dup(p1p2[0]);
            close(p1p2[0]);

            close(p2p3[0]);
            close(1);
            dup(p2p3[1]);
            close(p2p3[1]);
            execlp("sort", "sort", "-r", "-k", "2", "-n", "-t", ",", NULL);
            perror("Errore seconda execl\n");
            exit(9);
        }

        close(p1p2[0]);
        close(p2p3[1]);


        if((pid3=fork())<0)
        {
            perror("Errore terza fork\n");
            exit(11);
        }

        if(pid3==0)
        {
            signal(SIGINT,SIG_DFL);
            close(0);
            dup(p2p3[0]);
            close(p2p3[0]);

            char res[10];
            sprintf(res,"%d",risultati);
            execlp("head","head","-n",res,NULL);
            perror("Errore terza execl\n");
            exit(12);
        }

        close(p2p3[0]);
        sleep(1);
        kill(pid1,SIGUSR1);
        wait(&status);
        wait(&status);
        wait(&status);

        count++;
    }

}