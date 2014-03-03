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

$(document).ready(function(){
    log("starting...!\n");

    var weiss = 1, schwarz = -1;
    var tiefe = 1;
    var farbe = weiss;

    _newGame();
    log(print_brett());

    // zug legal / erlaubt
//    log( _legal(1, 1, 2, 1, _brett) );
//    log( _legal(1, 1, 1, 2, _brett) );

    //console.log(_hat_koenig(weiss, _brett));
    //console.log(_hat_koenig(schwarz, _brett));

    while ((_hat_koenig(weiss, _brett) == 1) && (_hat_koenig(schwarz, _brett) == 1))
    {

        _computer_zug(farbe, tiefe, _brett, _zug_temp, _punkte_int_temp, 1);
        farbe = -farbe; 
        
        log(print_zug(_zug_temp));

    _anwenden(_brett, _zug_temp);
    }
    log(print_brett());
});
