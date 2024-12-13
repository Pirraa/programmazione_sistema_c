#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

#define MAX 100

static volatile sig_atomic_t count=0;

void handler(){
    printf("Numero richieste servite: %d\n",count);
    exit(0);
}

int main(int argc,char* argv[])
{
    int fd,pid1,pid2,p1p2[2],p2p0[2],num_vm;
    char path[MAX*2];
    char fornitore[MAX],applicazione[MAX];
    //dir contiene file
    //nome file nome produttore
    //contenuto file nome vm,tipo app,tipo vm,stato vm
    //P0 riceve nome fornitore,nome applicazione
    //P1 identifica vm di uno specifico produttore e applicazione (grep applicazione)
    //P2 conta quelle operative (grep -c operative)
    //P0 riceve da P2 e stampa a schermo

    if(argc!=2)
    {
        fprintf(stderr,"errore numero argomenti\n");
        exit(1);
    }

    if(argv[1][0]!='/')
    {
        fprintf(stderr,"errore directory non assoluta\n");
        exit(2);
    }

    if((fd=open(argv[1],O_DIRECTORY))<0)
    {
        fprintf(stderr,"errore directory non esistente o permessi non presenti\n");
        exit(3);
    }

    signal(SIGINT,handler);

    while(1)
    {
        printf("Inserisci il fornitore di Vm: \n");
        scanf("%s",fornitore);

        if(strcmp(fornitore,"fine")==0)
        {
            break;
        }

        printf("Inserisci l'applicazione da ricercare: \n");
        scanf("%s",applicazione);

        if(strcmp(applicazione,"fine")==0)
        {
            break;
        }

        sprintf(path,"%s/%s.txt",argv[1],fornitore);
        if((fd=open(path,O_RDONLY))<0)
        {
            fprintf(stderr,"errore file non esistente o permessi non presenti\n");
            continue;
        }

        if((pipe(p1p2))<0)
        {
            perror("errore pipe\n");
            exit(5);
        }

        if((pid1=fork())<0)
        {
            perror("errore fork\n");
            exit(4);
        }

        if(pid1==0)
        {
            signal(SIGINT,SIG_DFL);
            close(p1p2[0]);
            close(1);
            dup(p1p2[1]);
            close(p1p2[1]);

            execlp("grep","grep",applicazione,path,NULL);
            perror("errore execl\n");
            exit(5);
        }

        close(p1p2[1]);

        if((pipe(p2p0))<0)
        {
            perror("errore pipe\n");
            exit(5);
        }

        if((pid2=fork())<0)
        {
            perror("errore fork\n");
            exit(4);
        }

        if(pid2==0)
        {
            signal(SIGINT,SIG_DFL);
            close(p2p0[0]);
            close(0);
            dup(p1p2[0]);
            close(p1p2[0]);

            close(1);
            dup(p2p0[1]);
            close(p2p0[1]);

            execlp("grep","grep","-c","operativa",NULL);
            perror("errore execl\n");
            exit(5);
        }
        
        close(p2p0[1]);
        close(p1p2[0]);

        int num_letti;
        char numero[5];
        num_letti=read(p2p0[0],numero,sizeof(int));
        close(p2p0[0]);
        numero[num_letti]='\0';
        num_vm=atoi(numero);
        printf("%s %s %d\n",applicazione,fornitore,num_vm);

        wait(NULL);
        wait(NULL);
        count++;

    }
    printf("Numero richieste servite: %d\n",count);
    return 0;
}