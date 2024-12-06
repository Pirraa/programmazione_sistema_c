#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>

//grep matricola file | wc -l
int main(int argc,char *argv[])
{
    char matricola[10];
    int pid,status;
    int p1p2[2];
    //controllo argomenti
    if(argc!=2)
    {
        fprintf(stderr,"Errore argomenti");
        exit(1);
    }

    if(argv[1][0]!='/')
    {
        fprintf(stderr,"Errore: il file deve essere assoluto\n");
        exit(2);
    }

    if(open(argv[1],O_RDONLY)<0)
    {
        //in generale uso perror per errori di sistema e fprintf per errori di programmazione
        fprintf(stderr,"Errore apertura file\n");
        exit(3);
    }

    
    /*più corretto fare così
    int fd;
    metto doppie parentesi perchè la condizione < è prioritaria rispetto all'assegnamento
    quindi senza parentesi assegnoa fd o 1 (cioè se va a buon fine il confronto quindi vero) o 0 (cioè se il confronto non va a buon fine quindi è falso)
    se sbaglio questo non posso fare scanf perchè stdin rimane chiuso
    if((fd=open(argv[1],O_RDONLY))<0)
    {
        perror("Errore apertura file");
        exit(1);
    }
    close(fd);*/

    printf("Inserisci numero matricola: ");
    scanf("%s",matricola);
    
    while(strcmp(matricola,"fine")!=0)
    {
        if(pipe(p1p2)==-1)//errore anche con <1
        {
            perror("pipe");
            exit(4);
        }
        //qui ci sta controllo if((pid=fork())<0) perror("fork");
        pid=fork();
        if(pid==0)
        {
            close(p1p2[0]);
            close(1);
            dup(p1p2[1]);
            close(p1p2[1]);
            execl("/bin/grep","grep",matricola,argv[1],(char *)0);
            perror("Errore execl");
            exit(5);
        }
        //posso anche mettere due variabili intere diverse per i due processi figli
        pid=fork();
        if(pid==0)
        {
            close(p1p2[1]);
            close(0);
            dup(p1p2[0]);
            close(p1p2[0]);
            execl("/usr/bin/wc","wc","-l",(char *)0);
            perror("Errore execl");
            exit(6);
        }
        close(p1p2[0]);
        close(p1p2[1]);
        wait(&status);
        wait(&status);

        printf("Inserisci numero matricola: ");
        scanf("%s",matricola);
    }
}