/* Code not needed for basic AI */

/*history_t history[HISTORY_MAX];*/

void zuglesbar_func(zug_t *zug_p, farbname_t farbe, brett_t *brett_p, zug_lesbar_t zuglesbar) {
    char bild_c;
    int vonx, vony, nachx, nachy;
    char schach=' ';
    char schachmatt=' ';
    char zugart='-';
    int figurf, figur;
    brett_t brett_copy;
    int index;
    zug_t dummy_zug;
    int schach_pos = 0;

    vonx = zug_p->von_x;
    vony = zug_p->von_y;
    nachx = zug_p->nach_x;
    nachy = zug_p->nach_y;

    if (vonx == -1) {
	sprintf(zuglesbar,"%s", "");
	return;
    }

    for (index=0; index<64 ; index++){
	brett_copy[index]= (*brett_p)[index];
    }

    if ((*brett_p)[nachx+8*nachy] != 0) {
	zugart = '*';
    }

    figurf 	= brett[vonx + 8*vony];
    figur	= abs(figurf);
    bild_c	= bild[figur];

    // test ob danach im schach
    anwenden(&brett_copy, zug_p);
    schach_pos = im_schach(-farbe, &brett_copy, &dummy_zug);
    if (schach_pos == 1) {
	schach = '+';
    }
    if (schach_pos == -1) {
	schach = '+';
	schachmatt = '+';
    }

    sprintf(zuglesbar,"%c%c%i%c%c%i%c%c", bild_c, vonx +'a', 8-vony, zugart, nachx+'a', 8-nachy, schach, schachmatt);
}


float bewertung_float(int bewertung_int, int farbe, int tiefe) {
	float bewertung_f = 0;

	int offset = 0;

	int drohwert_offset1 = 0;
	int drohwert_offset2 = -140;
	int drohwert_offset3 = +140;
	int drohwert_offset4 = 0;

	if (bewertung_int != 0 && bewertung_int > MAX_TMP_INIT && bewertung_int < -MAX_TMP_INIT) {

		if (mensch == weiss) {
			if (tiefe % 2) {
				offset = drohwert_offset1;
			} else {
				offset = drohwert_offset2;
			}
		} else {
			if (tiefe % 2) {
				offset = drohwert_offset3;
			} else {
				offset = drohwert_offset4;
			}
		}
		offset += 50;
	}

	bewertung_int += offset;

	if (bewertung_int > 0) {
		bewertung_f = log10(bewertung_int);
	} else if (bewertung_int < 0) {
		bewertung_f = -log10(-bewertung_int);
	} else {
		bewertung_f = 0;
	}

	if (bewertung_f > 4) {
		bewertung_f = 4;
	} else if (bewertung_f < -4) {
		bewertung_f = -4;
	}

	bewertung_f = bewertung_f*25;

	return bewertung_f;
}


// ############
// ### main ###
// ############
void sub_main (farbname_t farbe,int tiefe,brett_t *brett_p) {
	int i;
	zug_t zug;
	zug_lesbar_t zuglesbar, end_text;

	while (TRUE){

		/*        brett_anzeigen(TRUE, TRUE);*/

		zug.von_x = -1;	// init wegen change color

		if (hat_koenig(farbe, brett_p) && hat_koenig(-farbe, brett_p)) {
			if (farbe == computer) {
				computer_zug(farbe, tiefe, brett_p, &zug, TRUE);
				/*                letzter_zug_mensch = FALSE;*/
			} else {
				if (computer_gegen_computer == TRUE){
					computer_zug(farbe, tiefe, brett_p, &zug, TRUE);
					/*                    letzter_zug_mensch = FALSE;*/
				} else {
					/*                        mensch_zug(brett_p, &zug);*/
					/*                    letzter_zug_mensch = TRUE;*/
					if (zug.von_x == -1) { // change color
						/*                        computer_zug(farbe, tiefe, brett_p, &zug, TRUE);*/
						/*                        letzter_zug_mensch = FALSE;*/
					}
				};
			}

			/*            zuglesbar_func(&zug, farbe, brett_p, zuglesbar);*/

			/*            for (i=0; history[i].von_x != -1; i++) {};*/

			/*            history[i].von_x  = zug.von_x;*/
			/*            history[i].von_y  = zug.von_y;*/
			/*            history[i].nach_x = zug.nach_x;*/
			/*            history[i].nach_y = zug.nach_y;*/
			/*            strcpy(history[i].zuglesbar, zuglesbar);*/
			/*            history[i].punkte = punkte_int;*/

			/*            history[i+1].von_x  = -1;*/

			/*            zeige_zug(&zug);*/
			printf("zug von %c%d nach %c%d\n", 'A' + zug.von_x, 1 + zug.von_y, 'A' + zug.nach_x, 1 + zug.nach_y);
			/*    int von_y;*/
			/*    int nach_x;*/
			/*    int nach_y;*/
			anwenden(brett_p, &zug);  					// Brett aktualisieren
			farbe = -farbe;  						// Farbe aktualisieren
		} else {
			if (hat_koenig(-farbe, brett_p)) {
				if (computer == -farbe) {
					strcpy(message,"mate");
				} else {
					strcpy(message,"congratulation");
				}
				sprintf(end_text, "%s wins!", string_farbname(-farbe));
			} else if (hat_koenig(farbe, brett_p)) {
				if (computer == farbe) {
					strcpy(message,"mate");
				} else {
					strcpy(message,"congratulation");
				}
				sprintf(end_text, "%s wins!", string_farbname(farbe));
			} else {
				strcpy(message,"...");
				sprintf(end_text, "%s!", "draw");
			}
			break;
		}

	} //while TRUE

	/*    brett_anzeigen(TRUE, TRUE);*/

	/*    game_over(end_text);*/
}


/*int main(int argc, char *argv[])*/
/*{*/
/*    ReadCommandLine(argv);*/
/*    newGame();*/

// weit unten
/*    init_gui();*/

/*    sub_main(farbe, tiefe, &brett);*/
/*    computer_zug(schwarz, 3, &brett, &zug_temp, &punkte_int_temp, TRUE);*/
/*void erlaubte_zuege(int x, int y, brett_t *brett_p, int zug[], int schlag[]){*/

/*    return 0; //ok*/
/*}*/
