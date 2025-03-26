# **UNSTABLE STUDENTS - PR1 2024/25**

>  Questo è un progetto svolto nell'ambito dell'esame di **Programmazione 1 - Modulo II** *(PR1)* del **CdL in Informatica** dell'**Università degli Studi di Cagliari**

| **Studente**          | **Matricola** | **Progetto** | **E-Mail**                        |
|-----------------------|---------------|--------------|-----------------------------------|
| Diego Oliva           | **REDACTED**  |   AVANZATO   | **REDACTED**                      |

<br>

*Progetto **AVANZATO senza modalità automatica/IA**.*

---

<br>

> ### **Table of Content**
>  1. [Compilare ed eseguire il gioco](#compilare--eseguire-il-gioco)
>  2. [Spiegazione file sorgente](#spiegazione-file-sorgente)
>  3. [Descrizione e scopo strutture aggiuntive](#descrizione-e-scopo-strutture-aggiuntive)
>  4. [Descrizione flusso di gioco](#descrizione-flusso-di-gioco)

<br>

```
REPOSITORY STRUCTURE
·
│
│ GAME SAVES
├── saves						// directory contenente i salvataggi
│   ├── cache.txt				// cache dei nomi dei salvataggi
│   ├── savegame.sav
│   └── ···
│
│ SOURCE FILES
├── src							// directory contenente l'intero source code del progetto
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
│   ├── effects.c
│   ├── effects.h
│   ├── gameplay.c
│   ├── gameplay.h
│   ├── menu.c
│   ├── menu.h
│   ├── stats.c
│   ├── stats.h
│   ├── saves.c
│   ├── saves.h
│   ├── files.c
│   ├── files.h
│   ├── format.c
│   ├── format.h
│   ├── graphics.c
│   ├── graphics.h
│   ├── logging.c
│   ├── logging.h
│   ├── utils.c
│   ├── utils.h
│   ├── debugging.c
│   └── debugging.h
│
│ BUILD DIRECTORY
├── build						// directory contenente il binario compilato e i file oggetto
│   ├── unstable_students		// file eseguibile del gioco
│   └── *.o
│
│ IMAGES
├── imgs						// directory contenente le immagini utilizzate nel README.md
│   ├── card_box.png
│   ├── cards_group.png
│   ├── game_konsole.png
│   └── game_xterm.png
│
│ OTHER FILES
├── README.md					// relazione e documentazione
├── Makefile					// Makefile per la compilazione
├── Specifiche_v2.0.pdf			// specifiche di riferimento per il progetto
├── mazzo.txt					// file contenente l'intero mazzo di gioco (per le nuove partite)
├── log.txt						// file di log del gioco
└── .gitignore					// lista di file da ignorare (per git)
```

<br>
<br>

## Compilare & eseguire il gioco

...

Per gestire la compilazione ho fatto uso di `make`, creando diversi target nel [Makefile](./Makefile):
- `clean`: elimina il binario compilato e tutti i file oggetto creati durante la compilazione
- `run`: compila e avvia il gioco
- `rebuild`: esegue la pulizia (target `clean`) e compila il gioco
- `gdb`: compila e avvia il gioco tramite il debugger `gdb`, utile per individuare punti e cause di crash
- `valgrind`: compila e avvia il gioco tramite il tool `valgrind` per trovare memory leak e corruzzioni della memoria
- `debug`: compila il gioco definendo l'identificatore `DEBUG` per la compilazione condizionale di alcune parti di codice atte a tracciare la gestione della memoria

...

Per avere una corretta visualizzazione della **TUI** (Terminal User Interface) del gioco è necessario eseguirlo su un terminale con almeno `146` colonne, diversamente i gruppi di carte più grandi non verrebbero rappresentati correttamente dato che andrebbero a capo rompendo la formattazione.
> *Per usare il gioco su terminali più "piccoli" (con meno colonne) è sufficiente diminuire la costante `CARDS_PER_ROW` (attualmente impostata a `4`) nel [file delle costanti](src/constants.h).*

Ecco due esempi di come dovrebbe essere visualizzata l'interfaccia del gioco in maniera corretta (su terminali da almeno `146` colonne):

| ![Gioco su terminale Konsole](imgs/game_konsole.png) |
|:--:|
| *TUI del gioco su terminale Konsole* |

| ![Gioco su terminale XTerm](imgs/game_xterm.png) |
|:--:|
| *TUI del gioco su terminale XTerm* |



## Spiegazione file sorgente
> [!TIP]
> Per ogni file sorgente (.c/.h) presente nel progetto, spiegare brevemente il contenuto e lo scopo del file.

Per ciascun file sorgente che contiene definizioni di funzioni utilizzate anche da altre unità di compilazione esiste un file header corrispondente, contenente i prototipi di tali funzioni "esportate", ma, tendenzialmente, non di quelle utilizzate solo internamente nel file sorgente, per evitare l'uso di funzioni interne "dall'esterno": uso gli header come interfacce alle rispettive unità di compilazione, che sono divise per "temi" di gestione, come descritto in seguito.\
Non ho associato un corrispettivo file sorgente a tutti i file header (ad esempio [constants.h](./src/constants.h), [types.h](./src/types.h) e [structs.h](./src/structs.h)) poiché in alcuni casi è solo necessario definire dei nuovi tipi, delle strutture o delle costanti (tramite macro) utilizzate in diverse parti del progetto.

Di seguito sono descritti brevemente i diversi file sorgente e il loro rispettivo scopo:

### main.c
Questo file sorgente contiene l'entry point del programma, ovvero la funzione `main`, nella quale avviene l'inizializzazione (e cleanup) del gioco, viene mostrato il menu principale e viene avviato il [game loop](#game-loop).

### constants.h
Questo header non ha un corrispettivo file sorgente .c associato in quanto contiene solamente le definizioni delle costanti (es. numero massimo e minimo di giocatori e lunghezze massime di alcune stringhe) e alcuni letterali usati nel gioco (es. nomi statici dei file coi quali interagisce il programma e stringhe utilizzate nella realizzazione della grafica su terminale).

### structs.h
In questo header sono definite tutte le strutture utilizzate nel progetto (descritte nella seguente sezione). Una scelta un po' particolare inserirle tutte nello stesso file anzi che in appositi file come card.h, player.h e simili ma ho trovato questa soluzione meno confusionaria e meno dispersiva.

### enums.c & enums.h
Seguendo la stessa logica della della scelta adottata per structs.h, nel file header sono definite tutte le enumerazioni usate nel progetto.
Nel file .c sono invece definite le funzioni per effettuare le conversioni da ogni valore di ciascun enum alle corrispondenti stringhe, effettuando le associazioni tramite un metodo che ho trovato molto pulito e ottimale.

### types.h
In questo file sono contenute le definizioni dei tipi user-defined usati nel progetto.

### card.c & card.h
In questi file sorgente sono definite le principali operazioni effettuabili sulle carte, allo stesso modo di funzioni ausiliare nella gestione delle stesse.

### game.c & game.h
Questi file sorgente contengono delle funzioni essenziali per l'inizializzazione e la terminazione del gioco, ma non utilizzate durante il suo dinamico svolgimento.

### gameplay.c & gameplay.h
Questi file sorgente contengono praticamente l'intera logica di gioco e le principali funzioni per la gestione della partita in corso.

### menu.c & menu.h
Contengono solamente la funzione che gestisce il menu' principale mostrato all'avvio del gioco, prima di iniziare una partita.

### saves.c & saves.h
Qui viene gestita la logica dei salvataggi, con relativo menu' di scelta salvataggio salvato nella cache.

<br>

## Descrizione e scopo strutture aggiuntive
> [!TIP]
> Descrivere le strutture dati aggiuntive utilizzate nel progetto e perché vi sono tornate utili (se presenti).

### GameContext
La struttura aggiuntiva principale che mi è stata molto utile nel mantenere gestibile il passaggio di parametri fra le varie funzioni è stata GameContext, infatti essa viene passata a quasi tutte le funzioni inerenti alla gestione della partita.
Ecco la struttura in questione:
```c
struct GameContext {
	giocatoreT *curr_player;
	cartaT *mazzo_pesca, *mazzo_scarti, *aula_studio;
	int n_players, round_num;
	bool game_running;
	FILE *log_file;
	const char *save_path;
};
typedef struct GameContext game_contextT;
```

Mi è sufficiente utilizzare tale struttura per contenere l'intero stato della partita (all'inizio di un round, non durante), come si può notare dai prototipi delle funzioni usate per caricare e salvare i salvataggi:

```c
game_contextT *load_game(const char *save_name);
void save_game(game_contextT *game_ctx);
```

Ciò che contiene questa struttura è:
- un puntatore al giocatore che deve giocare (o che sta giocando) questo turno, che viene fatto avanzare con comodità grazie alla circolarità della lista dei giocatori alla quale appartiene.
- un puntatore alla testa ciascuna lista concatenata di carte: pesca, scarti e aula studio.
- un intero rappresentante la quantità di giocatori che stanno partecipando alla partita.
- un intero rappresentante il numero del round al quale lo stato della partita si trova.
- un booleano rappresentante se il gioco è in esecuzione (o in conclusione, solo quando un giocatore vince e la partita termina, oppure si esce dalla partita con il tasto 0 del menu d'azione).
- un puntatore a FILE (file stream) relativo al file di log, aperto prima di iniziare a giocare e chiuso quando si esce dal gioco.
TODO: update here

L'utilizzo che faccio di questa struttura è semplice e lineare: la alloco sullo heap all'avvio del gioco (tramite le funzioni `new_game` o `load_game`) e ne passo il puntatore alle diverse funzioni del game-loop (`begin_round`, `play_round`, `end_round`) che la passeranno a loro volta ad altre funzioni che implementano la logica di gioco; alla fine dell'esecuzione del gioco (uscita dal game-loop) la rilascio assieme a tutti i suoi campi (tramite `clear_game`).


### MultiLineText
Per gestire la formattazione di testo su più linee ho definito una apposita struttura che possa contenere un numero arbitrario di linee, tramite allocazioni dinamiche.
Ecco la struttura:
```c
struct MultiLineText {
	int n_lines;
	const char **lines;
	int *lengths;
};
typedef struct MultiLineText multiline_textT;
typedef multiline_textT freeable_multiline_textT;
```

Il campo `lines` contiene un puntatore ad un array di `char*` (dinamicamente allocato) contenente esattamente `n_lines` puntatori. \
Il campo `lengths` contiene un puntatore ad un array di interi (dinamicamente allocato) contenente esattamente `n_lines` interi, rappresentante ciascuno la lunghezza dell'`i`-esima linea puntata dall'array `lines` all'indice `i`.

Si può notare che questa struttura viene bindata a due diversi tipi (definiti in `types.h`), il secondo dei quali (`freeable_multiline_textT`) delinea, tramite il suo nome, la necessità di effettuare un cleanup delle linee (stringhe) in esso contenute, poiché tutte allocate nell'heap; la funzione per fare ciò è `clear_freeable_multiline`.

I principali file nei quali viene impiegata questa struttura sono `format.c` e `graphics.c`. \
Questa struttura mi è stata largamente d'aiuto per rappresentare i box delle carte formattate singolarmente (tramite `build_card`), per poi printarli tutti assieme lungo una riga ed eventualmente in colonne, come matrici tramite la funzione `show_cards_restricted`, ottenendo risultati come il seguente:

![Gruppo di box di carte stampate su diverse linee e colonne](/imgs/cards_group.png "Gruppo di box di carte stampate su diverse linee e colonne")

### WrappedText
Nuovamente, per la gestione della formattazione, ho creato una apposita struttura per contenere una stringa di testo che viene divisa in più linee, rispettando una data lunghezza massima, tramite la sostituzione, nelle corrette posizioni, degli spazi con dei terminatori di stringa (`'\0'`), di fatto, creando nuove sottostringhe. La logica di tale divisione è gestita in `format.c` dalla funzione `wrap_text`. \
Ecco la struttura dati che contiene le informazioni necessarie a mantenere traccia delle sottostringhe generate:
```c
struct WrappedText {
	char *text;
	multiline_textT multiline;
};
```

Come si può vedere questa struttura fa uso della precedentemente descritta struttura `MultiLineText` nella sua forma che non necessita cleanup di ciascuna delle stringhe singolarmente, dato che la funzione `wrap_text` usa solamente una stringa allocata sullo heap (puntata da `text` per poi essere freeata) che viene splittata, ma pur sempre mantenuta in un solo chunk dell'heap (che contiene quindi tutte le sottostringhe).

Ho usato questa struttura per formattare all'interno del box delle carte le descrizioni andando a capo in maniera dinamica, come nell'esempio di seguito, partendo dalla stringa
di descrizione `"Se questa carta e' nella tua aula all'inizio del tuo turno, puoi scartare 2 carte poi eliminare una carta studente dall'aula di un altro giocatore."` si ottiene questo risultato wrappandola:

![Box della carta 'Aula dei vecchi progetti'](/imgs/card_box.png "Box della carta 'Aula dei vecchi progetti'")

TODO: add PlayerStats here

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

### Game loop
Ho suddiviso il flusso del game loop (ciascun round) in 3 fasi:
- begin (`begin_round`): salva la partita, salva le statistiche della partita, mostra le informazioni del round attuale, applica gli effetti iniziali delle carte dell'aula e fa pescare una carta (dovuta da regolamento) al giocatore corrente.
- play (`play_round`): mostra il [menu di azione](#menu-dazione) al giocatore.
- end (`end_round`): vengono scartate le eventuali carte in eccesso dalla mano del giocatore corrente, vengono aggiornate le statistiche e viene effettuato il controllo per la sua vittoria, con conseguente fine o continuazione della partita.


### Menu d'azione
Nel menu di scelta d'azione è possibile scegliere fra le seguenti opzioni:

- `Gioca una carta dalla tua mano`: permette al giocatore corrente di giocare una carta qualsiasi dal suo mazzo. Termina la fase d'azione se viene giocata una carta.
- `Pesca un'altra carta`: fa pescare una carta al giocatore corrente. Termina la fase d'azione.
- `Visualizza le tue carte`: mostra al giocatore corrente tutte le sue carte: mazzo, aula studenti, bonus/malus.
- `Visualizza lo stato degli altri giocatori`: permette al giocatore corrente di visualizzare lo stato degli altri giocatori.
- `Esci dalla partita`: chiede conferma ed esce dal gioco.


...


### Carte giocabili
Le carte giocabili vengono contate tramite la funzione `count_playable_cards`, che considera vari casi, di seguito spiegati:
- Solo carte del tipo specificato (type) possono essere giocate: supponendo di star giocando a seguito di un effetto come `[GIOCA, IO, STUDENTE]` sarebbe possibile giocare solamente una carta `STUDENTE` (quindi `MATRICOLA`, `STUDENTE_SEMPLICE` o `LAUREANDO`) dal proprio mazzo, mentre con type = `ALL` questo controllo viene sempre passato.
- Le carte di tipo `ISTANTANEA` non possono essere giocate durante il proprio turno.
- Le carte con tipo target di un effetto `IMPEDIRE` attivo sul giocatore non possono essere giocate.

### Giocare una carta
La funzione `play_card`, che si occupa per l'appunto di permettere al giocatore corrente di scegliere una carta del suo mazzo da giocare.\
Se non ci sono carte giocabili secondo quanto descritto in [Carte Giocabili](#carte-giocabili) la fase di giocata di una carta termina senza chiedere alcuna interazione all'utente in quanto non è possibile giocare alcuna carta.\
Successivamente, una volta accertato che il giocatore abbia delle carte che può giocare nel sul mazzo, gli viene richiesto (in base al tipo della carta da giocare), quale sceglie fra le carte del suo mazzo.\
Se su tale carta vi è un effetto `IMPEDIRE` attivo sul giocatore corrente, tale carta non potrà essere giocata e verrà chiesto al giocatore di sceglierne un'altra.

In base al tipo di carta si svolgeranno azioni diverse:
- Bonus/Malus: si chiede al giocatore corrente su quale giocatore target vuole applicare la carta (lo stesso è compreso fra le scelte possibili) e tale giocatore (se non è lo stesso giocatore che tira la carta) avrà la possibilità di [difendersi](#difendersi-da-una-carta) dal piazzamento della carta Bonus/Malus (se nel suo mazzo ha a disposizione carte con l'effetto `IMPEDIRE` che glielo permettano)
- Studente: l'unico possibile target di tali carte è il giocatore corrente stesso
- Magia: il target della carta è definito dagli effetti descritti nella carta stessa

In caso di carta duplicata già presente nell'aula del target viene chiesta conferma di voler giocare comunque tale carta sul target all'utente e in caso positivo la carta andrà persa (scartata) e l'azione di play si conclude, mentre in caso negativo l'utente potrà scegliere una nuova carta o un nuovo target su cui giocarla (l'azione di play non si considera conclusa).

Giocando una carta `BONUS`/`MALUS` o `STUDENTE`, se il target non si è difeso, verrà inserita nella sua aula la carta giocata, attivando eventuali effetti di ingresso nell'aula (`SUBITO`).

Al giocare di una carta `MAGIA`, questa verrà prima rimossa dalla mano del giocatore corrente, poi ne verranno applicati gli effetti, e successivamente verrà spostata nel mazzo degli scarti. Questo per evitare particolari edge case (si immagini giocare una carta `MAGIA` che impone di eliminare una carta dal proprio mazzo e si scegliesse la carta stessa: senza l'aggiunta di altri check ne conseguirebbe un doppio inserimento nel mazzo degli scarti).

### Applicazione degli effetti

Per l'applicazione degli effetti ho adottato un flow di selezione col seguente ordine:
* `apply_effects` chiama `apply_effects_now` se il `quando` fornito corrisponde.
* `apply_effects_now` verifica se gli effetti della carta che si vogliono applicare sono opzionali, e in tal caso chiede all'utente se vuole applicarli o meno. In caso positivo si procede a chiamare `apply_effect` per ciascun effetto appartenente alla carta, verificando se l'effetto è stato bloccato dalla difesa della vittima. Se gli effetti vengono bloccati da una difesa, le carte `BONUS` e `MALUS` finiscono dritte negli scarti.
* `apply_effect` chiama la funzione `apply_effect_target` per ciascun giocatore target dell'effetto, dopo aver chiesto a tutti gli interessati (eccetto il giocatore corrente) che possono, se vogliono difendersi dall'effetto.
* `apply_effect_target` semplicemente chiama l'apposita funzione `apply_effect_<nome azione>_target` sul target fornito.

#### Ecco una breve descrizione delle azioni degli effetti applicabili:

- `ELIMINA`: il giocatore corrente sceglie quale carta vuole eliminare dall'aula del target. La carta che viene eliminata dall'aula del target attiva i suoi effetti di uscita dall'aula se con quando = `FINE`.
- `SCARTA`: se il target è il giocatore corrente, può scegliere quale carta scartare dal suo mazzo, diversamente la carta viene estratta casualmente dal mazzo della vittima e scartata.
- `GIOCA`: se il target è il giocatore corrente, esso gioca una carta dal suo mazzo, diversamente si crea un sotto-round all'interno del round nel quale il giocatore target dell'effetto gioca una carta del tipo specificato come se fosse il suo turno.
- `RUBA`: effetto sensato solo con target diverso dal giocatore corrente. La carta che viene rubata viene spostata dall'aula del target all'aula del giocatore corrente, attivando eventuali effetti di uscita o entrata in aula.
- `PRENDI`: effetto sensato solo con target diverso dal giocatore corrente. La carta che viene rubata viene spostata dalla mano del target alla mano del giocatore corrente.
- `PESCA`: il giocatore target pesca una carta dal mazzo di pesca. Se il tipo di carta target dell'effetto è diverso da `ALL`, avviene prima la pesca di una carta come usuale ma successivamente, se la carta pescata non è del tipo specificato, questa viene automaticamente scartata e la pesca si conclude.
- `SCAMBIA`: il mazzo (mano) del giocatore target viene scambiato con quello del giocatore corrente. L'unico target carte sensato è `ALL`.

### Effetti particolari

Non tutti gli effetti rispettano il pattern di essere "applicabili" da un giocatore giocante ad un giocatore target al giocare della carta come quelli descritti in [applicazione degli effetti](#ecco-una-breve-descrizione-delle-azioni-degli-effetti-applicabili).

Vi sono effetti che sono `SEMPRE` in essere una volta che la carta viene piazzata:

- `MOSTRA`: quando una carta con questo effetto è nei Bonus/Malus di un giocatore, alla visualizzazione del suo stato da parte degli altri giocatori, oltre alle normali carte dell'aula (pubbliche), verranno mostrare anche le carte del suo mazzo (private).
- `IMPEDIRE`: quando una carta con questo effetto è nei Bonus/Malus di un giocatore, non gli sarà possibile giocare carte del tipo specificato dall'effetto `IMPEDIRE`.
- `INGEGNERE`: quando una carta con questo effetto è nei Bonus/Malus di un giocatore, non gli sarà possibile vincere la partita anche se dovesse raggiungere il numero di studenti per vincere la partita nella sua aula.

Esiste inoltre un'altra azione degli effetti che si comporta ancora diversamente ed è alla base del sistema di difesa come descritto nella sezione relativa al [difendersi da una carta](#difendersi-da-una-carta): `BLOCCA`. Questa azione può essere presente solo in carte del tipo `ISTANTANEA` e permette di "rispondere" all'effetto di una carta del tipo specificato che si dovrebbe subire, bloccandolo.

### Difendersi da una carta
Quando devono essere applicati (da parte del giocatore corrente) degli effetti che impattano gli altri giocatori (`TU`, `VOI`, `TUTTI`), questi ultimi hanno la possibilità di difendersi "rispondendo" alla giocata (ad un effetto che li colpirebbe) usando una carta di tipo `ISTANTANEA`. Ovviamente tale carta deve poter essere giocata dalla vittima (non ci devono essere effetti `IMPEDIRE` attivi sulle carte `ISTANTANEA` del giocatore vittima).\
Ho assunto che tutte le carte `ISTANTANEA` debbano avere **anche** l'effetto `BLOCCA` (con carta target compatibile con quella dalla quale ci si vuole difendere) per potersi difendere al fine di semplificare l'implementazione della "risposta" ad effetti imposti da altri giocatori, ma ho reso comunque possibile aggiungere altri effetti oltre al `BLOCCA` alle carte di difesa.\
Una volta selezionata la carta con la quale la vittima si vuole difendere (se si vuole difendere), è come se si creasse un __sottoround all'interno del round__ dato che vengono eseguiti gli effetti della carta scelta (es. `[GIOCA, IO, BONUS]` o `[RUBA, TU, LAUREANDO]`) come se fosse il turno della vittima. Una volta portati a termine gli effetti della carta `ISTANTANEA` usata per difendersi, questa viene scartata e l'iniziale effetto della carta attaccante viene fermato, assieme all'intera catena di effetti della carta attaccante stessa.\
Ad ogni effetto di una carta attaccante verrà chiesto nuovamente ai giocatori target che possono difendersi se vogliono farlo (in caso avessero rifiutato di farlo per gli effetti precedenti e cambiato idea sugli effetti successivi) fin quando qualcuno non si difenda o finiscano gli effetti della carta attaccante.\
La logica del sistema di difesa è gestita dalla funzione `target_defends` (che utilizza in aggiunta `player_can_defend` e `card_can_block`).

Nell'applicare gli effetti con target giocatori `VOI` o `TUTTI`, prima di iniziare ad applicare l'effetto, viene domandato a ciascun giocatore vittima (eccetto colui che gioca la carta attaccante) che ne ha la possibilità se vuole bloccare l'attacco prima che questo inizi e difendere, di fatto, tutte le vittime.

Difendendosi dalla carta attaccante questa farà una diversa fine in base al suo tipo:
- `LAUREANDO`: la carta resterà all'interno dell'aula del giocatore attaccante, ma gli effetti saranno fermati
- `MAGIA`: la carta finirà (come normalmente) negli scarti e gli effetti saranno fermati
- `BONUS`/`MALUS`: la carta finirà negli scarti

...
