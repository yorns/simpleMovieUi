#!/bin/bash
# should be installed into /usr/bin/
LOG=/tmp/db_mnt.log
#name="`basename "$DEVNAME"`"
db_root_dir=$1

# parameters are $1 mount directory
echo "find all databases on $db_root_dir" >> $LOG
if [ -d "$db_root_dir" ]; then
  echo "analyzing $db_root_dir" >> $LOG

  while IFS= read -r -d $'\0' file; do
    DB_FILES+=("$file")
  done < <(find $db_root_dir -iname "database.json" -print0)

  # run through list and send information to ui
  for i in "${DB_FILES[@]}"
  do
  #  i=${i#"./"}
    filename=$(basename "$i")
    pathname=$(dirname "$i")
    pname=`echo "$pathname" | sed 's@'"$db_root_dir/"'@@g'`
    echo /usr/bin/sender "echo $pathname" ui_db >> $LOG
    /usr/bin/sender "echo $pathname" ui_db
  done
fi
