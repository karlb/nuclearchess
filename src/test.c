#include "atomschach.c"

void sub_main (farbname_t farbe,int tiefe,brett_t *brett_p) {
	int i;
	zug_t zug;
	zug_lesbar_t zuglesbar, end_text;
	bool letzter_zug_mensch;
	bool computer_gegen_computer = TRUE;

	while (TRUE){
		/*        brett_anzeigen(TRUE, TRUE);*/

		zug.von_x = -1;	// init wegen change color

		if (hat_koenig(farbe, brett_p) && hat_koenig(-farbe, brett_p)) {
			if (farbe == computer) {
				computer_zug(farbe, tiefe, brett_p, &zug);
				/*                letzter_zug_mensch = FALSE;*/
			} else {
				if (computer_gegen_computer == TRUE){
					computer_zug(farbe, tiefe, brett_p, &zug);
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

			printf("%c%d -> %c%d\n", 'A' + zug.von_x, 1 + zug.von_y, 'A' + zug.nach_x, 1 + zug.nach_y);
			anwenden(brett_p, &zug);
			farbe = -farbe;
		} else {
			if (hat_koenig(-farbe, brett_p)) {
				if (computer == -farbe) {
					//strcpy(message,"mate");
				} else {
					//strcpy(message,"congratulation");
				}
				//sprintf(end_text, "%s wins!", string_farbname(-farbe));
				printf("%d wins\n", -farbe);
			} else if (hat_koenig(farbe, brett_p)) {
				if (computer == farbe) {
					//strcpy(message,"mate");
				} else {
					//strcpy(message,"congratulation");
				}
				//sprintf(end_text, "%s wins!", string_farbname(farbe));
				printf("%d wins\n", farbe);
			} else {
				//strcpy(message,"...");
				//sprintf(end_text, "%s!", "draw");
				printf("draw\n");
			}
			break;
		}

	} //while TRUE

	/*    brett_anzeigen(TRUE, TRUE);*/

	/*    game_over(end_text);*/
}


void test() {
	int farbe = 1;
	int tiefe = 1;

	printf("classic\n");
	newGame(0);
    sub_main(farbe, tiefe, &brett);

	printf("\nimmune_pawns\n");
	newGame(1);
    sub_main(farbe, tiefe, &brett);
}

void bench() {
	int farbe = 1;
	int tiefe = 3;

	newGame(0);
    sub_main(farbe, tiefe, &brett);
}


int main(int argc, char *argv[]) {
	if (argc == 2) {
		if (strcmp(argv[1], "bench") == 0) {
			bench();
		}
	} else {
		test();
	}
}

