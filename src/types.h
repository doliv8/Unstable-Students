#ifndef TYPES_H
#define TYPES_H

// base types
typedef struct Giocatore giocatoreT;
typedef struct Carta cartaT;
typedef struct Effetto effettoT;

typedef enum TipoCarta tipo_cartaT;
typedef enum Quando quandoT;
typedef enum Azione azioneT;
typedef enum TargetGiocatori target_giocatoriT;
// end base types

typedef struct GameContext game_contextT;
typedef struct MultiLineText multiline_textT;
typedef multiline_textT freeable_multiline_textT;
typedef struct WrappedText wrapped_textT;
typedef struct MultiLineContainer multiline_containerT;

#endif // TYPES_H