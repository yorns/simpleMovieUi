#!/bin/bash

VIDEOFILES=()
DATABASE=database.json
echo "[" > $DATABASE
while IFS= read -r -d $'\0' file; do
  #echo "$file"
  VIDEOFILES+=("$file")
done < <(find . -iname "*.mpg" -o -name "*.mpeg" -o -name "*.avi" -o -name "*.m4v" -print0)

SKELETON=/usr/share/simpleVideoUi/skeleton.json
for i in "${VIDEOFILES[@]}" 
do
#echo "$i"
i=${i#"./"}
filename=$(basename "$i")
filename="${filename%.*}"
sed 's@%fullname%@'\""$i"\"'@; s@%name%@'\""$filename"\"'@' $SKELETON >> $DATABASE
done

echo "]" >> $DATABASE
