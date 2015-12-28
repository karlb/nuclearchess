_brett = _get_brett();
_zug_temp = _get_zug_temp();

var game = (function () {
	var my = {};
	var board;
    var undo_stack = [];
    var nuclear_strike = false;  // either 'false' or the field where the strike hit
    var waiting_for_player = true;
    var game_over;
    var move_end_callbacks = [];
    var cfg;

    my.new_game = function (game_cfg, fen) {
		cfg = game_cfg;
        _newGame(cfg.keep_pawns, cfg.dead_squares);
		if (fen) {
			board.position(fen, false);
			position_to_board(board.position());
		}
        update_html_board(board, false);
		if (game_cfg.shuffle) {
            shuffle(board, 8);
            shuffle(board, 1);
		}
        board.orientation(cfg.player_color);
        game_over = false;
        clear_winner();
        $('.hint').removeClass('hint');
		state_to_url();
    };

    function calc_move(color, depth, cb) {
        if (game_over) {
            return;
        }
        $('#thinking').show();
        setTimeout(function() {
            _computer_zug(color === 'white' ? 1 : -1,
                          depth, _brett, _zug_temp, 1);
            $('#thinking').hide();
            var move = get_current_move();
            cb(move);
        }, 10);
    }

    function computer_turn() {
        calc_move(cfg.player_color === 'white' ? 'black' : 'white',
            cfg.depth,
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
						if (
							board.position()[field] !== undefined
							&& board.position()[field] !== 'nuke'
							&& (
								board.position()[field][1] !== 'P'
								|| !cfg.keep_pawns
							)
						) {
							console.log(board.position()[field], cfg.keep_pawns);
							piece_classes.push('.square-' + field + ' img');
						}
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
        if (_hat_koenig(1, _brett) != 1 && _hat_koenig(-1, _brett) != 1) {
            game_over = true;
            set_winner('Draw game');
        }
        else if (_hat_koenig(1, _brett) != 1) {
            game_over = true;
            set_winner('Black wins!');
        }
        else if (_hat_koenig(-1, _brett) != 1) {
            game_over = true;
            set_winner('White wins!');
        }
    }

	function state_to_url() {
        history.replaceState({}, '',
            '#' + board.fen() +
            '-' + cfg.depth +
            '-' + cfg.player_color[0] +
            '-' + (cfg.keep_pawns ? 't' : 'f') +
            '-' + (cfg.dead_squares ? 't' : 'f')
        );
	}

    function on_move_end(from_field, to_field) {
        show_nuclear_strike();
        check_game_over();
        waiting_for_player = true;
		state_to_url();

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

	my.init = function init() {
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
					piece[0] !== cfg.player_color[0]  // it's not his piece
					|| game_over  // the game has already ended
					|| waiting_for_player === false  // animation in progress or computer's turn
				) {
					return false;
				}
			}
		});
		my.board = board;

		resize(board);

		$(window).resize(function(){
			resize(board);
		});

		$('#undo').click(function () {
			var position = undo_stack.pop();
			position_to_board(position);
			update_html_board(board);
			game_over = false;
			clear_winner();
		});

		$('#hint').click(function () {
			var depth = cfg.depth + 1;
			if (depth < 3) {
				depth += 1;
			}
			calc_move(cfg.player_color,
				depth,
				function(move) {
					$('.square-' + move[0] + ', .square-' + move[1]).addClass('hint');
				}
			);
		});

		$('#new-game').click(function () {
			page('start');
		});
	};

    // set position from url if a fen-string is in the URL fragment part
    /*
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
    */

    /*$('#play').click(function () {
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
    });*/
	return my;
}());
