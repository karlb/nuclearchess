#include "atomschach.c"

void sub_main (farbname_t farbe,int tiefe,brett_t *brett_p) {
	zug_t zug;
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


int parse_fen(char *fen) {
	int x = 0;
	int y = 0;

	for (int fen_i=0; fen[fen_i] != '-'; fen_i++) {
		int i = x + 8 * y;
		switch (fen[fen_i]) {
			case 'p': brett[i] = -PAWN; x++; break;
			case 'r': brett[i] = -ROOK; x++; break;
			case 'n': brett[i] = -KNIGHT; x++; break;
			case 'b': brett[i] = -BISHOP; x++; break;
			case 'q': brett[i] = -QUEEN; x++; break;
			case 'k': brett[i] = -KING; x++; break;
			case 'P': brett[i] = PAWN; x++; break;
			case 'R': brett[i] = ROOK; x++; break;
			case 'N': brett[i] = KNIGHT; x++; break;
			case 'B': brett[i] = BISHOP; x++; break;
			case 'Q': brett[i] = QUEEN; x++; break;
			case 'K': brett[i] = KING; x++; break;
			case 'u': brett[i] = NUKE; x++; break;
			case '/':
				y++;
				x = 0;
				break;
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
				{
					int empty_fields = strtol(&fen[fen_i], NULL, 10);
					for (int i2=0; i2 < empty_fields; i2++) {
						brett[i + i2] = 0;
						x++;
					}
				}
				break;
			default: return -1;
		}
		//printf("%c %d %d %d\n", fen[fen_i], i, x, y);
	}
	return 0;
}


void test() {
	int farbe = 1;
	int tiefe = 1;

	printf("classic\n");
	newGame(FALSE, FALSE);
    sub_main(farbe, tiefe, &brett);

	printf("\nimmune_pawns\n");
	newGame(TRUE, FALSE);
    sub_main(farbe, tiefe, &brett);
}

void bench() {
	int farbe = 1;
	int tiefe = 4;

	newGame(FALSE, FALSE);
    sub_main(farbe, tiefe, &brett);
}


void save_board() {
	FILE *fp = fopen("board.csv", "w");
	for (int i=0; i<=64; i++) {
		fprintf(fp, "%d,", brett[i]);
	}
	fprintf(fp, "\n%d,%d\n,", immune_pawns, dead_squares);
	fclose(fp);
}


void load_board() {
	FILE *fp = fopen("board.csv", "r");
	for (int i=0; i<=64; i++) {
		fscanf(fp, "%d,", &brett[i]);
	}
	fscanf(fp, "\n%d,%d\n,", &immune_pawns, &dead_squares);
	fclose(fp);
}


int main(int argc, char *argv[]) {
	if (argc >= 2) {
		if (strcmp(argv[1], "bench") == 0) {
			bench();
		} else if (strcmp(argv[1], "new_board") == 0) {
			int _immune_pawns = strtol(argv[2], NULL, 10);
			int _dead_squares = strtol(argv[3], NULL, 10);

			newGame(_immune_pawns, _dead_squares);
			save_board();
		} else if (strcmp(argv[1], "make_turn") == 0) {
			zug_t zug;
			int farbe = strtol(argv[2], NULL, 10);
			int tiefe = strtol(argv[3], NULL, 10);
			//printf(">> %d %d\n", farbe, tiefe);

			newGame(FALSE, FALSE);
			load_board();
			computer_zug(farbe, tiefe, &brett, &zug);
			anwenden(&brett, &zug);
			save_board();

			if (!hat_koenig(-farbe, &brett)) {
				if (hat_koenig(-farbe, &brett)) {
					printf("%d wins\n", farbe);
				} else {
					printf("draw\n");
				}
				return 1;
			}
			return 0;
		} else if (strcmp(argv[1], "score") == 0) {
			int punkte_weiss, punkte_schwarz;
			int farbe = weiss;
			newGame(FALSE, FALSE);
			if (parse_fen(argv[2])) {
				printf("Bad fen notation\n");
				exit(1);
			}
			punkte(farbe, &brett, &punkte_weiss, &punkte_schwarz);
			printf("%d\n", farbe * (punkte_weiss - punkte_schwarz));
		} else {
			printf("Bad command '%s'\n", argv[1]);
		}
	} else {
		test();
	}
}

