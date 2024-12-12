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
#include <bits/fcntl-linux.h>
#include <asm-generic/fcntl.h>

#define DIM 100
#define DIM_TOT 200

static volatile sig_atomic_t count = 0;

void handler() 
{ printf("Numero richieste: %d\n", count); exit(0); }

int main(int argc, char **argv)
{
    // Controlli generici
    if (argc != 2) 
    {
        fprintf(stderr, "Errore: numero di argomenti sbagliato.\n");
        exit(0);
    }

    if (argv[1][0] != '/') 
    {
        fprintf(stderr, "Errore: la directory deve essere assoluta.\n");
        exit(1);
    }

    int file_descriptor;

    if ((file_descriptor = open(argv[1], O_DIRECTORY, 0777)) < 0)
    {
        fprintf(stderr, "Errore: la directory non è accessibile.\n");
        exit(2);
    }
    close(file_descriptor);

    // Dichiarazioni variabili necessarie
    char cliente[DIM], giorno[DIM], nome_file[DIM_TOT];
    int n_risultati, pid1, pid2, pid3, status;
    int p1p2[2], p2p3[2];

    // Interruzione tramite segnale
    signal(SIGINT, handler);

    // Corpo del programma
    while (1)
    {
        // Input dell'utente
        printf("Inserisci il codice del cliente:\n");
        scanf("%s", cliente);

        printf("Inserisci il giorno:\n");
        scanf("%s", giorno);

        printf("Inserisci il numero di risultati da mostrare:\n");
        scanf("%d", &n_risultati);

        // Controllo sugli input
        if (n_risultati <= 0)
        {
            fprintf(stderr, "Errore: il numero di risultati da mostrare deve essere maggiore di 0\n");
            exit(3);
        }

        // Unisco il percorso del file
        sprintf(nome_file, "%s/%s.tran", argv[1], giorno);

        // Controllo esistenza del file
        int file_descriptor;

        if ((file_descriptor = open(nome_file, O_RDONLY, 0777)) < 0) 
        {
            fprintf(stderr, "Errore: apertura file non riuscita.\n");
            continue;
        }

        close(file_descriptor);

        // Nascita primo figlio
        if (pipe(p1p2) < 0) 
        {
            fprintf(stderr, "Errore: creazione della prima pipe non riuscita.\n");
            exit(3);
        }

        pid1 = fork();
        if (pid1 < 0) 
        {
            fprintf(stderr, "Errore: creazione del primo figlio.\n");
            exit(3);
        }

        if (pid1 == 0) 
        {
            // Ignora SIGINT
            signal(SIGINT, SIG_DFL);

            // Codice del primo figlio
            close(p1p2[0]);

            // Redirezione stdout
            close(1);
            dup(p1p2[1]);
            close(p1p2[1]);

            // Esecuzione compito
            execlp("grep", "grep", cliente, nome_file, NULL);

            fprintf(stderr, "Errore: esecuzione grep non riuscita.\n");
            exit(4);
        }

        close(p1p2[1]);

        // Nascita secondo figlio
        if (pipe(p2p3) < 0)
        {
            fprintf(stderr, "Errore: creazione della seconda pipe non riuscita.\n");
            exit(3);
        }

        pid2 = fork();
        if (pid2 < 0) 
        {
            fprintf(stderr, "Errore: creazione del secondo figlio.\n");
            exit(5);
        }

        if (pid2 == 0) 
        {
            // Ignora SIGINT
            signal(SIGINT, SIG_DFL);

            // Codice del secondo figlio

            // Redirezione stdin
            close(0);
            dup(p1p2[0]);
            close(p1p2[0]);

            // Redirezione stdout
            close(1);
            dup(p2p3[1]);
            close(p2p3[1]);
            close(p2p3[0]);

            // Esecuzione compito
            execlp("sort", "sort", "-n", NULL);

            fprintf(stderr, "Errore: esecuzione grep non riuscita.\n");
            exit(6);
        }

        close(p1p2[0]);
        close(p2p3[1]);

        // Nascita terzo figlio
        pid3 = fork();
        if (pid3 < 0) 
        {
            fprintf(stderr, "Errore: creazione del terzo figlio.\n");
            exit(7);
        }

        if (pid3 == 0) 
        {
            // Ignora SIGINT
            signal(SIGINT, SIG_DFL);

            // Codice del terzo figlio

            // Redirezione stdin
            close(0);
            dup(p2p3[0]);
            close(p2p3[0]);
            
            int importi_file_descriptor = open("importi_massimi.txt", O_WRONLY | O_TRUNC | O_CREAT, 0777);
            if (importi_file_descriptor < 0)
            {
                fprintf(stderr, "Errore: file importi massimi non creabile.\n");
                exit(8);
            }
            
            // Redirezione stdout
            close(1);
            dup(importi_file_descriptor);
            close(importi_file_descriptor);

            char stringa[100];
            sprintf(stringa, "%d", n_risultati);

            // Esecuzione compito
            execlp("head", "head", "-n", stringa, NULL);

            fprintf(stderr, "Errore: esecuzione tail non riuscita.\n");
            exit(9);
        }

        close(p2p3[0]);

        wait(&status);
        wait(&status);
        wait(&status);

        count++;
    }
    
    return 0;
}