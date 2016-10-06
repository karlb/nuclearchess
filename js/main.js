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
            $(window).height() - $('#top-bar').outerHeight() - (phonegap ? 0 : $('#bottom-bar').outerHeight()),
            500);
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


function main() {
    game.init();
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
        "5oj72xlvpx1c",
        AdjustConfig.EnvironmentProduction);
    Adjust.create(adjustConfig);

    main();
}, false);


// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
