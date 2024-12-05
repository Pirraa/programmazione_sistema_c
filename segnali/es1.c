#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>

//non ottimizzare l'accesso, così l'accesso sarà sempre consistente sia che sia acceduta dal corpo principale del processo sia in modo asincrono da una procedura che il processo esegue a valle di una sigaction
static volatile sig_atomic_t counter=0;

void handler(int signal)
{
    printf("Processo figlio %d ha compiuto %d iterazioni\n", getpid(), counter);
    exit(0);
}

int main(int argc,char *argv[])
{
    if(argc!=3)
    {
        printf("Errore argomenti\n");
        exit(1);
    }

    int Nf=atoi(argv[1]);
    int Nsec=atoi(argv[2]);

    for(int i=0; i<Nf; i++)
    {
        int pid=fork();
        if(pid==0)
        {
            //meglio usare la sigaction
            signal(SIGUSR1,handler);
            while(1)
            {
                sleep(1);
                counter++;
            }
        }
    }

    sleep(Nsec);
    //se faccio così devo dire al padre di ignorar sigusr1, altrimenti viene killato anche lui e non fa le wait
    //per fare ignorare i seganali al padre 
    //signal(SIGUSR1,SIG_IGN);
    kill(0,SIGUSR1);

    for (int i = 0; i < Nf; i++) {
        wait(NULL);
    }

    return 0;
}