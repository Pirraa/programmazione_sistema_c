Librerie necessarie
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/types.h>

Definizione di variabile globale
static volatile sig_atomic_t count = 0;

Firma del main
int main(int argc, char **argv);

Controllo numero argomenti
if (argc != 2) {
	fprintf(stderr, "Errore\n");
	exit(1);
}

Apertura / Creazione file
// O_WRONLY     - Solo scrittura
// O_TRUNC      - Se il file esiste già il contenuto viene cancellato quando viene aperto
// O_CREAT      - Crea il file se non esistente, necessita di permessi
// O_APPEND     - Appenda cose ad un file
// O_DIRECTORY  - Se il pathname non è una directory crea errore
// O_PATH       - Ottiene il descrittore del file che contiene la location nel file system del file
// O_TMPFILE    - Crea un file temporaneo

// rwx      rwx     rwx
// user     group   others
int fd = open(argv[1], O_WRONLY | O_TRUNC | O_CREAT, 0644);
if (fd < 0) {
	fprintf(stderr, "Error creating file %s\n", argv[1]);
	exit(2);
}

Prendere in input da stdin
char buff[1024];
printf("Inserisci una stringa: \n");
fgets(buff, 1024, stdin);

Scrivere un testo su file fino al raggiungimento della parola 'fine'
// se uso fgets avro' in buff anche il carattere di new line
// se uso scanf faccio il controllo solo con "fine" (senza \n) 
while (strcmp(buff, "fine\n") != 0) {
	write(fd, buff, strlen(buff));
	printf("Inserisci un'altra stringa (fine per uscire):\n");
	fgets(buff, 1024, stdin);
}

Ricordasi di chiudere il file descriptor
close(fd);

Creazione di un figlio e distinzione dal padre
int pid = fork();
if ( pid < 0) {
	perror("fork");
	exit(3);
} else if (pid == 0) {
	/* Figlio */
	char log[256];
			
	/* apro il file in append */
	fd = open("conteggio.txt", O_WRONLY | O_APPEND);
			
	/* creo il log dell'operazione di conteggio */
	sprintf(log, "%s %s\n", argv[1], argv[i]);
			
	/* scrivo il log su file */
	write(fd, log, strlen(log));
			
	/* chiudo il file */
	close(fd);

	printf("\nNum di righe in cui compare la stringa %s:\n", argv[i]);
	/* chiamata a: grep -c stringa nomefile */
	execlp("grep","grep","-c", argv[i], argv[1], (char *)0);
			
	perror("exec");
	exit(4);
}

Attesa del padre che i figli finiscano
for(i = 0; i < argc; i++) {
	/* Padre - Attendo la terminazione di uno dei figli */
	wait(&status);
}

Numero positivo maggiore di zero
/* atoi non rileva errori di conversione, 
		meglio usare strtol() / strtoul() */
if ( (num = atoi(argv[1])) <= 0 ) {
	printf("Errore: %s deve essere un intero positivo maggiore di zero\n", argv[1]);
	exit(2);
}

Creazione pipe
int p2p3[2];
if (pipe(p2p3) < 0) {
	perror("P0: pipe p2p3");
	exit(7);
}

Fork
int pid1;
pid1 = fork();
if (pid1 < 0) {
	perror("P0: fork P1");
	exit(5);
}

Signal Interrupt - ctrl+c
signal(SIGINT, handler_sigint);

Ignoro la SIGINT
signal(SIGINT, SIG_DFL);

Imposto una Signal
signal(SIGUSR1, handler_sigusr1);
handler_sigusr1 è una funzione fatta dall utente

Lancio Signal
// invio segnale SIGUSR1 a P1
sleep(1);
kill(pid1, SIGUSR1);

Aspetto un Signal
pause();

Leggo da una pipe
char buffer[100];
while ((n_read = read(p3p0[0], buffer, 100)) > 0) {
	buffer[n_read] = '\0';
	printf("%s", buffer);
}

Controllo Path assoluto
if (argv[1][0] != '/') {
    printf("errore: <dir> deve essere un path assoluto\n");
    exit(2);
}

Controllo che una cartella esista
if ( (fd=opendir(argv[1])) == NULL) {
    printf("errore: <dir> non esiste\n");
    exit(3);
}
close(fd);

Creo un percorso per accedere al file avendo il nome del file e la directory assoluta
sprintf(path, "%s/%s.txt", argv[1], id_utente);

Controllo che esista un file
fd = open(path, O_RDONLY);
if (fd < 0) {
    printf("errore: il file %s non esiste\n", path);
    continue;
}
close(fd);

Redirezione stdin
// redirigo stdin
close(0);
dup(p1p2[0]);
close(p1p2[0]);

Redirezione stdout
// redirigo stdout
close(1);
dup(p2p3[1]);
close(p2p3[1]);

Utilizzo execlp
execlp("tail", "tail", "-n", n_str, NULL);
perror("P3: execlp");
exit(12);