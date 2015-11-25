/*  Nuclear Chess: A Chess variant:
    Whenever a piece is captured both pieces die and all pieces on neighbour fields.

    Copyright (C) 2003 Harald Bartel
    Thanks to Karl Bartel and Ulf Bartel.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Harald Bartel
    GERMANY
    harald.bartel@prozentor.de
*/

// ### to do ###
// # nicht 3 mal den vollen Zug wiederholen
// # den Wert der Dame optimal bestimmen (>=200)

#include <stdio.h>
#include <string.h>
#include <math.h>	// for points
#include <stdlib.h>

#include "atomschach.h"

#define LIST_LEN 65
#define VERWANDLUNS_BONUS 60

umgebung_t umgebung_liste[128];

int x_koordinate[64];
int y_koordinate[64];

bool immune_pawns;

brett_t brett_start = {-4,-3,-2,-5,-6,-2,-3,-4, -1,-1,-1,-1,-1,-1,-1,-1,  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  1, 1, 1, 1, 1, 1, 1, 1,  4, 3, 2, 5, 6, 2, 3, 4};
brett_t brett;

farbname_t mensch, computer;

int   grundwert[7]; 	// eigentlich von 1 bis 6

const int malus_schach_und_dran		= 50;
const int malus_schach_und_nicht_dran	= 4000;
const int malus_schachmatt		= 10000;

// ############
// ### subs ###
// ############


void add_capture(int new_capture, brett_t *brett_p,
				 int schlag[], int *schlag_index_p, int farbe)
{
	int target_piece = (*brett_p)[new_capture];

	if (
			target_piece * farbe < 0  // opponent's piece
			&& target_piece != 100  // not nuke
	) {
		//fprintf(stderr, "%d %d %d\n", new_capture, *schlag_index_p, farbe);
		schlag[(*schlag_index_p)++] = new_capture;
	}
}


void allowed_straight_moves(int x_change, int y_change,
		int x, int y, brett_t *brett_p,
		int zug[], int *zug_index_p, int schlag[], int *schlag_index_p,
		int farbe)
{
	int i_change = x_change + 8 * y_change;
	int X = x + x_change;
	int Y = y + y_change;
	int I = X + 8 * Y;

	//fprintf(stderr, ">> %d %d %d -- %d\n", I, X, Y, *zug_index_p);
	while (
			X >= 0 && X <= 7 &&
			Y >= 0 && Y <= 7 &&
			(*brett_p)[I] == 0
    ) {
		//fprintf(stderr, "%d %d %d -- %d\n", I, X, Y, *zug_index_p);
		zug[(*zug_index_p)++] = I;
		//fprintf(stderr, "%d %d %d -- %d\n", I, X, Y, *zug_index_p);
		X += x_change;
		Y += y_change;
		I += i_change;
	}
	//fprintf(stderr, "foo\n");
	if (
		X >= 0 && X <= 7 &&
		Y >= 0 && Y <= 7
	) {
		//fprintf(stderr, ">> %d %d %d\n", X, Y, I);
		add_capture(I, brett_p, schlag, schlag_index_p, farbe);
	}
}


void allowed_moves_diagonal(
		int x, int y, brett_t *brett_p,
		int zug[], int *zug_index_p, int schlag[], int *schlag_index_p,
		int farbe)
{
	// up-right
	allowed_straight_moves(+1, -1, x, y, brett_p,
			zug, zug_index_p, schlag, schlag_index_p, farbe);
	// down-right
	allowed_straight_moves(+1, +1, x, y, brett_p,
			zug, zug_index_p, schlag, schlag_index_p, farbe);
	// up-left
	allowed_straight_moves(-1, -1, x, y, brett_p,
			zug, zug_index_p, schlag, schlag_index_p, farbe);
	// down-left
	allowed_straight_moves(-1, +1, x, y, brett_p,
			zug, zug_index_p, schlag, schlag_index_p, farbe);
}


void allowed_moves_axial(
		int x, int y, brett_t *brett_p,
		int zug[], int *zug_index_p, int schlag[], int *schlag_index_p,
		int farbe)
{
	// up
	allowed_straight_moves( 0, -1, x, y, brett_p,
			zug, zug_index_p, schlag, schlag_index_p, farbe);
	// down
	allowed_straight_moves( 0, +1, x, y, brett_p,
			zug, zug_index_p, schlag, schlag_index_p, farbe);
	// left
	allowed_straight_moves(-1,  0, x, y, brett_p,
			zug, zug_index_p, schlag, schlag_index_p, farbe);
	// right
	allowed_straight_moves(+1,  0, x, y, brett_p,
			zug, zug_index_p, schlag, schlag_index_p, farbe);
}

void allowed_move_or_capture(
		int i, brett_t *brett_p,
		int zug[], int *zug_index_p, int schlag[], int *schlag_index_p,
		int farbe)
{
	if ((*brett_p)[i] == 0) {
		zug[(*zug_index_p)++] = i;
	} else {
		add_capture(i, brett_p, schlag, schlag_index_p, farbe);
	}
}


void erlaubte_zuege(int x, int y, brett_t *brett_p, int zug[], int schlag[]){
    farbname_t farbe;

    int zug_index=0;
    int schlag_index=0;

    int figurf, figur;
    int i, I, X, Y, index;

    bool weiss=TRUE;

    umgebung_t *uref_p;
    int ui, ziel;

    i 		= x + 8*y;
    figurf 	= (*brett_p)[i];
    figur	= abs(figurf);

	if (figurf > 0) {
		farbe = weiss;
		weiss = TRUE;
	} else {
		farbe = schwarz;
		weiss = FALSE;
	}

	switch (figur) {
		case 1: 						// ### Bauer ###
			if (weiss) {
				if (y > 0) {
					if	((*brett_p)[i-8] == 0) {		// Bauer Schritt
						zug[zug_index++] = i-8;
						if	(y == 6   && (*brett_p)[i-16] 	== 0) { // Bauer Doppel-Schritt
							zug[zug_index++] = i-16;
						}
					}
					if	(x < 7) {	// Bauer Schlag rechts
						add_capture(i - 7, brett_p, schlag, &schlag_index, farbe);
					}
					if	(x > 0) {	// Bauer Schlag links
						add_capture(i - 9, brett_p, schlag, &schlag_index, farbe);
					}
				}
			} else {
				if (y < 7) {
					if	((*brett_p)[i+8] == 0) {		// Bauer Schritt
						zug[zug_index++] = i+8;
						if	(y == 1   && (*brett_p)[i+16] 	== 0) { // Bauer Doppel-Schritt
							zug[zug_index++] = i+16;
						}
					}
					if	(x < 7) {	// Bauer Schlag rechts
						add_capture(i + 9, brett_p, schlag, &schlag_index, farbe);
					}
					if	(x > 0) {	// Bauer Schlag links
						add_capture(i + 7, brett_p, schlag, &schlag_index, farbe);
					}
				}
			}
			break;
		case 2: 						// ### Läufer ###
			allowed_moves_diagonal(x, y, brett_p,
					zug, &zug_index, schlag, &schlag_index, farbe);
			break;
		case 3: 						// ### Springer ###
			if (x < 6) {
				if (y > 0) {						// Springer rechts oben
					allowed_move_or_capture(i - 6, brett_p,
							zug, &zug_index, schlag, &schlag_index, farbe);
				}
				if (y < 7) {						// Springer rechts unten
					allowed_move_or_capture(i + 10, brett_p,
							zug, &zug_index, schlag, &schlag_index, farbe);
				}
			}
			if (x < 7) {
				if (y > 1) {						// Springer oben rechts
					allowed_move_or_capture(i - 15, brett_p,
							zug, &zug_index, schlag, &schlag_index, farbe);
				}
				if (y < 6) {						// Springer unten rechts
					allowed_move_or_capture(i + 17, brett_p,
							zug, &zug_index, schlag, &schlag_index, farbe);
				}
			}
			if (x > 1) {
				if (y > 0) {						// Springer links oben
					allowed_move_or_capture(i - 10, brett_p,
							zug, &zug_index, schlag, &schlag_index, farbe);
				}
				if (y < 7) {						// Springer links unten
					allowed_move_or_capture(i + 6, brett_p,
							zug, &zug_index, schlag, &schlag_index, farbe);
				}
			}
			if (x > 0) {
				if (y > 1) {						// Springer oben links
					allowed_move_or_capture(i - 17, brett_p,
							zug, &zug_index, schlag, &schlag_index, farbe);
				}
				if (y < 6) {						// Springer unten links
					allowed_move_or_capture(i + 15, brett_p,
							zug, &zug_index, schlag, &schlag_index, farbe);
				}
			}
			break;
		case 4: 						// ### Turm ###
			allowed_moves_axial(x, y, brett_p,
					zug, &zug_index, schlag, &schlag_index, farbe);
			break;
		case 5: 						// ### Dame ###
			allowed_moves_diagonal(x, y, brett_p,
					zug, &zug_index, schlag, &schlag_index, farbe);
			allowed_moves_axial(x, y, brett_p,
					zug, &zug_index, schlag, &schlag_index, farbe);
			break;
		case 6:							// ### König ###
			uref_p = &(umgebung_liste[i]);

			for (index=0 ; (*uref_p)[index] != -1; index++){
				ui=(*uref_p)[index];
				ziel = (*brett_p)[ui];
				if 		(ziel == 0)		{
					zug[zug_index++]=ui;
				} else if 	(farbe*ziel < 0) 	{
					schlag[schlag_index++]=ui;
				}
			}
			// # Rochade
			if (x == 4) {
				if 		(y == 7 && weiss) {
					if	((*brett_p)[i+1] == 0 && (*brett_p)[i+2] == 0 && (*brett_p)[i+3] == 4) {				// weisse Rochade rechts
						zug[zug_index++]=i+2;
					}
					if 	((*brett_p)[i-1] == 0 && (*brett_p)[i-2] == 0 && (*brett_p)[i-3] == 0 && (*brett_p)[i-4] == 4) {	// weisse Rochade links
						zug[zug_index++]=i-2;
					}
				} else if 	(y == 0 && !weiss) {
					if	((*brett_p)[i+1] == 0 && (*brett_p)[i+2] == 0 && (*brett_p)[i+3] == -4) {				// schwarze Rochade rechts
						zug[zug_index++]=i+2;
					}
					if 	((*brett_p)[i-1] == 0 && (*brett_p)[i-2] == 0 && (*brett_p)[i-3] == 0 && (*brett_p)[i-4] == -4) {	// schwarze Rochade links
						zug[zug_index++]=i-2;
					}
				}
			}

			break;
		default :
			break;
	}

	zug[zug_index++]=-1;
	schlag[schlag_index++]=-1;
}

void punkte(farbname_t farbe, brett_t *brett_p, int *punkte_weiss_p, int *punkte_schwarz_p) {
    int x,y;
    int i,index,index2,index3;
    int figur, figurf;
    bool weiss;
    int drohwert, zusatzwert, gesamtwert;
    int nebenkoenig_wert;
    int schlagpunkte = 0;
    int schlagpunktemax;
    int malus;
    bool dran;

    int refz[LIST_LEN];
    int refs[LIST_LEN];

    int schlag;
    umgebung_t *uref_p;
    int ui;
    umgebung_t *urefs_p;

    int schlagfigurf, schlagfigur;

    int punkte_weiss	= 0;
    int punkte_schwarz	= 0;

    bool weiss_hat_koenig	= FALSE;
    bool schwarz_hat_koenig	= FALSE;

    int anzahl_figuren = 0;
    int anzahl_turm_weiss = 0;
    int anzahl_turm_schwarz = 0;
    int anzahl_springer_weiss = 0;
    int anzahl_springer_schwarz = 0;

    int eigener_koenig_in_schlagbereich = 0;
    int schlagfigurremisf = 0;

	for ( y=0 ; y<8 ; y++) {
		for ( x=0 ; x<8 ; x++) {
			i = x+8*y;
			figurf = (*brett_p)[i];

			if (figurf*farbe > 0) {
				dran	= TRUE;
			} else {
				dran	= FALSE;
			}

			if (figurf != 0) {
				anzahl_figuren++;
				figur	= abs(figurf);
				if (figurf > 0) {
					weiss = TRUE;
				} else {
					weiss = FALSE;
				}

				uref_p = &(umgebung_liste[i]);
				// Zusatztwerte für bestimmte Figuren
				zusatzwert 		= 0;
				nebenkoenig_wert 	= 0;
				if 	(figur == 1) {				// Bauer Zusatzwert
					if (weiss) {
						zusatzwert	= 30-5*y;	// (6-y)*10/2
						if (y == 1 && (*brett_p)[i-8] == 0) {
							zusatzwert	+= VERWANDLUNS_BONUS;	// Bauernverwandlung möglich
						}
					} else {
						zusatzwert 	= 5*y-5;	// (y-1)*10/2
						if (y == 6 && (*brett_p)[x+8] == 0) {
							zusatzwert	+= VERWANDLUNS_BONUS;	// Bauernverwandlung möglich
						}
					}
				} else if (figur == 6) { 			// König Zusatzwert
					if (weiss) {
						weiss_hat_koenig	= TRUE;
					} else {
						schwarz_hat_koenig	= TRUE;
					}
					for (index=0 ; (*uref_p)[index] != -1; index++){
						ui= (*uref_p)[index];
						if 	((*brett_p)[ui] == 0) 			{zusatzwert+=10;}
						else if (((*brett_p)[i])*((*brett_p)[ui]) < 0) 	{nebenkoenig_wert += grundwert[abs((*brett_p)[ui])];};
					}
				} else if (figur == 3) { // # Springer
					if (weiss) {
						anzahl_springer_weiss++;
					} else {
						anzahl_springer_schwarz++;
					}
				} else if (figur == 4) { // # Turm
					if (weiss) {
						anzahl_turm_weiss++;
					} else {
						anzahl_turm_schwarz++;
					}
				}

				// mit eigenen Figuren besetzte Nachbarfelder sind schlecht
				malus = 0;
				for (index=0 ; (*uref_p)[index] != -1; index++){
					ui= (*uref_p)[index];
					if (figurf*((*brett_p)[ui]) > 0) {malus++;};
				}
				erlaubte_zuege(x, y, brett_p, refz , refs);

				// Felder bedrohen ist gut
				if (dran) {
					//laenge von refz
					for (index=0 ; refz[index] != -1; index++){};
					drohwert 	= index*10;
				} else {
					drohwert = 0;
				}

				// Schlagmöglichkeit ist gut
				schlagpunktemax = 0;
				for (index=0 ; refs[index] != -1; index++){
					schlag=refs[index];
					if (dran) {
						if (abs(x-x_koordinate[schlag]) > 1 || abs(y-y_koordinate[schlag]) > 1) {
							schlagpunkte 	= -grundwert[figur];
						} else {
							schlagpunkte 	= 0;
						}
					}

					urefs_p = &(umgebung_liste[schlag+64]);
					for (index2=0 ; (*urefs_p)[index2] != -1; index2++){
						schlagfigurf = (*brett_p)[(*urefs_p)[index2]];
						schlagfigur  = abs(schlagfigurf);
						if (dran) {
							if (figurf*schlagfigurf < 0) {
								schlagpunkte += grundwert[schlagfigur];
							} else {
								schlagpunkte -= grundwert[schlagfigur];
							}
						}

						if (schlagfigur == 6 && figurf*schlagfigurf < 0) {
							eigener_koenig_in_schlagbereich = 0;
							for (index3=0 ; (*urefs_p)[index3] != -1; index3++){
								schlagfigurremisf = (*brett_p)[(*urefs_p)[index3]];
								if (abs(schlagfigurremisf) == 6 && schlagfigurremisf*figurf > 0) {
									eigener_koenig_in_schlagbereich = 1;
									break;
								}
							}

							if (!eigener_koenig_in_schlagbereich) {
								if (dran) {  // fremder König kann sofort geschlagen werden
									if (weiss) {
										punkte_schwarz	-= malus_schach_und_nicht_dran;
									} else {
										punkte_weiss	-= malus_schach_und_nicht_dran;
									}
								} else {  // fremder König im Schach kann aber nicht sofort geschlagen werden
									if (weiss) {
										punkte_schwarz	-= malus_schach_und_dran;
									} else {
										punkte_weiss	-= malus_schach_und_dran;
									}
								}
							}
						}
					}
					if (dran && schlagpunkte > schlagpunktemax) {
						schlagpunktemax = schlagpunkte;
					}
				}
				if (dran && schlagpunktemax > 0) {
					drohwert += schlagpunktemax;
				}

				gesamtwert = (grundwert[figur]*(16-malus))/16 + zusatzwert;

				if (weiss) {
					punkte_weiss	+= gesamtwert + (drohwert - nebenkoenig_wert)/2;
				} else {
					punkte_schwarz	+= gesamtwert + (drohwert - nebenkoenig_wert)/2;
				}
			}
		}
	}

	punkte_weiss   += anzahl_turm_weiss*(32-anzahl_figuren)*2   - anzahl_springer_weiss*(32-anzahl_figuren);
	punkte_schwarz += anzahl_turm_schwarz*(32-anzahl_figuren)*2 - anzahl_springer_schwarz*(32-anzahl_figuren);

	if (!weiss_hat_koenig) {
		punkte_weiss	-= malus_schachmatt;
	}
	if (!schwarz_hat_koenig) {
		punkte_schwarz	-= malus_schachmatt;
	}
	if (punkte_weiss == punkte_schwarz) {
		punkte_weiss++;
	}

	if (!weiss_hat_koenig && !schwarz_hat_koenig) {	// remis
		punkte_weiss	= 0;
		punkte_schwarz	= 0;
	}

	// return values
	*punkte_weiss_p=punkte_weiss;
	*punkte_schwarz_p=punkte_schwarz;
}

int im_schach(farbname_t farbe, brett_t *brett_p, zug_t *zug_p) {

	// ### erster Rückgabewert ######################################
	// # -1	: kein König mehr
	// #  0	: König nicht im Schach
	// #  1	: König im Schach 	(mattzug  zurückgeben)
	// #  2	: remis möglich 	(remiszug zurückgeben)
	// ### zweiter Rückgabewert: zug ################################

	int x, y, i;
	int koenig_x=-1;
	int koenig_y=-1;
	int koenig_pos=-1;
	int fremder_koenig_x=-1;
	int fremder_koenig_y=-1;
	int refz[LIST_LEN];
	int refs[LIST_LEN];
	int index;

	int schlagx, schlagy, schlag;

	for (y=0 ; y<8 ; y++){
		for (x=0 ; x<8 ; x++){
			i = x+8*y;
			if ((*brett_p)[i] == farbe*6) {
				koenig_pos 		= i;  // Position des eigenen Königs
				koenig_x		= x;
				koenig_y		= y;
			}
			if ((*brett_p)[i] == -farbe*6) {
				fremder_koenig_x	= x;  // Position des fremden Königs
				fremder_koenig_y	= y;
			}
		}
	}

	if (koenig_pos<0) {
		return (-1);					// kein König mehr
	}

	for (y=0 ; y<8 ; y++){
		for (x=0 ; x<8 ; x++){
			i = x+8*y;
			if (farbe * ((*brett_p)[i]) < 0) { // fremde Figur
				erlaubte_zuege(x, y, brett_p, refz, refs);
				for (index=0; refs[index] != -1 ;index++){
					schlag=refs[index];
					schlagx	= x_koordinate[schlag];
					schlagy	= y_koordinate[schlag];

					if ( (abs(koenig_x-schlagx) <= 1) && (abs(koenig_y-schlagy) <= 1) ) { // koenig durch fremde figur bedroht
						if ( (abs(fremder_koenig_x-schlagx) > 1) || (abs(fremder_koenig_y-schlagy) > 1) ) { // remis möglich
							zug_p->von_x=x;
							zug_p->von_y=y;
							zug_p->nach_x=schlagx;
							zug_p->nach_y=schlagy;

							return (1);
						}
					}
				}
			}
		}
	}

	return 0;  // König nicht im Schach
}


void anwenden(brett_t *brett_p, zug_t *zug_p) {
	int von, nach, index;
	int ui;
	int bewegte_figur;

	umgebung_t *uref_p;

	von 	= zug_p->von_x +8*zug_p->von_y;
	nach	= zug_p->nach_x + 8*zug_p->nach_y;

	if ((*brett_p)[nach] == 0) {	// ### ziehen ###
		bewegte_figur = (*brett_p)[von];
		(*brett_p)[von]	= 0;

		(*brett_p)[nach] = bewegte_figur;

		if ( ((*brett_p)[nach] ==  1) && (zug_p->nach_y == 0) ) {
			(*brett_p)[nach] 	=  5;					// Bauernverwandlung weiss
		} else if ( ((*brett_p)[nach] == -1) && (zug_p->nach_y == 7) ) {
			(*brett_p)[nach] 	= -5;					// Bauernverwandlung schwarz
		}
		if (abs((*brett_p)[nach]) == 6) {				// Rochade
			if 	(zug_p->nach_x - zug_p->von_x == 2) {
				(*brett_p)[nach-1]	= (*brett_p)[nach+1];
				(*brett_p)[nach+1]	= 0;
			} else if 	(zug_p->nach_x - zug_p->von_x == -2) {
				(*brett_p)[nach+1]	= (*brett_p)[nach-2];
				(*brett_p)[nach-2]	= 0;
			}
		}
	} else {  // ### schlagen ###
		(*brett_p)[von]	= 0;

		uref_p = &(umgebung_liste[nach+64]);
		for (index=0 ; (*uref_p)[index] != -1 ; index++) {
			ui = (*uref_p)[index];
			if (immune_pawns
				   	&& abs((*brett_p)[ui]) == 1  // is pawn
					&& ui != nach  // is not hit directly
				) {
				// skip pawns if immune
				continue;
			}
			(*brett_p)[ui]	= 0;
		}
	}
}

void zug_anhaengen(zug_t zugliste[], zug_t *zug){
    int index;

    for (index=0; (zugliste[index]).von_x != -1 ; index++){}
    zugliste[index].von_x = zug->von_x;
    zugliste[index].von_y = zug->von_y;
    zugliste[index].nach_x = zug->nach_x;
    zugliste[index].nach_y = zug->nach_y;

    zugliste[index+1].von_x=-1;
}

// to do: langsam, aber wird nicht oft aufgerufen.
void vorsortieren(int punkte_liste[], int index_liste[], int nr){
	int j, i;
	int max;
	int index_liste_index;
	bool schon_drin;

	for (index_liste_index=0 ; index_liste_index < nr ; index_liste_index++){
		max=2*INIT_FACTOR*MAX_TMP_INIT;
		for (j=0 ; j < nr ; j++){
			if ((punkte_liste[j] >= max) && ((index_liste_index == 0) || ((index_liste_index > 0) && (punkte_liste[j] <= punkte_liste[index_liste[index_liste_index-1]])))) {
				schon_drin = FALSE;
				for (i = 0; i < index_liste_index; i++) {
					if (index_liste[i] == j) {
						schon_drin = TRUE;
						break;
					}
				}
				if (!schon_drin) {
					index_liste[index_liste_index] = j;
					max = punkte_liste[j];
				}
			}
		}
	}
}


void bester_zug(farbname_t farbe, int start_tiefe, int tiefe, int max_tmp, brett_t *brett_p,zug_t zugliste[],int *gegenpunkte_max_p) {
	int punkte_weiss, punkte_schwarz;
	int x, y, i;
	int count, j, gegenpunkte;
	int gegenpunkte_max=MAX_TMP_INIT;
	brett_t brett;
	zug_t zugliste_copy[ZUKUNFT_MAX];
	zug_t zugliste_max[ZUKUNFT_MAX];
	int index = 0;

	zug_t zug_liste[2*LIST_LEN];
	int   punkte_liste[2*LIST_LEN];
	int   index_liste[2*LIST_LEN];

	int zug_index=0;
	int punkte_index=0;

	zug_t *zug;

	int refz[LIST_LEN];
	int refs[LIST_LEN];

	int punkte_max;
	int index_max=0;
	int abbruchart = 0;

	zugliste_max[0].von_x = -1;

	// alle erlaubten Züge aller eigenen Figuren bestimmen
	for (y=0 ; y < 8 ; y++) {
		for (x=0 ; x < 8 ; x++) {
			i = x+8*y;
			if (farbe*((*brett_p)[i]) > 0) {
				erlaubte_zuege(x, y, brett_p , refz, refs);
				// Züge und Schläge
				for (index=0 ; refz[index] != -1 ; index++){
					j=refz[index];

					zug=&(zug_liste[zug_index++]);

					zug->von_x = x;
					zug->von_y = y;
					zug->nach_x = x_koordinate[j];
					zug->nach_y = y_koordinate[j];

					// Punkte fuer moeglichen zug bestimmen (auf copy)
					for (count=0; count<64 ; count++){	// neu erzeugen / brett kopierten
						brett[count]= (*brett_p)[count];
					}

					anwenden(&brett, zug);
					punkte(-farbe, &brett, &punkte_weiss, &punkte_schwarz);

					punkte_liste[punkte_index++]= farbe*( punkte_weiss - punkte_schwarz);
				}

				// genau dasselbe nochmal (refs statt refz)
				for (index=0 ; refs[index] != -1 ; index++){
					j=refs[index];

					zug=&(zug_liste[zug_index++]);

					zug->von_x = x;
					zug->von_y = y;
					zug->nach_x = x_koordinate[j];
					zug->nach_y = y_koordinate[j];

					// Punkte fuer moeglichen zug bestimmen (auf copy)
					for (count=0; count<64 ; count++){	// neu erzeugen / brett kopierten
						brett[count]= (*brett_p)[count];
					}

					anwenden(&brett, zug);
					punkte(-farbe, &brett, &punkte_weiss, &punkte_schwarz);

					punkte_liste[punkte_index++]= farbe*( punkte_weiss - punkte_schwarz);
				}
			}
		}
	}

	// abbruch bei tiefe == 0
	if (tiefe == 0) {
		//ulfi was ist wenn es keinen zug gibt ? -> punkte_index
		punkte_max 	= punkte_liste[0];
		index_max 	= 0;

		//suche nach bestem zug
		for (j=1 ; j<punkte_index ; j++){
			if (punkte_liste[j] > punkte_max) {
				index_max	= j;
				punkte_max	= punkte_liste[j];
			}
		}

		*gegenpunkte_max_p= -punkte_max;
		zug_anhaengen(zugliste, &zug_liste[index_max]);
		return;
	}

	vorsortieren(punkte_liste, index_liste, punkte_index);  // ergebnis in index_liste

	if ((start_tiefe-tiefe) % 2 == 1) {  // Zug des nicht aktiven Spielers
		gegenpunkte_max = (tiefe+1)*MAX_TMP_INIT;
	}

	for (j=0 ; j < punkte_index ; j++) {  // der Reihe nach die besten Züge durchlaufen

		index=index_liste[j];

		if (tiefe == start_tiefe && j==0) {
			/*            selected = 1;*/
			/*            select_old_x = select_x;*/
			/*            select_old_y = select_y;*/
			/*            select_x = zug_liste[index].von_x;*/
			/*            select_y = zug_liste[index].von_y;*/
			/*            show_selected_piece(select_x, select_y, select_old_x, select_old_y);*/
		}

		if ((start_tiefe-tiefe) % 2 == 0) {  // ### Zug des aktiven Spielers ###
			if (punkte_liste[index] < -malus_schach_und_nicht_dran + 1500) {
				//printf ("V%i",tiefe);
				continue;  // Verlierer-Züge nicht weiter untersuchen
			}
			if (punkte_liste[index] > malus_schach_und_nicht_dran - 1500) {
				//printf ("G%i",tiefe);
				abbruchart = 1;
				//printf("\nbreak G %d\n", tiefe);
				break;  // Gewinner-Züger sofort ziehen
			}
		} else { // ### Zug des nicht aktiven Spielers ###
			if (punkte_liste[index] < -malus_schach_und_nicht_dran + 1500) {
				//printf ("v%i",tiefe);
				continue;  // Verlierer-Züge weiter untersuchen (er könnte sich noch verbessern)
			}
			if (punkte_liste[index] > malus_schach_und_nicht_dran - 1500) {
				printf("\nDieser Fall sollte nie auf treten, da vorher V erkannt werden sollte!\n");
				exit(1);
				//printf ("g%i",tiefe);
				abbruchart = 2;
				//printf("\nbreak g %d\n", tiefe);
				break;                                                           		// Gewinner-Züger sofort ziehen
			}
		}

		for (count=0; count<64 ; count++){	// neu erzeugen / brett kopierten
			brett[count]= (*brett_p)[count];
		}

		// zugliste kopieren
		for (count=0 ; count<ZUKUNFT_MAX ; count++){
			zugliste_copy[count].von_x  = zugliste[count].von_x;
			zugliste_copy[count].von_y  = zugliste[count].von_y;
			zugliste_copy[count].nach_x = zugliste[count].nach_x;
			zugliste_copy[count].nach_y = zugliste[count].nach_y;

			if (zugliste_copy[count].von_x == -1) {break;};
		}

		anwenden(&brett, &(zug_liste[index]) );		// Zug auf Brett anwenden
		zug_anhaengen(zugliste_copy, &zug_liste[index]);

		if (punkte_liste[index] == 0) {
			//printf ("=%i",tiefe);
			gegenpunkte = 0;				// remis // to do: ok ???
		} else {
			bester_zug(-farbe, start_tiefe, tiefe-1, gegenpunkte_max, &brett, zugliste_copy , &gegenpunkte);
		}

		if (gegenpunkte > gegenpunkte_max) {
			gegenpunkte_max	= gegenpunkte;
			index_max           = index;

			for (count=0 ; count<ZUKUNFT_MAX ; count++){
				zugliste_max[count].von_x  = zugliste_copy[count].von_x;
				zugliste_max[count].von_y  = zugliste_copy[count].von_y;
				zugliste_max[count].nach_x = zugliste_copy[count].nach_x;
				zugliste_max[count].nach_y = zugliste_copy[count].nach_y;
				if (zugliste_max[count].von_x == -1) {break;};
			}

			if (tiefe == start_tiefe) {
				//printf("\n * %d * von %d, %d nach %d, %d Punkte %d\n", j,  zugliste_max[0].von_x, zugliste_max[0].von_y, zugliste_max[0].nach_x, zugliste_max[0].nach_y, gegenpunkte);
				/*                selected = 1;*/

				/*                select_old_x = select_x;*/
				/*                select_old_y = select_y;*/
				/*                    select_x = zugliste_max[0].von_x;*/
				/*                select_y = zugliste_max[0].von_y;*/
				/*                show_selected_piece(select_x, select_y, select_old_x, select_old_y);*/
			}

		}

		// hier testen
		//	if ( (max_tmp >= (tiefe+1)*MAX_TMP_INIT) && (-gegenpunkte <= max_tmp)) 
		if (tiefe!=start_tiefe && (max_tmp >= (tiefe+1)*MAX_TMP_INIT) && (-gegenpunkte <= max_tmp)) {
			//printf ("%i",tiefe);
			break;
		}
	}

	if (zugliste_max[0].von_x == -1) {
		for (count=0 ; count<ZUKUNFT_MAX ; count++){
			zugliste_max[count].von_x  = zugliste[count].von_x;
			zugliste_max[count].von_y  = zugliste[count].von_y;
			zugliste_max[count].nach_x = zugliste[count].nach_x;
			zugliste_max[count].nach_y = zugliste[count].nach_y;
			if (zugliste_max[count].von_x == -1) {break;};
		}

		if (abbruchart == 1) {								// G
			//printf(" AG%d ", tiefe);
			zug_anhaengen(zugliste_max, &zug_liste[index]); // index oder 0
		} else if (abbruchart == 2) {							// g
			//printf(" Ag%d ", tiefe);
			zug_anhaengen(zugliste_max, &zug_liste[index]); // index oder 0
		} else {
			if ((start_tiefe-tiefe) % 2 == 0) {						// V
				//printf(" AV%d ", tiefe);
				zug_anhaengen(zugliste_max, &zug_liste[index_liste[0]]); // index oder index_liste[0]
			} else {									// v
				//printf(" Av%d ", tiefe);
				zug_anhaengen(zugliste_max, &zug_liste[index_liste[0]]); // index oder index_liste[0]
			}
		}
	}

	*gegenpunkte_max_p = -gegenpunkte_max;

	for (count=0 ; count<ZUKUNFT_MAX ; count++) {
		zugliste[count].von_x  = zugliste_max[count].von_x;
		zugliste[count].von_y  = zugliste_max[count].von_y;
		zugliste[count].nach_x = zugliste_max[count].nach_x;
		zugliste[count].nach_y = zugliste_max[count].nach_y;
		if (zugliste[count].von_x == -1) {break;};
	}

	return;
}


// returns score, changes zug_p
int computer_zug(farbname_t farbe, int tiefe, brett_t *brett_p, zug_t *zug_p) {
	int im_schach_int;

	int max_tmp=INIT_FACTOR*MAX_TMP_INIT;
	int bewertung_int = 0;

	zug_t zukunft[ZUKUNFT_MAX];

	zukunft[0].von_x	= -1; // leer
	zukunft[1].von_x	= -1; // leer
	zukunft[2].von_x	= -1; // leer

	im_schach_int = im_schach(-farbe, brett_p, zug_p);
	if 	(im_schach_int == 1) {
		// fremden König schlagen
		if (farbe == computer) {
			bewertung_int = MAX_TMP_INIT;
			//	    if (set_message) {
			//		strcpy(message,"mate");
			//	    }
		}

		zukunft[0].von_x	= zug_p->von_x;
		zukunft[0].von_y	= zug_p->von_y;
		zukunft[0].nach_x	= zug_p->nach_x;
		zukunft[0].nach_y	= zug_p->nach_y;

		return bewertung_int;
	}

	bester_zug(farbe, tiefe, tiefe, max_tmp, brett_p, zukunft, &bewertung_int);

	if (zukunft[0].von_x != -1) {
		zug_p->von_x  = zukunft[0].von_x;
		zug_p->von_y  = zukunft[0].von_y;
		zug_p->nach_x = zukunft[0].nach_x;
		zug_p->nach_y = zukunft[0].nach_y;
	} else {
		printf("\nDieser Fall sollte nie auftreren!\n\n");
		exit(1);
	}

	if (bewertung_int >= -MAX_TMP_INIT_SMALL) {
		bewertung_int = -MAX_TMP_INIT;
		//if (set_message) {
			//strcpy(message,"you may win");
		//}
	} else if (bewertung_int <= MAX_TMP_INIT_SMALL) {
		bewertung_int = MAX_TMP_INIT;
		//if (set_message) {
			//strcpy(message,"you will loose");
		//}
	}
	return bewertung_int;
}


void umgebung(int x, int y, bool mit_mitte, umgebung_t *umgebung_p) {
	int u;
	int index=0;

	u = x + 8*y;
	if (mit_mitte) {
		(*umgebung_p)[index++]=u;
	};

	if (x < 7) {
		(*umgebung_p)[index++]=u+1;
		if (y > 0) {
			(*umgebung_p)[index++]=u-7;
		}
		if (y < 7) {
			(*umgebung_p)[index++]=u+9;
		}
	}
	if (x > 0) {
		(*umgebung_p)[index++]=u-1;
		if (y > 0) {
			(*umgebung_p)[index++]=u-9;
		}
		if (y < 7) {
			(*umgebung_p)[index++]=u+7;
		}
	}
	if (y < 7) {
		(*umgebung_p)[index++]=u+8;
	}
	if (y > 0) {
		(*umgebung_p)[index++]=u-8;
	}

	(*umgebung_p)[index++]=-1;
}


void init_umgebung (void) {
	int x, y;
	bool mit_mitte;
	for (mit_mitte=0; mit_mitte<2 ; mit_mitte++) {
		for (y=0 ; y < 8 ; y++) {
			for (x=0 ; x < 8 ; x++) {
				// umgebung initialisieren
				umgebung(x, y, mit_mitte, &(umgebung_liste[x + 8*y + 64*mit_mitte]));
				if (mit_mitte == 0) {
					// Koordinaten-Arrays initialisieren
					x_koordinate[x+8*y]	= x;
					y_koordinate[x+8*y]	= y;
				}
			}
		}
	}
}


void newGame(bool _immune_pawns) {
	int index;
	int mensch_farbe;

	immune_pawns = _immune_pawns;

	// #####################
	// ### Einstellungen ###
	// #####################

	mensch_farbe = 1;			// weiss

	// ############
	// ### init ###
	// ############

	if (mensch_farbe == weiss) {
		computer = -1;
		mensch = 1;
	} else {
		computer = 1;
		mensch = -1;
	}

	init_umgebung();

	grundwert[0]	= 0; 	// leeres Feld
	grundwert[1]	= 10;
	grundwert[2]	= 40;
	grundwert[3]	= 50;
	grundwert[4]	= 30;
	grundwert[5]	= 400;	// 200
	grundwert[6]	= 300;

	for (index=0; index<64 ; index++){
		brett[index] 		= brett_start[index];
	}
}


/* Functions for access from JS */
zug_t zug_temp;
int punkte_int_temp;
void set_zug_temp(int von_x, int von_y, int nach_x, int nach_y)
{
	zug_temp.von_x = von_x;
	zug_temp.von_y = von_y;
	zug_temp.nach_x = nach_x;
	zug_temp.nach_y = nach_y;
}
int von_x(zug_t zug) {return zug.von_x;}
int von_y(zug_t zug) {return zug.von_y;}
int nach_x(zug_t zug) {return zug.nach_x;}
int nach_y(zug_t zug) {return zug.nach_y;}
brett_t* get_brett() {return &brett;}
zug_t* get_zug_temp() {return &zug_temp;}
int* get_punkte_int_temp() {return &punkte_int_temp;}


bool legal(int vonx, int vony, int nachx, int nachy, brett_t *brett_p) {
	int refz[LIST_LEN];
	int refs[LIST_LEN];

	int nach,index;

	// zug für figur legal?
	nach = nachx+8 * nachy;
	erlaubte_zuege(vonx, vony, brett_p, refz, refs );

	for (index=0 ; refz[index] != -1; index++){
		if (nach == refz[index]) {
			return TRUE;
		}
	}

	for (index=0 ; refs[index] != -1; index++){
		if (nach == refs[index]) {
			return TRUE;
		}
	}

	return FALSE;
}


bool hat_koenig(farbname_t farbe, brett_t *brett_p){
    bool hat_koenig=FALSE;
    int i;

	for (i=0 ; i<64 ; i++){
		if (farbe* ((*brett_p)[i]) == 6) {
			hat_koenig=TRUE;
			break;
		};
	}

    return hat_koenig;
}
