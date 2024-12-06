#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/wait.h>

/*
read con valore di ritorno byte letti e aggiunta terminatore per stampare
open con o_rdonly per controllare se esiste file
sprintf per convertire intero in stringa da passare al comando come flag
opendir come open ma per le cartelle
static volatile sig_atomic per variabile condivisa fra tutti i processi e tutte le funzioni (main e handler segnali) globale (contatore di richieste)
signal con funzione per gestire e con sig_dfl per ignorare
kill per inviare segnali
pause per fare si che processo si fermi parta quando gli arriva segnale (corpo della funzione di quel segnale anche vuoto)
tail -n 1 per prendere ultima riga di un file
sort -n per ordinare sulla base della data prima parte della riga del file
*/

static volatile sig_atomic_t count = 0;

void handler_sigusr1() {}

void handler_sigint() {
	printf("numero di richieste: %d\n", count);
	exit(0);
}

int main(int argc, char *argv[]) {
    char id_utente[10], path[100], buffer[100];
    int fd, n, pid1, pid2, pid3, p1p2[2], p2p3[2], p3p0[2], n_read;

    // controllo che il numero di parametri sia corretto
    if (argc != 2) {
        printf("errore: numero sbagliato di parametri\n");
        exit(1);
    }

    // controllo che argv[1] sia un path assoluto
    if (argv[1][0] != '/') {
        printf("errore: <dir> deve essere un path assoluto\n");
        exit(2);
    }

    // controllo che argv[1] esista
    if ( (fd=opendir(argv[1])) == NULL) {
        printf("errore: <dir> non esiste\n");
        exit(3);
    }
	close(fd);

	// imposto la gestione di SIGINT
	signal(SIGINT, handler_sigint);

    while (1) {
        printf("inserire id utente: ");
        scanf("%s", id_utente);

        // controllo che <dir>/<id_utente>.txt esista
        sprintf(path, "%s/%s.txt", argv[1], id_utente);
        fd = open(path, O_RDONLY);
        if (fd < 0) {
            printf("errore: il file %s non esiste\n", path);
            continue;
        }
		close(fd);

        printf("inserire numero di risultati da visualizzare: ");
        scanf("%d", &n);

        // controllo che n sia un intero positivo
        if (n <= 0) {
            printf("errore: %d non è un intero positivo\n", n);
            continue;
        }

        // creo pipe per comunicazione tra P1 e P2
		if (pipe(p1p2) < 0) {
			perror("P0: pipe p1p2");
			exit(4);
		}

		// creo P1
		pid1 = fork();
		if (pid1 < 0) {
			perror("P0: fork P1");
			exit(5);
		}
		if (pid1 == 0) {
			// imposto la gestione di SIGINT
			signal(SIGINT, SIG_DFL);

            // imposto la gestione di SIGUSR1
            signal(SIGUSR1, handler_sigusr1);

			// aspetto segnale SIGUSR1
            pause();

			// chiudo pipe non necessarie
			close(p1p2[0]);

			// redirigo stdout
			close(1);
			dup(p1p2[1]);
			close(p1p2[1]);

			// ordino le righe in modo cronologico
			execlp("sort", "sort", "-n", path, NULL);
			perror("P1: execlp");
			exit(6);
		}

        // creo pipe per comunicazione tra P2 e P3
		if (pipe(p2p3) < 0) {
			perror("P0: pipe p2p3");
			exit(7);
		}

		// creo P2
		pid2 = fork();
		if (pid2 < 0) {
			perror("P0: fork P2");
			exit(8);
		}
		if (pid2 == 0) {
			// imposto la gestione di SIGINT
			signal(SIGINT, SIG_DFL);

			// chiudo pipe non necessarie
			close(p1p2[1]);
			close(p2p3[0]);

			// redirigo stdin
			close(0);
			dup(p1p2[0]);
			close(p1p2[0]);

			// redirigo stdout
			close(1);
			dup(p2p3[1]);
			close(p2p3[1]);

			// ordino le righe per durata
			execlp("grep", "grep", "NON RESTITUITO", NULL);
			perror("P2: execlp");
			exit(9);
		}

		// chiudo pipe non necessarie
		close(p1p2[0]);
		close(p1p2[1]);

		// creo pipe per comunicazione tra P3 e P0
		if (pipe(p3p0) < 0) {
			perror("P0: pipe p3p0");
			exit(10);
		}

        // creo P3
		pid3 = fork();
		if (pid3 < 0) {
			perror("P0: fork P3");
			exit(11);
		}
		if (pid3 == 0) {
			char n_str[10];

			// imposto la gestione di SIGINT
			signal(SIGINT, SIG_DFL);

			// chiudo pipe non necessarie
			close(p2p3[1]);
			close(p3p0[0]);

			// redirigo stdin
			close(0);
			dup(p2p3[0]);
			close(p2p3[0]);

			// redirigo stdout
			close(1);
			dup(p3p0[1]);
			close(p3p0[1]);

			// visualizzo gli utlimi <n> risultati
			sprintf(n_str, "%d", n);
			execlp("tail", "tail", "-n", n_str, NULL);
			perror("P3: execlp");
			exit(12);
		}

		// chiudo pipe non necessarie
		close(p2p3[0]);
		close(p2p3[1]);
		close(p3p0[1]);

        // invio segnale SIGUSR1 a P1
		sleep(1);
        kill(pid1, SIGUSR1);

		// aspetto terminazione figli
		wait(NULL);
		wait(NULL);
		wait(NULL);

		// leggo risultati da P3
		while ((n_read = read(p3p0[0], buffer, 100)) > 0) {
			buffer[n_read] = '\0';
			printf("%s", buffer);
		}

		// chiudo pipe non necessarie
		close(p3p0[0]);

		count++;
    }

    return 0;    
}