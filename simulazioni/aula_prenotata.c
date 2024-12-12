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
static volatile sig_atomic_t count=0;

void handler(){printf("le richieste sono %d\n",count);exit(0);}


int main(int argc,char*argv[])
{
    //cartella assoluta
    //file con nome aula
    //contenuto file orario inizio,orario fine, data, codice corso,nome docente
    char aula[MAX];
    char data[9];
    int p1p2[2],p2p3[2],p3p0[2];
    int pid1,pid2,pid3;
    int risultati;
    int fd;
    int nread;
    char buffer[MAX];

    if(argc!=2)
    {
        fprintf(stderr,"errore argomenti\n");
        exit(1);
    }

    if(argv[1][0]!='/')
    {
        fprintf(stderr,"errore directory non assoluta\n");
    }

    if((fd = open(argv[1], O_DIRECTORY)) < 0){
		fprintf(stderr, "Errore: %s non ha i permessi necessari o non è una cartella\n", argv[1]);
		exit(EXIT_FAILURE);
	}
	close(fd);

    signal(SIGINT,handler);

    while(1)
    {
        printf("inserisci il nome dell'aula\n");
        scanf("%s",aula);

        printf("inserisci la data\n");
        scanf("%s",data);

        printf("inserisci il numero di risultati da mostrare\n");
        scanf("%d",&risultati);

        if(risultati<=0)
        {
            fprintf(stderr,"errore risultati non validi\n");
            continue;
        }

        char path[MAX*2];
        sprintf(path,"%s/%s.txt",argv[1],aula);

        fd=open(path,O_RDONLY,0777);
        if(fd<0)
        {
            fprintf(stderr,"errore apertura file\n");
            continue;
        }
        close(fd);

        if(pipe(p1p2)<0)
        {
            perror("errore pipe\n");
            exit(2);
        }
        if((pid1=fork())<0)
        {
            perror("errore fork\n");
            exit(3);
        }
        if(pid1==0)
        {
            signal(SIGINT,SIG_DFL);
            close(p1p2[0]);
            close(1);
            dup(p1p2[1]);
            close(p1p2[1]);

            execlp("grep","grep",data,path,NULL);
            perror("errore execl grep\n");
            exit(4);
        }

        close(p1p2[1]);

        if(pipe(p2p3)<0)
        {
            perror("errore pipe\n");
            exit(5);
        }

        if((pid2=fork())<0)
        {
            perror("errore fork\n");
            exit(6);
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

            execlp("sort","sort","-n",NULL);
            perror("errore execl sort\n");
            exit(7);
        }

        close(p1p2[0]);
        close(p2p3[1]);

        if(pipe(p3p0)<0)
        {
            perror("errore pipe\n");
            exit(8);
        }

        if((pid3=fork())<0)
        {
            perror("errore fork\n");
            exit(9);
        }

        if(pid3==0)
        {
            signal(SIGINT,SIG_DFL);
            close(0);
            dup(p2p3[0]);
            close(p2p3[0]);

            close(p3p0[0]);
            close(1);
            dup(p3p0[1]);
            close(p3p0[1]);

            char buffer[MAX];
            sprintf(buffer,"%d",risultati);
            execlp("head","head","-n",buffer,NULL);
            perror("errore execl head\n");
            exit(10);
        }

        close(p2p3[0]);
        close(p3p0[1]);

       while ((nread = read(p3p0[0], buffer, MAX)) > 0) {
            buffer[nread] = '\0'; // Aggiungi il terminatore di stringa
            printf("%s", buffer); // Usa printf invece di write
            fflush(stdout);
        }
        close(p3p0[0]);

        wait(NULL);
        wait(NULL);
        wait(NULL);

        count++;

    }
    return 0;
}