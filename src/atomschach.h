#ifndef ATOMSCHACH_H
#define ATOMSCHACH_H

typedef int brett_t[64];

typedef enum {
    weiss = 1 , 
    schwarz = -1
}farbname_t;

typedef char zug_lesbar_t[30];

typedef struct{
    int von_x;
    int von_y;
    int nach_x;
    int nach_y;
} zug_t;

typedef struct{
    int 		von_x;
    int 		von_y;
    int 		nach_x;
    int 		nach_y;
    zug_lesbar_t	zuglesbar;
    int			punkte;
} history_t;


typedef int bool;
typedef int umgebung_t[10];

#define TRUE 1
#define FALSE 0

#define MAX_TMP_INIT -10000
#define MAX_TMP_INIT_SMALL -8000

/****************************/
/* increase MAX_TIEFE for   */
/* higher difficulty levels */
/****************************/
#define MAX_TIEFE 6

// mindestens MAX_TIEFE + 2 ! +1 wegen tipp
#define ZUKUNFT_MAX MAX_TIEFE+3

// mindestend 1 höher als MAX_TIEFE+2+1
#define INIT_FACTOR MAX_TIEFE+4

#define HISTORY_MAX 512

//extern int farbe, mensch, computer, spiel, tiefe;
extern int mensch, computer, spiel, tiefe;
extern int mensch_farbe;
extern brett_t brett;
extern brett_t merke_brett;
extern history_t history[HISTORY_MAX];
extern bool legal (int vonx, int vony, int nachx, int nachy, brett_t *brett_p);
extern umgebung_t umgebung_liste[128];

extern int im_schach(farbname_t farbe, brett_t *brett_p, zug_t *zug_p);
extern void computer_zug (farbname_t farbe, int tiefe, brett_t *brett_p, zug_t *zug_p, int *punkte_p, bool set_message);
extern void newGame (void);
extern void sub_main (farbname_t farbe,int tiefe,brett_t *brett_p);
extern void zuglesbar_func(zug_t *zug_p, farbname_t farbe, brett_t *brett_p, zug_lesbar_t zuglesbar);
extern void anwenden(brett_t *brett_p, zug_t *zug_p);
extern char *string_farbname(farbname_t farbname);
extern float bewertung_float(int bewertung_int, int farbe, int tiefe);

#endif
