#!/bin/bash
# parameters are $1 mount directory
MNT_PNT=$1
while IFS= read -r -d $'\0' file; do
  DB_FILES+=("$file")
done < <(find $MNT_PNT -iname "database.json" -print0)

# run through list and send information to ui
for i in "${DB_FILES[@]}"
do
#  i=${i#"./"}
  filename=$(basename "$i")
  pathname=$(dirname "$i")
  pname=`echo "$pathname" | sed 's@'"$MNT_PNT/"'@@g'`
  echo /usr/bin/sender "echo $MNT_PNT $pname $filename" ui_db
   /usr/bin/sender "echo $i" ui_db
done
