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

function field_to_indexes(field) {
    var x = field.charCodeAt(0) - 'a'.charCodeAt(0);
    var y = 8 - parseInt(field[1]);
    return [x, y]
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

/* Returns true if board changed */
function update_html_board(board, show_animation) {
    var position = board_to_position(board);
    if (ChessBoard.objToFen(position) === ChessBoard.objToFen(board.position())) {
        return false;
    } else {
        board.position(position, show_animation);
        return true;
    }
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
    var width = Math.min($(window).width(), $(window).height() - $('#top-bar').height());
    var board_border = 4;
    width -= 8; // margin
    $("#board, #top-bar").width(
        Math.floor((width - board_border)/8)*8 + board_border + "px");
    board.resize();
}

function random_x_pos() {
    var x = Math.floor(Math.random() * 8);
    return String.fromCharCode('a'.charCodeAt(0) + x);
}

function shuffle(board, row) {
    var pos = board.position();

    for (var i=0; i<50; i++) {
        var from = random_x_pos() + row;
        var to = random_x_pos() + row;
        var tmp = pos[to];
        pos[to] = pos[from];
        pos[from] = tmp;
    }

    board.position(pos);
    position_to_board(pos);
}


var board;
$(document).ready(function(){
    var weiss = 1, schwarz = -1;
    var thinking_depth = 1;
    var undo_stack = [];
    var nuclear_strike = false;  // either 'false' or the field where the strike hit
    var waiting_for_player = true;
    var game_over;
    var move_end_callbacks = [];
    var player_color = 'white';

    function restart() {
        _newGame();
        update_html_board(board, false);
        game_over = false;
        $('#winner').text('');
    }

    function computer_turn() {
        if (game_over) {
            return;
        }
        $('#thinking').show();
        setTimeout(function() {
            _computer_zug(player_color === 'white' ? schwarz : weiss,
                          thinking_depth, _brett, _zug_temp, _punkte_int_temp, 1);
            $('#thinking').hide();
            _anwenden(_brett, _zug_temp);

            var from = indexes_to_field(_von_x(_zug_temp), _von_y(_zug_temp));
            var to = indexes_to_field(_nach_x(_zug_temp), _nach_y(_zug_temp));
            check_for_nuclear_strike(to);
            board.move(from + '-' + to);
        }, 10);
    }

    function check_for_nuclear_strike(to_field) {
        if (board.position()[to_field] !== undefined) {
            nuclear_strike = to_field;
        }
    }

    function show_nuclear_strike(after_strike_callback) {
        if (after_strike_callback === undefined) {
            after_strike_callback = function () {};
        }
        if (nuclear_strike) {
            // select surrounding fields
            var indexes = field_to_indexes(nuclear_strike);
            var center_x = indexes[0],
                center_y = indexes[1];
            var x, y;
            var piece_classes = [];
            for (y=-1; y<=1; y++) {
                for (x=-1; x<=1; x++) {
                    if (
                        (center_x + x >= 0) && 
                        (center_x + x < 8) && 
                        (center_y + y >= 0) && 
                        (center_y + y < 8) 
                    )
                    {
                        field = indexes_to_field(center_x + x, center_y + y);
                        piece_classes.push('.square-' + field + ' img');
                    }
                }
            }
            var pieces = $(piece_classes.join(', '));
            pieces.addClass('shake shake-hard');
            setTimeout(function() {
                pieces.removeClass('shake shake-hard');
                update_html_board(board); // remove eleminated pieces
                after_strike_callback();
            }, 1000);
        } else {
            // update only necessary for castling
            if (update_html_board(board)) {
                // wait until castling animation has finished
                move_end_callbacks.push(after_strike_callback);
            } else {
                after_strike_callback();
            }
        }
        nuclear_strike = false;
    }

    function check_game_over() {
        if (_hat_koenig(weiss, _brett) != 1 && _hat_koenig(schwarz, _brett) != 1) {
            game_over = true;
            $('#winner').text('Draw game');
        }
        else if (_hat_koenig(weiss, _brett) != 1) {
            game_over = true;
            $('#winner').text('Black is winner!');
        }
        else if (_hat_koenig(schwarz, _brett) != 1) {
            game_over = true;
            $('#winner').text('White is winner!');
        }
    }

    function on_move_end(from_field, to_field) {
        show_nuclear_strike();
        check_game_over();
        waiting_for_player = true;
        history.replaceState({}, '', '#' + board.fen() + '-' + thinking_depth + '-' + player_color);

        $.each(move_end_callbacks, function(i, callback) {
            callback();
        });
        move_end_callbacks = [];
    }

    function on_snap(from_field, to_field, piece) {
        show_nuclear_strike(function () {
            check_game_over();
            window.setTimeout(computer_turn, 250);
        });
    }

    function on_drop(from_field, to_field, piece) {
        // ignore user action if move is not allowed
        zug = make_zug(from_field, to_field);
        if (zug === 'invalid') {
            return 'snapback';
        }

        undo_stack.push(board_to_position(_brett));
        waiting_for_player = false;
        check_for_nuclear_strike(to_field);
        _anwenden(_brett, zug);
    }

    board = new ChessBoard('board', {
        pieceTheme: 'lib/chessboardjs/img/chesspieces/wikipedia/{piece}.png',
        draggable: true,
        moveSpeed: 1600,
        onSnapEnd: on_snap,
        onDrop: on_drop,
        onMoveEnd: on_move_end,
        onDragStart: function (from, piece) {
            // picking up pieces it not allowed when ...
            if (
                piece[0] !== player_color[0]  // it's not his piece
                || game_over  // the game has already ended
                || waiting_for_player === false  // animation in progress or computer's turn
            ) {
                return false;
            }
        }
    });


    resize(board);

    $(window).resize(function(){
        resize(board);
    });

    restart(board);
    // set position from url if a fen-string is in the URL fragment part
    if (window.location.hash) {
        var url_data = window.location.hash.slice(1).split('-');
        var fen = url_data[0];
        thinking_depth = url_data[1];
        player_color = url_data[2];
        board.position(fen, false);
        position_to_board(board.position());
        board.orientation(player_color);
    }
    $('#difficulty').val(thinking_depth);

    $('#play').click(function () {
        player_color = $('input[name=play-as]:checked').val();
        restart(board);
        if ($('#shuffle').is(':checked')) {
            shuffle(board, 8);
            shuffle(board, 1);
        }
        board.orientation(player_color);
        if (player_color == 'black') {
            computer_turn();
        }
    });
    $('#undo').click(function () {
        var position = undo_stack.pop();
        position_to_board(position);
        update_html_board(board);
        game_over = false;
        $('#winner').text('');
    });
    $('#difficulty').change(function () {thinking_depth = parseInt(this.value)});
});

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
