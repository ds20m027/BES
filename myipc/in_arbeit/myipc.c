/**
* @file myipc.c
* Betriebssysteme Beispiel 3
* myipc - Datenaustausch mittels Shared Memory und Semaphoren
* per Ringpuffer zwischen Sender und Empfänger
*
* @author Ralf Ziefuhs <ic17b065@technikum-wien.at>
* @author Clemens Fritzsche <ic17b087@technikum-wien.at>
*
* @date 2018/06/15
*
* @version 1.0
*
*/

/*
* -------------------------------------------------------------- includes --
*/
#include "myipc.h"
#include <stdio.h>
#include <sem182.h>
#include <sys/ipc.h>      
#include <sys/shm.h>     
#include <unistd.h>      
#include <sys/types.h>
#include <errno.h>
#include <semaphore.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

/*
* --------------------------------------------------------------- defines --
*/
/*
* -------------------------------------------------------------- typedefs --
*/
/*
* ------------------------------------------------------------- functions --
*/
static long init_ringbuffer(int argc, const char const * argv[]);
static int get_sem(long init_val);
static int rm_sem(int semid);
static int *get_shm(long size, int flags);
static int rm_shm(void);
static void clean_all(int exit_code);

/*
* --------------------------------------------------------------- globals --
*/
static int uid_key = -1;
static int *shmptr = (int *)-1;
static int shmid = -1;
static int r_semid = -1;
static int w_semid = -1;



/**
*
* \brief Programmstart von Sender bzw. Empfänger
*
* Diese Funktion ist für den Datenaustausch mittels Shared Memory
* und Semaphoren per Ringpuffer zwischen Sender und Empfänger zuständig.
*
* \param argc Enthaelt die Anzahl von Argumenten.
*			  (Immer groesser oder gleich 1)
* \param argv Ein Array mit Null-terminierten Zeichenfolgen.
*			  (argv[0] = Befehl des Programmaufrufs)
*
* \return Liefert an das Betriebsystem einen Rueckgabecode,
*		   ob die Ausfuehrung Erfolgreich oder Fehlerhaft war.
* \retval 0 Erfolgreich
* \retval 1 Fehlerhaft
*
*/
int start_ipc(int argc, const char const * argv[], const int type)
{
	long rb_size = init_ringbuffer(argc, argv);
	//return size

	uid_key = 1000 * getuid();
		
	//Semaphoren anlegen
	int r_semid = get_sem(0);
	int w_semid = get_sem(rb_size);

	//SENDER

	long rb_index = 0; /* Ringbuffer Index */
	int zeichen = EOF;
	
	switch (type)
	{
		case TYPE_Sender:

			//Shared Memory anlegen
			shmptr = get_shm(rb_size, 0);
			
			/* Schleife bis EOF geschrieben wurde */
			do
			{
				/* Character aus STDIN einlesen */
				zeichen = fgetc(stdin);

				if (ferror(stdin))
				{                            
					print_error("Fehler bei fgetc");
					clean_all(EXIT_FAILURE);
				}

				/* Down Signal */
				while (P(w_semid) == -1)
				{
					/* Ignoriere Interrupt-Signal */
					if (errno != EINTR)
					{
						/* Fehlerbehandlung fuer P() */
						print_error("Fehler bei P()");
						clean_all(EXIT_FAILURE);
					}
				}

				/* Character in den Shared Memory schreiben */
				shmptr[rb_index++] = zeichen;
				//shmptr[rb_index] = zeichen;
				//rb_index++;

				/* Up Signal */
				if (V(r_semid) == -1)
				{
					/* Fehlerbehandlung fuer V() */
					print_error("Fehler bei V()");
					clean_all(EXIT_FAILURE);
				}

				/* Ringbuffer */
				rb_index %= rb_size;

			} while (zeichen != EOF);
			
			clean_all(EXIT_SUCCESS);
			break;

		case TYPE_EMPFAENGER:
			/* Shared Memory Read-Only */
			shmptr = get_shm(rb_size, SHM_RDONLY);

			/* Schleife bis EOF gelesen wurde */
			do
			{
				/* Down Signal */
				while (P(r_semid) == -1)
				{
					/* Ignoriere Interrupt-Signal */
					if (errno != EINTR)
					{
						/* Fehlerbehandlung fuer P() */
						print_error("P() failed.");
						clean_all(EXIT_FAILURE);
					}
				}

				/* Lese Character aus dem Shared Memory */
				zeichen = shmptr[rb_index++];
				//zeichen = shmptr[rb_index++];
				//rb_index++;

				/* Up Signal */
				if (V(w_semid) == -1)
				{
					/* Fehlerbehandlung fuer V() */
					print_error("V() failed.");
					clean_all(EXIT_FAILURE);
				}

				/* Gelesener Character auf STDOUT ausgeben */
				if (zeichen != EOF)
				{
					if (fputc(zeichen, stdout) == EOF)
					{
						/* Fehlerbehandlung fuer fputc() */
						print_error("fputc() failed.");
						clean_all(EXIT_FAILURE);
					}

				}

				/* Ringbuffer */
				rb_index %= rb_size;

			} while (zeichen != EOF);
						
			clean_all(EXIT_SUCCESS);
			break;
			
		case default:
			fprintf(stderr, "Usage: %s Programmfehler\n", argv[0]);
			exit(EXIT_FAILURE);
			break;

	}
	
}

/**
*
* \brief Ringbuffer Initialisierung
*
* Erstellt die Keys fuer Shared Memory und Semaphoren.
* Definiert den Programmnamen fuer Usage/Errors.
* Liest die Buffergroesse von argv ein.
* Usage Meldung bei falscher Verwendung.
*
* \param argc Enthaelt die Anzahl von Argumenten.
*			  (Immer groesser oder gleich 1)
* \param argv Ein Array mit Null-terminierten Zeichenfolgen.
*			  (argv[0] = Befehl des Programmaufrufs)
*
* \return Die gewuenschte Buffergroesse
*
*/
static long init_ringbuffer(int argc, const char const * argv[])
{
	char *endptr = NULL;
	long size = -1;
	int opt = -1;
		
	/*Fehlende Argumente
	if (argc < 2)
	{
		size = -1;
	}*/

	//Parsen der Argumente
	while ((opt = getopt(argc, argv, "m:")) != -1)
	{
		switch (opt)
		{
			case 'm':
				//Umwandlung der Eingabe in eine Zahl
				size = strtol(optarg, &endptr, 10);

				if ((errno == ERANGE && (size == LONG_MIN || size == LONG_MAX))
					|| (errno != 0 && size == 0)
					|| *endptr != '\0'
					|| size > UINT_MAX 
					|| size > SHMMAX)
				{
					size = -1;
				}
				break;
		
			default: size = -1; break;
		}
	}

	//Fehlende oder zuviele Argumente
	if (argc < 2 || size < 0 || optind < argc )
	{
		fprintf(stderr, "Usage: %s -m <ringbuffer elements>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	
	return size;
}

/**
*
* \brief Get Semaphore
*
* Holt die Semaphore ID und legt sie ggf. an.
*
* \param initval 0|Buffergroesse
*
* \return Semaphore ID
*
* \note Folgende globale Variablen werden verwendet: uid_key \a r_semid \a w_semid
*
*/
static int get_sem(long init_val)
{
	int key = -1;
	//int *semid = &r_semid;
	//Read Semaphore
	if (init_val == 0)
	{
		key = uid_key + 1;
	}
	//Write Semaphore
	if (init_val > 0)
	{		
		//semid = &w_semid;
		key = uid_key + 2;
	}

	//Semaphore wird angelegt
	const int semid = seminit(key, 0660, init_val)
	
	if (semid == -1)
	{
		if (errno == EEXIST)
		{
			semid = semgrab(key);

			if (retval == -1)
			{                            
				print_error("semgrab fehlgeschlagen");
				clean_all(EXIT_FAILURE);
			}
			
		}
		else
		{                                
			print_error("seminit fehlgeschlagen");
			clean_all(EXIT_FAILURE);
		}
	}
	
	return semid;
}

/**
*
* \brief Remove Semaphore
*
* Entfernt die Semaphore mit der angegebenen ID.
*
* \param semid Semaphore ID
*
* \return Ausfuehrungsstatus
* \retval 0 Erfolgreich
* \retval -1 Fehlerhaft
*
*/
static int rm_sem(int semid)
{
	int retval = 0;

	/* Entferne Semaphore */
	if (semrm(semid) == -1)
	{
		print_error("Fehler bei semrm()");
		retval = -1;
	}

	return retval;
}

/**
*
* \brief Get Shared Memory
*
* Holt wenn vorhanden die Shared Memory ID,
*  falls nicht vorhanden wird diese erstellt.
* Bindet die Shared Memory in den virtuellen Adressraum ein.
*
* \param size Groesse des Buffers
* \param flags 0|SHM_RDONLY
*
* \return Pointer zum Shared Memory
*
* \note Folgende globale Variablen werden verwendet: \a shmptr \a shmid
*
*/
static int *get_shm(long size, int flags)
{
	int key = uid_key + 3;

	/* Hole Shared Memory ID bzw. erstelle diese */
	shmid = shmget(key, size * sizeof(int), 0660 | IPC_CREAT | IPC_EXCL);
	if (shmid == -1)
	{
		print_error("Fehler bei shmget()");
		clean_all(EXIT_FAILURE);
	}

	/* Blendet Shared Memory im Prozess ein */
	shmptr = shmat(shmid, NULL, flags);
	if (shmptr == (int *)-1)
	{
		print_error("Fehler bei shmat()");
		clean_all(EXIT_FAILURE);
	}

	return shmptr;
}

/**
*
* \brief Remove Shared Memory
*
* Blendet Shared Memory aus dem virtuellen Adressraum aus.
* Entfernt Shared Memory und gibt sie wieder frei.
*
* \return Ausfuehrungsstatus
* \retval 0 Erfolgreich
* \retval -1 Fehlerhaft
*
* \note Folgende globale Variablen werden verwendet: \a shmptr \a shmid
*
*/
static int rm_shm(void)
{
	int retval = 0;

	/* Blendet Shared Memory im Prozess aus */
	if (shmdt(shmptr) == -1)
	{
		print_error("Fehler bei shmdt()");
		retval = -1;
	}

	/* Entfernt Shared Memory */
	if (shmctl(shmid, IPC_RMID, NULL) == -1)
	{
		print_error("Fehler bei shmctl()");
		retval = -1;
	}

	return retval;
}

/**
*
* \brief Gibt eine Fehlermeldung auf stderr aus.
*
* Diese Funktion gibt eine Fehlermeldung auf stderr aus.
* Falls errno einen Fehlercode beinhaltet,
* wird die dazugehoerige Fehlermeldung auch ausgegeben.
*
* \param msg Die zu ausgebende Fehlermeldung.
*
* \return Kein Rueckgabewert
*
* \note Folgende globale Variablen werden verwendet: \a cmd
*
*/
static void print_error(const char *msg)
{
	/* Fehlercode ist vorhanden, die dazugehoerige Fehlermeldung wird ausgegeben */
	if (errno != 0)
	{
		fprintf(stderr, "%s: %s - %s\n", cmd, msg, strerror(errno));
		errno = 0;
	}
	/* Nur die uebergebene Fehlermeldung wird ausgegeben */
	else
	{
		fprintf(stderr, "%s: %s\n", cmd, msg);
	}
}

/**
*
* \brief Gibt alle Ressourcen frei und beendet.
*
* Diese Funktion gibt alle Ressourcen frei und beendet mit
* dem exit_code.
*
* \return Kein Rueckgabewert
*
* \note Folgende globale Variablen werden verwendet: \a r_semid \a w_semid
*
*/
static void clean_all(int exit_code)
{
	/* Entferne Lese Semaphore falls vorhanden */
	if (r_semid > 0)
	{
		rm_sem(r_semid);
	}

	/* Entferne Schreib Semaphore falls vorhanden */
	if (w_semid > 0)
	{
		rm_sem(w_semid);
	}

	/* Entferne Shared Memory falls vorhanden */
	if (shmid > 0)
	{
		rm_shm();
	}
	
	if (fflush(stdout) == EOF)
	{
		print_error("Fehler bei fflush(stdout)");
	}
	
	exit(exit_code);
}


// =================================================================== eof ==

// Local Variables:
// mode: c
// c-mode: k&r
// c-basic-offset: 8
// indent-tabs-mode: t
// End: