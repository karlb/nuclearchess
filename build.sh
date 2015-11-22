#!/bin/sh
emcc -O2 src/atomschach.c -o www/js/atomschach.js -s EXPORTED_FUNCTIONS="['_newGame', '_sub_main', '_von_x', '_von_y', '_nach_x', '_nach_y', '_get_brett', '_legal', '_set_zug_temp', '_hat_koenig', '_get_zug_temp', '_get_punkte_int_temp', '_anwenden', '_computer_zug']" \
-s EXPORT_ALL=1 \
--memory-init-file 0
#-s EXPORTED_GLOBALS=[brett]
