#include <ctype.h>

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


int parse_fen(char *fen, int *farbe, int *tiefe) {
	int i = 0;
	int fen_i;

	for (fen_i=0; fen[fen_i] != '-'; fen_i++) {
		switch (fen[fen_i]) {
			case 'p': brett[i] = -PAWN; i++; break;
			case 'r': brett[i] = -ROOK; i++; break;
			case 'n': brett[i] = -KNIGHT; i++; break;
			case 'b': brett[i] = -BISHOP; i++; break;
			case 'q': brett[i] = -QUEEN; i++; break;
			case 'k': brett[i] = -KING; i++; break;
			case 'P': brett[i] = PAWN; i++; break;
			case 'R': brett[i] = ROOK; i++; break;
			case 'N': brett[i] = KNIGHT; i++; break;
			case 'B': brett[i] = BISHOP; i++; break;
			case 'Q': brett[i] = QUEEN; i++; break;
			case 'K': brett[i] = KING; i++; break;
			case 'u': brett[i] = NUKE; i++; break;
			case '/': break;
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
						brett[i] = 0;
						i++;
					}
				}
				break;
			default: return -1;
		}
		//printf("%c %d\n", fen[fen_i], i);
	}
	char farbe_c, immune_pawns_c, dead_squares_c;
	int variables_read = sscanf(&fen[fen_i], "-%d-%c-%c-%c\n", tiefe, &farbe_c, &immune_pawns_c, &dead_squares_c);
	if (variables_read != 4) {
		printf("sscanf returned %d\n", variables_read);
		exit(1);
	};
	if (farbe_c == 'w') {
		*farbe = 1;
	} else if (farbe_c == 'b') {
		*farbe = -1;
	} else {
		printf("Bad color in fen string");
		exit(1);
	}
	immune_pawns = immune_pawns_c - '0';
	dead_squares = dead_squares_c - '0';
	return 0;
}



void write_fen(FILE *fp, int farbe, int tiefe) {
	int empty_fields = 0;
	char pieces[] = ".pbnrqk";

	for (int i=0; i<64; i++) {
		if (i % 8 == 0 && i > 0) {
			putc('/', fp);
		}
		switch (brett[i]) {
			case 0:
				empty_fields++;
			   	break;
			case 100:
				putc('u', fp);
				break;
			default:
				{
					int abs_val = abs(brett[i]);
					if (abs_val > 6) {
						printf("Bad case\n");
						exit(1);
					}
					if (brett[i] > 0) {
						putc(toupper(pieces[abs_val]), fp);
					} else {
						putc(pieces[abs_val], fp);
					}
				}
		}
		if (i == 63 || brett[i + 1] != 0) {
			fprintf(fp, "%d", empty_fields);
			empty_fields = 0;
		}
	}
	fprintf(fp, "-%d-%c-%c-%c\n", tiefe, (farbe == 1) ? 'w' : 'b', immune_pawns ? 't': 'f', dead_squares ? 't' : 'f');
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


int main(int argc, char *argv[]) {
	if (argc >= 2) {
		if (strcmp(argv[1], "bench") == 0) {
			bench();
		} else if (strcmp(argv[1], "new_board") == 0) {
			int _immune_pawns = strtol(argv[2], NULL, 10);
			int _dead_squares = strtol(argv[3], NULL, 10);

			newGame(_immune_pawns, _dead_squares);
			//save_board();
		} else if (strcmp(argv[1], "make_turn") == 0) {
			zug_t zug;
			int farbe = strtol(argv[2], NULL, 10);
			int tiefe = strtol(argv[3], NULL, 10);
			//printf(">> %d %d\n", farbe, tiefe);

			newGame(FALSE, FALSE);
			//load_board();
			computer_zug(farbe, tiefe, &brett, &zug);
			anwenden(&brett, &zug);
			//save_board();

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
			int tiefe;
			newGame(FALSE, FALSE);
			if (parse_fen(argv[2], &farbe, &tiefe)) {
				printf("Bad fen notation\n");
				exit(1);
			}
			punkte(farbe, &brett, &punkte_weiss, &punkte_schwarz);
			printf("%d\n", farbe * (punkte_weiss - punkte_schwarz));
			write_fen(stdout, farbe, 1);
		} else {
			printf("Bad command '%s'\n", argv[1]);
		}
	} else {
		test();
	}
}

