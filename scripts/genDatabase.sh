#!/bin/bash

declare -a VIDEOFILES=()
declare -a NEWVIDEOFILES=()

DATABASE=database1.json
ORIG_DATABASE=database.json

while IFS= read -r -d $'\0' file; do
  VIDEOFILES+=("$file")
done < <(find . \( -iname "*.mpg" -or -iname "*.mpeg" -or -iname "*.avi" -or -iname "*.m4v" -or -iname "*.mp4" \) -and -print0 )

SKELETON=/usr/share/simpleMovieUi/skeleton.json

if [ -e $ORIG_DATABASE ]; then
  head -n -1 $ORIG_DATABASE > $DATABASE
  echo "," >> $DATABASE
else
  echo "[" > $DATABASE
fi

arraylength=${#VIDEOFILES[@]}

for (( i=0; i<${arraylength}; i++ )); do
  tmp=${VIDEOFILES[$i]#"./"}
  if grep "$tmp" $ORIG_DATABASE > /dev/null; then
    echo "ignore old file $tmp"
  else
    NEWVIDEOFILES+=("$tmp")
  fi
done

arraylength=${#NEWVIDEOFILES[@]}

for (( i=0; i<${arraylength}; i++ )); do
  echo ${NEWVIDEOFILES[$i]} $i "/" $arraylength
  tmp=${NEWVIDEOFILES[$i]}
    filename=$(basename "$tmp")
    filename="${filename%.*}"
    sed 's@%fullname%@'\""$tmp"\"'@; s@%name%@'\""$filename"\"'@' $SKELETON >> $DATABASE
    if [ ${i} -lt $((${arraylength} - 1)) ]; then
      echo "," >> $DATABASE
    fi
done

echo "]" >> $DATABASE
