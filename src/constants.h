#ifndef CONSTANTS_H
#define CONSTANTS_H

#define GIOCATORE_NAME_LEN 31
#define CARTA_NAME_LEN 31
#define CARTA_DESCRIPTION_LEN 255

#define MIN_PLAYERS 2
#define MAX_PLAYERS 4

#define CARDS_PER_PLAYER 5
#define ENDROUND_MAX_CARDS 5
#define WIN_STUDENTS_COUNT 6

#define SAVE_NAME_LEN 255

#define SAVES_DIRECTORY "saves/"
#define FILE_SAVES_CACHE "cache.txt"
#define SAVE_PATH_EXTENSION ".sav"
#define FILE_MAZZO "mazzo.txt"
#define FILE_LOG "log.txt"

#define ACTION_PLAY_HAND 1
#define ACTION_DRAW 2
#define ACTION_VIEW_OWN 3
#define ACTION_VIEW_OTHERS 4
#define ACTION_QUIT 0

#define MAX_EFFECTS 6

#define ONE_ELEMENT 1

// "Slant" font
#define MENU_ASCII_ART ANSI_YELLOW \
"   __  __           __        __    __        _____ __            __           __      \n" \
"  / / / /___  _____/ /_____ _/ /_  / /__     / ___// /___  ______/ /__  ____  / /______\n" \
" / / / / __ \\/ ___/ __/ __ `/ __ \\/ / _ \\    \\__ \\/ __/ / / / __  / _ \\/ __ \\/ __/ ___/\n" \
"/ /_/ / / / (__  ) /_/ /_/ / /_/ / /  __/   ___/ / /_/ /_/ / /_/ /  __/ / / / /_(__  ) \n" \
"\\____/_/ /_/____/\\__/\\__,_/_.___/_/\\___/   /____/\\__/\\__,_/\\__,_/\\___/_/ /_/\\__/____/  \n" \
ANSI_RESET "                                    with " ANSI_RED "<3" ANSI_RESET " by " ANSI_BOLD ANSI_CYAN "doliv" ANSI_RESET "\n"

#define MENU_NEWGAME 1
#define MENU_LOADSAVE 2
#define MENU_QUIT 0

#define CHOICE_AULA 1
#define CHOICE_BONUSMALUS 2

#define CARD_WIDTH 34
#define CARD_BORDER_WIDTH 1
#define CARD_CONTENT_WIDTH CARD_WIDTH-2*CARD_BORDER_WIDTH
#define CARD_PADDING 4
#define CARD_DESCRIPTION_HEIGHT 6
#define CARD_EFFECTS_HEADER_HEIGHT 3
#define CARD_EFFECTS_HEIGHT MAX_EFFECTS+CARD_EFFECTS_HEADER_HEIGHT
#define CARD_HEIGHT 1+2+CARD_DESCRIPTION_HEIGHT+CARD_EFFECTS_HEIGHT+1
#define CARDS_PER_ROW 4
#define CARDS_HORIZONTAL_SPACING 2


#define HORIZONTAL_BAR "--------------------------------" // CARD_CONTENT_WIDTH long
#define CARD_BORDER_HORIZONTAL "-"
#define CARD_BORDER_VERTICAL "|"
#define CARD_CORNER_LEFT "("
#define CARD_CORNER_RIGHT ")"

#define ROUND_BANNER_WIDTH 64
#define BANNER_BORDER_WIDTH 1
#define ROUND_BANNER_CONTENT_WIDTH ROUND_BANNER_WIDTH-2*BANNER_BORDER_WIDTH
#define BANNER_HORIZONTAL_BAR "--------------------------------------------------------------" // ROUND_BANNER_CONTENT_WIDTH long

// text styles
#define ANSI_RESET "\033[0m"
#define ANSI_BOLD "\033[1m"
#define ANSI_UNDERLINE "\033[4m"

// text colors
#define ANSI_RED "\033[31m"
#define ANSI_GREEN "\033[32m"
#define ANSI_YELLOW "\033[33m"
#define ANSI_BLUE "\033[34m"
#define ANSI_MAGENTA "\033[35m"
#define ANSI_CYAN "\033[36m"
#define ANSI_WHITE "\033[37m"

// background colors
#define ANSI_BG_RED "\033[41m"
#define ANSI_BG_GREEN "\033[42m"
#define ANSI_BG_YELLOW "\033[43m"
#define ANSI_BG_BLUE "\033[44m"
#define ANSI_BG_MAGENTA "\033[45m"
#define ANSI_BG_CYAN "\033[46m"
#define ANSI_BG_WHITE "\033[47m"

#define PRETTY_USERNAME ANSI_UNDERLINE "%s" ANSI_RESET
#define COLORED_CARD_TYPE "%s%s" ANSI_RESET

#endif // CONSTANTS_H