#!/bin/bash

VIDEOFILES=()
DATABASE=database1.json
echo "[" > $DATABASE
while IFS= read -r -d $'\0' file; do
  VIDEOFILES+=("$file")
done < <(find . \( -iname "*.mpg" -or -iname "*.mpeg" -or -iname "*.avi" -or -iname "*.m4v" \) -and -print0 )

SKELETON=/usr/share/simpleVideoUi/skeleton.json
for i in "${VIDEOFILES[@]}" 
do
echo $i
i=${i#"./"}
filename=$(basename "$i")
filename="${filename%.*}"
sed 's@%fullname%@'\""$i"\"'@; s@%name%@'\""$filename"\"'@' $SKELETON >> $DATABASE
echo "," >> $DATABASE
done
echo "]" >> $DATABASE
