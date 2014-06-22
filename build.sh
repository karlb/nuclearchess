#!/bin/sh
emcc -O2 src/atomschach.c -o js/atomschach.js -s EXPORTED_FUNCTIONS="['_newGame', '_sub_main', 'von_x', 'von_y', 'nach_x', 'nach_y']" \
-s EXPORT_ALL=1 \
-s NAMED_GLOBALS=1
