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

function indexes_to_field(x, y) {
    return String.fromCharCode('a'.charCodeAt(0) + x) + (8 - y);
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
    var position = board_to_position(board);
    board.position(position, true);
}

function board_to_position(board) {
    var x,y;
    var position = {};
    var piece_mapping = {
        1: 'P',
        2: 'B',
        3: 'N',
        4: 'R',
        5: 'Q',
        6: 'K'
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
    return position;
}

function position_to_board(position, board) {
    var x,y;
    var piece_mapping = {
        P: '1',
        B: '2',
        N: '3',
        R: '4',
        Q: '5',
        K: '6'
    };

    for (y=0; y<8; y++) {
        for (x=0; x<8; x++) {
            // get piece from chessboard.js
            var field = indexes_to_field(x, y);
            var piece = position[field];

            // convert to piece_code for _brett
            var piece_code;
            if (piece === undefined) {
                piece_code = 0;
            } else {
                piece_code = piece_mapping[piece[1]];
                if (piece[0] === 'b') {
                    piece_code *= -1;
                }
            }

            // write result to _brett
            var pointer = _brett + ((x+y*8) << 2);
            HEAP32[pointer >> 2] = piece_code;
        }
    }
}

function resize(board) {
    var width = Math.min($(window).width(), $(window).height());
    $("#board").width(Math.floor(width/8)*8 + "px");
    board.resize();
}


function restart(board) {
    _newGame();
    update_html_board(board);
}


var board;
$(document).ready(function(){
    var weiss = 1, schwarz = -1;
    var tiefe = parseInt($('#difficulty').val());
    var undo_stack = [];

    function computer_turn() {
        if ((_hat_koenig(weiss, _brett) != 1) || (_hat_koenig(schwarz, _brett) != 1)) {
            // game over
            console.log('game over');
            return;
        }
        _computer_zug(schwarz, tiefe, _brett, _zug_temp, _punkte_int_temp, 1);
        _anwenden(_brett, _zug_temp);

        var from = indexes_to_field(_von_x(_zug_temp), _von_y(_zug_temp));
        var to = indexes_to_field(_nach_x(_zug_temp), _nach_y(_zug_temp));
        board.move(from + '-' + to);
    }

    function on_move_end(from_field, to_field) {
        update_html_board(board); // remove eleminated pieces
    }

    function on_snap(from_field, to_field, piece) {
        update_html_board(board); // remove eleminated pieces
        window.setTimeout(computer_turn, 250);
    }

    function on_drop(from_field, to_field, piece) {
        undo_stack.push(board_to_position(_brett));
        zug = make_zug(from_field, to_field);
        if (zug === 'invalid') {
            return 'snapback';
        }
        _anwenden(_brett, zug);
    }

    board = new ChessBoard('board', {
        pieceTheme: 'lib/chessboardjs/img/chesspieces/wikipedia/{piece}.png',
        draggable: true,
        moveSpeed: 2000,
        onSnapEnd: on_snap,
        onDrop: on_drop,
        onMoveEnd: on_move_end
    });


    resize(board);

    $(window).resize(function(){
        resize(board);
    });

    restart(board);

    $('#restart').click(function () {restart(board)});
    $('#undo').click(function () {
        var position = undo_stack.pop();
        console.log(position);
        position_to_board(position);
        update_html_board(board);
    });
    $('#difficulty').change(function () {tiefe = parseInt(this.value)});

    // zug legal / erlaubt
//    log( _legal(1, 1, 2, 1, _brett) );
//    log( _legal(1, 1, 1, 2, _brett) );
});

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
