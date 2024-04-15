#!/bin/bash
#
# make a template file

[ -z "$FILEDIR" ] && FILEDIR=. 


usage() {
  echo "$0 [LANGUAGE] [FILENAME]"
  echo "Where LANGUAGE is 'c' or 'd'"
  exit 1
}
template() {
  if [[ "$1" = "c" ]]; then
    if [[ ! -z "$2" ]]; then
      printf "#ifndef __$2_H\n#define __$2_H\n#endif /* __$2_H */\n" > "$FILEDIR/$2.h"
      printf "#include <$2.h>\n\n" > "$FILEDIR/$2.c"
    fi 
  elif [[ "$1" = "d" ]]; then
    echo "IS D"
    if [[ ! -z "$2" ]]; then
      printf "module $2;\n\n" > "$FILEDIR/$2.d"
    fi 
  fi 
  }

  if [[ -z "$1" ]]; then
    usage
  fi 
  if [[ -z "$2" ]]; then
    usage
  fi
  template "$@"
