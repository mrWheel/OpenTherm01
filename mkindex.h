#! /usr/bin/env bash
#
# This script takes index.html index.css and index.js from data to create single array to avoid FS interaction
#
# When called with parameter, this will be the prefix used for the 3 files
#
# All output is written into index.h

if [ $# -gt 0 ]
then
  prefix=$1
else
  prefix=""
fi

exec 1> index.h

# write header

cat <<EOH
static const char indexPage[] =
R"(
EOH


# copy index.html line by line to ouput

while read line
do

  # before </head> in index.html add css and js files

  if [[ "${line}" == *${prefix}index.js* ]]
  then
    echo '<script>'
    cat data/${prefix}index.js
    echo '</script>'
  else if [[ "${line}" == *${prefix}index.css* ]]
    then
      echo '<style>'
      cat data/${prefix}index.css
      echo '</style>'
    else
      echo "${line}"
    fi
  fi

done < data/${prefix}index.html

# write trailing bits

echo ')";'

