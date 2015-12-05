function indexes_to_field(x, y) {
    return String.fromCharCode('a'.charCodeAt(0) + x) + (8 - y);
}


function get_current_move() {
    var from = indexes_to_field(_von_x(_zug_temp), _von_y(_zug_temp));
    var to = indexes_to_field(_nach_x(_zug_temp), _nach_y(_zug_temp));
    return [from, to];
}

function apply_move(move, brett) {
    var zug = make_zug(move[0], move[1]);
    _anwenden(brett, zug);
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
            if (piece_code === 100) {
                position[field] = 'nuke';
                continue;
            } 
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
        K: '6',
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
            } else if (piece === 'nuke') {
                piece_code = 100;
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
    var phonegap = (document.URL.indexOf("http://") == -1);
    $('#bottom-bar').toggle(!phonegap);

    var width = Math.min(
            $(window).width() - 8,
            $(window).height() - $('#top-bar').outerHeight() - (phonegap ? 0 : $('#bottom-bar').outerHeight()));
    var board_border = 4;
    $("#board, #top-bar, #bottom-bar").width(
        Math.floor((width - board_border)/8)*8 + board_border + "px");
    board.resize();

    $('.app-store').toggle(($(window).width() > width + 300) && !phonegap);
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


function clear_winner() {
    $('#winner').fadeOut();
}

function set_winner(text) {
    $('#winner').text(text);
    $('#winner').fadeIn();
}


var board;
function main() {
    _brett = _get_brett();
    _zug_temp = _get_zug_temp();
    var weiss = 1, schwarz = -1;
    var thinking_depth = 1;
    var undo_stack = [];
    var nuclear_strike = false;  // either 'false' or the field where the strike hit
    var waiting_for_player = true;
    var game_over;
    var move_end_callbacks = [];
    var player_color = 'white';
    var immune_pawns = false;
    var dead_squares = false;

    function new_game() {
        _newGame(immune_pawns, dead_squares);
        update_html_board(board, false);
        game_over = false;
        clear_winner();
        $('.hint').removeClass('hint');
    }

    function calc_move(color, depth, cb) {
        if (game_over) {
            return;
        }
        $('#thinking').show();
        setTimeout(function() {
            _computer_zug(color === 'white' ? weiss : schwarz,
                          depth, _brett, _zug_temp, 1);
            $('#thinking').hide();
            var move = get_current_move();
            cb(move);
        }, 10);
    }

    function computer_turn() {
        calc_move(player_color === 'white' ? 'black' : 'white',
            thinking_depth,
            function(move) {
                apply_move(move, _brett);
                check_for_nuclear_strike(move[1]);
                board.move(move[0] + '-' + move[1]);
            }
        );
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
            set_winner('Draw game');
        }
        else if (_hat_koenig(weiss, _brett) != 1) {
            game_over = true;
            set_winner('Black wins!');
        }
        else if (_hat_koenig(schwarz, _brett) != 1) {
            game_over = true;
            set_winner('White wins!');
        }
    }

    function on_move_end(from_field, to_field) {
        show_nuclear_strike();
        check_game_over();
        waiting_for_player = true;
        history.replaceState({}, '',
            '#' + board.fen() +
            '-' + thinking_depth +
            '-' + player_color[0] +
            '-' + (immune_pawns ? 't' : 'f') +
            '-' + (dead_squares ? 't' : 'f')
        );

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
        $('.hint').removeClass('hint');

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

    // set position from url if a fen-string is in the URL fragment part
    if (window.location.hash) {
        var url_data = window.location.hash.slice(1).split('-');
        var fen = url_data[0];
        thinking_depth = parseInt(url_data[1]);
        player_color = (url_data[2] === 'w') ? 'white' : 'black';
        immune_pawns = url_data[3] === 't';
        dead_squares = url_data[4] === 't';
        new_game();
        board.position(fen, false);
        position_to_board(board.position());
        board.orientation(player_color);
    } else {
        new_game();
    }
    $('#difficulty').val(thinking_depth);
    $('#immune_pawns').val(immune_pawns);
    $('#dead_squares').val(dead_squares);

    $('#play').click(function () {
        player_color = $('input[name=play-as]:checked').val();
        immune_pawns = $('#immune_pawns').is(':checked');
        dead_squares = $('#dead_squares').is(':checked');
        new_game();
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
        clear_winner();
    });
    $('#hint').click(function () {
        var depth = thinking_depth + 1;
        if (depth < 3) {
            depth += 1;
        }
        calc_move(player_color,
            depth,
            function(move) {
                $('.square-' + move[0] + ', .square-' + move[1]).addClass('hint');
            }
        );
    });
    $('#difficulty').change(function () {thinking_depth = parseInt(this.value)});
};


$(document).ready(function() {
    var isCordovaApp = !!window.cordova;

    if (!isCordovaApp) {
        main();
    }
});


// will only be called on cordova
document.addEventListener("deviceready", function () {
    // adjust tracking
    var adjustConfig = new AdjustConfig(
        "u8x75dwq8q2o",
        AdjustConfig.EnvironmentProduction);
    Adjust.create(adjustConfig);

    main();
}, false);


// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
