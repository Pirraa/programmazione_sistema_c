#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>

/* lunghezza massima delle stringhe in input/output */
#define SEARCH_STR 100
#define ANS_STR 20

int main(int argc, char** argv) {
	
	/* definizione di un nuovo tipo "pipe" per gestire più
	 * facilmente l'allocazione dinamica della memoria.
	 * NB: scope non globale, ma limitato al main. */
	typedef int pipe_t[2];
	
	int i, j, n_ret, pid, status;
	char nome[SEARCH_STR], ret[ANS_STR];
	
	/*
	 * la consegna prevede che il processo P0 
	 * comunichi il nome della materia prima a ciascun processo Pi (figlio).
	 * pipes_in padre->figlo 
	 * pipes_out figlo->padre
	 */
	pipe_t *pipes_in, *pipes_out;
	
	/* controllo che i parametri non siano troppo pochi */
	if (argc < 2) {
		fprintf(stderr, "Uso: controlla_disponibilita magazzino-1 ... magazzino-N\n");
		exit(1);
	}
	
	/* controllo path relativi */
	for (i = 1; i < argc; ++i) {
		if (argv[i][0] == '/') {
			fprintf(stderr, "%s non e' un path relativo\n", argv[i]);
			exit(1);
		}
	}
	
	/* Alloco la memoria per ospitare le pipe.
	 * Un'alternativa consiste nel preallocare un numero massimo,
	 * predefinito, di int[2] nello stack del processo.
	 * Con pipes_in il padre comunica l'input inserito 
	 * dall'utente ai processi figli.
	 */
	pipes_in = (pipe_t *) malloc((argc - 1) * sizeof(pipe_t));
	pipes_out = (pipe_t *) malloc((argc - 1) * sizeof(pipe_t));
	
	/* Genero i figli, che non termineranno tra richieste successive
	 * ma continueranno a leggere l'input inviato dal processo padre
	 * sulla pipe (come da consegna). Il  conteggio sul magazzino 
	 * i-esimo verra' eseguito da un processo nipote generato dal 
	 * processo figlio. */
	for (i = 0; i < (argc - 1); ++i) {
		
		/* Genero le pipe per comunicare con i figli;
		 * ho bisogno esattamente di (argc - 1) pipe e figli */
		if (pipe(pipes_in[i]) < 0) {
			perror("pipes_in");
			exit(2);
		}
		if (pipe(pipes_out[i]) < 0) {
			perror("pipes_out");
			exit(3);
		}
		
		if ((pid = fork()) < 0) {
			perror("fork padre");
			exit(4);
		}
		if (pid == 0) { // Figlio Fi
			/* chiudo le pipe non utilizzate: quelle create dal padre
			 * per i figli precedenti e ancora aperte e quelle create
			 * per il figlio i-esimo, ma al quale non servono.
			 * gli estremi pipes_in[x][0] per x da 0 a i-1 sono
			 * chiusi dal padre dopo avere creato l'x-esimo figlio
			 * e non sono quindi presenti nel figlio i-esimo */
			for (j = 0; j <= i; ++j) {
				close(pipes_in[j][1]);
				close(pipes_out[j][0]);
			}
			
			/* sfrutto il fatto che la read su pipe ritorni con valore 0
			 * quando tutti i processi che la condividono chiudono
			 * il loro estremo in scrittura per quella pipe. */
			while (read(pipes_in[i][0], nome, sizeof(nome)) > 0) {
				/* genero un nipote per contare le righe in cui compare
					* l'elemento selezionato e comunicare al processo padre
					* il risultato. 
					*/
				if ((pid=fork()) < 0) {
					perror("fork figlio");
					exit(3);
				} 
				if (pid == 0) { // Nipote Ni
					/* chiudo la pipe in lettura dal padre, che al nipote non serve */
					close(pipes_in[i][0]);
					
					/* redireziono lo stdout sulla pipe diretta al padre */
					close(1);
					dup(pipes_out[i][1]);
					close(pipes_out[i][1]);
					
					execlp("grep", "grep", "-c", nome, argv[i+1], (char*)0);
					perror("exec nipote");
					exit(4);
				}
				
				/* Figlio Fi: aspetto la terminazione del nipote */
				wait(&status);
			} // fine while figlio Fi
			
			/* Figlio Fi: chiudo le pipe e termino */
			printf("Fine figlio %d\n",i);
			close(pipes_in[i][0]);
			close(pipes_out[i][1]);
			
			free(pipes_in);
			free(pipes_out);
			
			exit(0);
		} // fine figlio Fi
		
		/* chiudo estremità delle pipe non usate dal padre */
		close(pipes_in[i][0]);
		close(pipes_out[i][1]);
	} // fine for padre
	
	/* Ciclo principale del padre: si interfaccia con l'utente per
	 * leggere il nome dell'oggetto da cercare e lo manda
	 * a tutti i figli per la ricerca */
	printf("\nInserisci il nome dell'oggetto da cercare:\n");
	scanf("%s", nome);
	while ( strcmp(nome, "fine") != 0 ) {
		/* passo il nome da cercare a tutti i figli */
		for (i = 0; i < (argc - 1); ++i) {
			/* ATTENZIONE: si deve inviare anche il carattere di terminazione stringa */
			if (write(pipes_in[i][1], nome, strlen(nome) + 1) < 0){
				perror("write");
				exit(5);
			}
		}
		
		/* leggo le risposte dai figli */
		for (i = 0; i < (argc - 1); ++i) {
			/* reset della memoria ad ogni ciclo, per evitare output sporchi */
			memset(ret, 0, sizeof(ret));
			/* lettura da pipe i-esima */
			if ((n_ret = read(pipes_out[i][0], ret, sizeof(ret))) < 0){
				perror("read");
				exit(5);
			}
			printf("Nel file %s ci sono %d unità di stoccaggio per la materia prima %s\n",
				   argv[i+1], atoi(ret), nome);
		}
		printf("\nInserisci il nome dell'oggetto da cercare:\n");
		scanf("%s", nome);
	}
	
	printf("Sono il padre e ho ricevuto \"fine\"\n");
	
    /* faccio terminare i figli chiudendo gli estremi in scrittura
     * sulle pipe pipes_in, quindi chiudo le altre pipe e pulisco
     * la memoria aspettando tutti gli zombie.
     * E' FONDAMENTALE chiudere l'estremo in scrittura di
     * pipes_in PRIMA di effettuare la chiamata a wait(),
     * per evitare DEADLOCK. */
	for (i = 0; i < (argc - 1); ++i){
		printf("Sono il padre e chiudo pipes_in[%d][1] e pipes_in[%d][1]\n",i,i);
		close(pipes_in[i][1]);
		close(pipes_out[i][0]);
		wait(&status);
	}
	
	/* Deallocazione memoria */
	free(pipes_in);
	free(pipes_out);
	
	return 0;
}
