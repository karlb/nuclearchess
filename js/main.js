function print_zug(zug) {
    return(
        "zug von " +
        String.fromCharCode('A'.charCodeAt(0) + _von_x(zug)) +
        (1 + _von_y(zug)).toString() + 
        " nach " +
        String.fromCharCode('A'.charCodeAt(0) + _nach_x(zug)) +
        (1 + _nach_y(zug)).toString() +
        "\n"
    );
}

function make_zug(from_field, to_field) {
	var from_x = from_field.charCodeAt(0) - 'a'.charCodeAt(0),
		from_y = 8 - parseInt(from_field[1]),
		to_x = to_field.charCodeAt(0) - 'a'.charCodeAt(0),
		to_y = 8 - parseInt(to_field[1]);
    if (!_legal(from_x, from_y, to_x, to_y, _brett)) {
		return 'invalid';
	}
	_set_zug_temp(from_x, from_y, to_x, to_y);
	return _zug_temp;
}

function update_html_board(board) {
    var x,y;
	var position = {};
	var piece_mapping = {
		1: 'P', 
		2: 'B', 
		3: 'N', 
		4: 'R', 
		5: 'Q', 
		6: 'K', 
	};

    for (y=0; y<8; y++) {
        for (x=0; x<8; x++) {
            // pointer to 32-bit var
			var pointer = _brett + ((x+y*8) << 2);
            var piece_code = HEAP32[pointer >> 2];

			if (piece_code === 0) {
				continue;
			}
			var field = String.fromCharCode('a'.charCodeAt(0) + x) + (8 - y);
			var color = piece_code > 0 ? 'w' : 'b';
			var piece = piece_mapping[Math.abs(piece_code)];
			position[field] = color + piece;
        }
    }
	board.position(position, true);
}

$(document).ready(function(){
    var weiss = 1, schwarz = -1;
    var tiefe = 1;
	var board;

	function computer_turn() {
		if ((_hat_koenig(weiss, _brett) != 1) || (_hat_koenig(schwarz, _brett) != 1)) {
			// game over
			console.log('game over');
			return;
		}
		_computer_zug(schwarz, tiefe, _brett, _zug_temp, _punkte_int_temp, 1);
		_anwenden(_brett, _zug_temp);
	}

	function on_snap(from_field, to_field, piece) {
		update_html_board(board);
	}

	function on_drop(from_field, to_field, piece) {
		zug = make_zug(from_field, to_field);
		if (zug === 'invalid') {
			return 'snapback';
		}
		_anwenden(_brett, zug);
		computer_turn();
	}

	board = new ChessBoard('board', {
		pieceTheme: 'lib/chessboardjs/img/chesspieces/wikipedia/{piece}.png',
		draggable: true,
		moveSpeed: 2000,
		onSnapEnd: on_snap,
		onDrop: on_drop
	});

    _newGame();
	update_html_board(board);

    // zug legal / erlaubt
//    log( _legal(1, 1, 2, 1, _brett) );
//    log( _legal(1, 1, 1, 2, _brett) );
});
