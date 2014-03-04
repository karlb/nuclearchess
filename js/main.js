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

function update_board(board) {
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
            var pointer = (_brett+((x+y*8)<<2));
            var piece_code = HEAP32[((pointer)>>2)];

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

$(document).ready(function(){
    var weiss = 1, schwarz = -1;
    var tiefe = 1;
    var farbe = weiss;
	var board;

	function take_turns() {
		if ((_hat_koenig(weiss, _brett) != 1) || (_hat_koenig(schwarz, _brett) != 1)) {
			return;
		}

		_computer_zug(farbe, tiefe, _brett, _zug_temp, _punkte_int_temp, 1);
		farbe = -farbe; 
		
		log(print_zug(_zug_temp));
		_anwenden(_brett, _zug_temp);

		update_board(board);
	}

	board = new ChessBoard('board', {
		pieceTheme: 'lib/chessboardjs/img/chesspieces/wikipedia/{piece}.png',
		moveSpeed: 2000,
		onMoveEnd: function() {console.log('bar'); take_turns();}
	});

    log("starting...!\n");

    _newGame();
	update_board(board);
    log(print_brett());

    // zug legal / erlaubt
//    log( _legal(1, 1, 2, 1, _brett) );
//    log( _legal(1, 1, 1, 2, _brett) );

    //console.log(_hat_koenig(weiss, _brett));
    //console.log(_hat_koenig(schwarz, _brett));

    log(print_brett());
});
