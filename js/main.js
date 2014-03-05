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

function print_brett() {
    var x,y, str = "";
    for (y=0; y<8; y++) {
        for (x=0; x<8; x++) {
            // pointer to 32-bit var
            var pointer=(_brett+((x+y*8)<<2));
            str += ((HEAP32[((pointer)>>2)]).toString() + ", ");
        }
        str += ("\n");
    }
    return str;
}

function log(text)
{
    var htmls = [];
    var lines = text.split(/\n/);
    // The temporary <div/> is to perform HTML entity encoding reliably.
    //
    // document.createElement() is *much* faster than jQuery('<div/>')
    // http://stackoverflow.com/questions/268490/
    //
    // You don't need jQuery but then you need to struggle with browser
    // differences in innerText/textContent yourself
    var tmpDiv = jQuery(document.createElement('div'));
    for (var i = 0 ; i < lines.length ; i++) {
        htmls.push(tmpDiv.text(lines[i]).html());
    }
    $('#msg').append(htmls.join("<br>"));
}

function make_zug(from_field, to_field) {
	_set_zug_temp(
		from_field.charCodeAt(0) - 'a'.charCodeAt(0),
		parseInt(from_field[1]) - 1,
		to_field.charCodeAt(0) - 'a'.charCodeAt(0),
		parseInt(to_field[1]) - 1
	);
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
			var field = String.fromCharCode('a'.charCodeAt(0) + x) + (y + 1);
			var color = piece_code < 0 ? 'w' : 'b';
			var piece = piece_mapping[Math.abs(piece_code)];
			position[field] = color + piece;
        }
    }
	board.position(position, true);
}

/* Looks like this is not even necessary
function update_internal_board(board) {
	var piece_mapping = {
		P: 1,
		B: 2,
		N: 3,
		R: 4,
		Q: 5,
		K: 6,
	};

	// TODO: clear board before setting pieces
	var position = board.position();
	for (field in position) {
		console.log(field);
		var x = field[0].charCodeAt(0) - 'a'.charCodeAt(0);
		var y = parseInt(field[1]) - 1;
		var color = position[field][0] === 'w' ? -1 : 1;
		var piece = piece_mapping[position[field][1]];
		console.log(x, y, color, piece);

		var pointer = _brett + ((x+y*8) << 2);
		HEAP32[pointer >> 2] = color * piece;
	}
}
*/

$(document).ready(function(){
    var weiss = 1, schwarz = -1;
    var tiefe = 1;
    var farbe = weiss;
	var board;

	function computer_turn() {
		if ((_hat_koenig(weiss, _brett) != 1) || (_hat_koenig(schwarz, _brett) != 1)) {
			// game over
			console.log('game over');
			return;
		}
		_computer_zug(farbe, tiefe, _brett, _zug_temp, _punkte_int_temp, 1);
		log(print_zug(_zug_temp));
		_anwenden(_brett, _zug_temp);
		update_html_board(board);
	}

	function on_move_end() {
	}

	function on_snap(from_field, to_field, piece) {
		zug = make_zug(from_field, to_field);
		_anwenden(_brett, zug);
		computer_turn();
	}

	board = new ChessBoard('board', {
		pieceTheme: 'lib/chessboardjs/img/chesspieces/wikipedia/{piece}.png',
		draggable: true,
		moveSpeed: 2000,
		onMoveEnd: on_move_end,
		onSnapEnd: on_snap
	});

    log("starting...!\n");

    _newGame();
	update_html_board(board);
    log(print_brett());

    // zug legal / erlaubt
//    log( _legal(1, 1, 2, 1, _brett) );
//    log( _legal(1, 1, 1, 2, _brett) );

    //console.log(_hat_koenig(weiss, _brett));
    //console.log(_hat_koenig(schwarz, _brett));

    log(print_brett());
});
