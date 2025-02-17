# **UNSTABLE STUDENTS - PR1 2024/25**

>  Questo è un progetto svolto nell'ambito dell'esame di **Programmazione 1 - Modulo II** *(PR1)* del **CdL in Informatica** dell'**Università degli Studi di Cagliari**

| **Studente**          | **Matricola** | **Progetto** | **E-Mail**                        |
|-----------------------|---------------|--------------|-----------------------------------|
| Diego Oliva           | **REDACTED**  |   AVANZATO   | **REDACTED**                      |

<br>

---

<br>

> ### **Table of Content**
>  1. [Spiegazione file sorgente](#spiega-file-sorgente)
>  1. [Descrizione e scopo strutture aggiuntive](#descrizione-e-scopo-strutture-aggiuntive)
>  1. [Descrizione flusso di gioco](#descrizione-flusso-di-gioco)
>  1. [Descrizione logica AI](#descrizione-logica-ai)
>      - [Scelta carte](#scelta-carte)
>      - [Strategia di gioco](#strategia-di-gioco)

<br>

```
REPOSITORY STRUCTURE
·
│
│ GAME SAVES
├── saves                       // directory contenente i salvataggi
│   ├── savegame.sav
│   └── ···
│
│ SOURCE FILES
├── src                      	// directory contenente l'intero source code del progetto
│   ├── main.c
│   ├── constants.h
│   ├── structs.h
│   ├── enums.c
│   ├── enums.h
│   ├── types.h
│   ├── card.c
│   ├── card.h
│   ├── game.c
│   ├── game.h
│   ├── gameplay.c
│   ├── gameplay.h
│   ├── files.c
│   ├── files.h
│   ├── format.c
│   ├── format.h
│   ├── graphics.c
│   ├── graphics.h
│   ├── logging.c
│   ├── logging.h
│   ├── utils.c
│   └── utils.h
│
│ OTHER FILES
├── README.md                   //relazione e documentazione
└── .gitignore                  //file da ignorare
```

<br>
<br>

## Spiegazione file sorgente
> [!TIP]
> Per ogni file sorgente (.c/.h) presente nel progetto, spiegare brevemente il contenuto e lo scopo del file.

### main.c
Questo file sorgente contiene l'entry point del programma, ovvero la funzione `main`, nella quale avviene l'inizializzazione del gioco e il game loop.

### constants.h
Questo header non ha un corrispettivo file sorgente .c associato in quanto contiene solamente le definizioni delle costanti (es. numero massimo e minimo di giocatori e lunghezze massime di alcune stringhe) e alcuni letterali usati nel gioco (es. nomi statici dei file coi quali interagisce il programma e stringhe utilizzate nella realizzazione della grafica su terminale).

### structs.h
In questo header sono definite tutte le strutture utilizzate nel progetto (descritte nella seguente sezione). Una scelta un po' particolare inserirle tutte nello stesso file anzi che in appositi file come card.h, player.h e simili ma ho trovato questa soluzione meno confusionaria e meno dispersiva.

### enums.c & enums.h
Seguendo la stessa logica della della scelta adottata per structs.h, nel file header sono definite tutte le enumerazioni usate nel progetto.
Nel file .c sono invece definite le funzioni per effettuare le conversioni da ogni valore di ciascun enum alle corrispondenti stringhe, effettuando le associazioni tramite un metodo che ho trovato molto pulito e ottimale.

### game.c & game.h
Questi file sorgente contengono delle funzioni essenziali per l'inizializzazione e la terminazione del gioco, ma non necessarie durante il suo dinamico svolgimento.

### gameplay.c & gameplay.h
Questi file sorgente contengono praticamente l'intera logica di gioco e le principali funzioni per la gestione della partita in corso.


<br>

## Descrizione e scopo strutture aggiuntive
> [!TIP]
> Descrivere le strutture dati aggiuntive utilizzate nel progetto e perché vi sono tornate utili (se presenti).

### GameContext
La struttura aggiuntiva principale che mi è stata molto utile nel mantenere gestibile il passaggio di parametri fra le varie funzioni è stata GameContext, infatti essa viene passata a quasi tutte le funzioni inerenti alla gestione della partita.
Ecco la struttura in questione:
```c
struct GameContext {
	giocatoreT* curr_player;
	cartaT *mazzo_pesca, *mazzo_scarti, *aula_studio;
	int n_players, round_num;
	bool game_running;
	FILE *log_file;
};
typedef struct GameContext game_contextT;
```

Mi è sufficiente utilizzare tale struttura per contenere l'intero stato della partita (all'inizio di un round, non durante), come si può notare dai prototipi delle funzioni usate per caricare e salvare i salvataggi:

```c
game_contextT* load_game();
void save_game(game_contextT* game_ctx);
```

Ciò che contiene questa struttura è:
- un puntatore al giocatore che deve giocare (o che sta giocando) questo turno, che viene fatto avanzare con comodità grazie alla circolarità della lista dei giocatori alla quale appartiene.
- un puntatore alla testa ciascuna lista concatenata di carte: pesca, scarti e aula studio.
- un intero rappresentante la quantità di giocatori che stanno partecipando alla partita.
- un intero rappresentante il numero del round al quale lo stato della partita si trova.
- un booleano rappresentante che il gioco è in esecuzione (o in conclusione, solo quando un giocatore vince e la partita termina, oppure si esce dalla partita con il tasto 0 del menu di gioco).
- un puntatore a FILE (file stream) relativo al file di log, aperto prima di iniziare a giocare e chiuso quando si esce dal gioco.

L'utilizzo che faccio di questa struttura è semplice e lineare: la alloco sullo heap all'avvio del gioco (tramite le funzioni `new_game` o `load_game`) e ne passo il puntatore alle diverse funzioni del game-loop (`begin_round`, `play_round`, `end_round`) che la passeranno a loro volta ad altre funzioni che implementano la logica di gioco; alla fine dell'esecuzione del gioco (uscita dal game-loop) la rilascio assieme a tutti i suoi campi (tramite `clear_game`).

<br>

## Descrizione flusso di gioco
> [!TIP]
> Descrivere ad alto livello come vengono gestite le varie fasi di gioco (es. il loop principale, la gestione degli eventi, ecc.)

In questa sezione del `README.md` e nel codice (variabili e commenti) faccio riferimento a:
- utente [user]: chi interagisce col gioco sul terminale.
- giocatore corrente [curr(ent) player]: il giocatore a cui spetta giocare nell'attuale round.
- giocatore target: il giocatore che dovrà subire degli effetti (può essere il giocatore corrente).
- carte giocabili: le carte che è permesso giocare al giocatore corrente.
- carte duplicate: delle copie identiche di una stessa carta.
- lunghezza visibile [visible length]: la lunghezza visiva di una stringa una volta stampata sul terminale, non considerando gli escape ANSI per i colori e la formattazione (quindi non si tratta sempre della sua lunghezza in bytes in memoria).

...


...


### Carte giocabili
Le carte giocabili vengono contate tramite la funzione `count_playable_cards`, che considera vari casi, di seguito spiegati:
- Solo carte del tipo specificato (type) possono essere giocate: supponendo di star giocando a seguito di un effetto come `[GIOCA, IO, STUDENTE]` sarebbe possibile giocare solamente una carta STUDENTE (`MATRICOLA`, `STUDENTE_SEMPLICE` o `LAUREANDO`) dal proprio mazzo, mentre con type = `ALL` questo controllo viene sempre passato.
- Le carte di tipo `ISTANTANEA` non possono essere giocate durante il proprio turno.
- Le carte con tipo target di un effetto IMPEDIRE attivo sul giocatore non possono essere giocate.

### Giocare una carta
La funzione `play_card`, che si occupa per l'appunto di permettere al giocatore corrente di scegliere una carta del suo mazzo da giocare.\
Se non ci sono carte giocabili secondo quanto descritto in [Carte Giocabili](#carte-giocabili) la funzione termina senza chiedere alcuna interazione all'utente in quanto non è possibile giocare alcuna carta.\
...\
In caso di carta duplicata già presente nell'aula del target chiedo conferma di voler giocare comunque tale carta sul target all'utente e in caso positivo la carta andrà persa (scartata) e l'azione di play si conclude, mentre in caso negativo l'utente potrà scegliere una nuova carta o un nuovo target su cui giocarla (l'azione di play non si considera conclusa).


TODO: scarta io choice

...

Le carte Bonus/Malus possono essere giocate sia su sé stessi che sugli altri giocatori, mentre le carte STUDENTE solo su sé stessi.



<br>

## Descrizione logica AI
> [!NOTE]
> Questa sezione va compilata solo se si sta svolgendo il progetto AVANZATO.

### Scelta carte...
...
### Strategia di gioco...
...
