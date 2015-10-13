#!/bin/bash

if [ $# -ne 1 ]; 
    then echo Usage: $0 file.svgz
    exit 1
fi

if [ ! -f $1 ]; then
    echo "you must specify a valid svg"
    exit 1
fi


file=`echo $1 | cut -d'.' --complement -f2-`
mv $1 $file.svg.gz
gunzip $file.svg.gz

echo Processing $file

stylesheet='
      .ColorScheme-Text {
        color:#31363b;
      }
      .ColorScheme-Background {
        color:#eff0f1;
      }
      .ColorScheme-Highlight {
        color:#3daee9;
      }
      .ColorScheme-ViewText {
        color:#31363b;
      }
      .ColorScheme-ViewBackground {
        color:#fcfcfc;
      }
      .ColorScheme-ViewHover {
        color:#93cee9;
      }
      .ColorScheme-ViewFocus{
        color:#3daee9;
      }
      .ColorScheme-ButtonText {
        color:#31363b;
      }
      .ColorScheme-ButtonBackground {
        color:#eff0f1;
      }
      .ColorScheme-ButtonHover {
        color:#93cee9;
      }
      .ColorScheme-ButtonFocus{
        color:#3daee9;
      }
      '
colors=(\#31363b \#eff0f1 \#3daee9 \#fcfcfc \#93cee9)
colorNames=(ColorScheme-Text ColorScheme-Background ColorScheme-Highlight ColorScheme-ViewBackground ColorScheme-ViewHover)

xml ed --subnode "/svg:svg/svg:defs" -t elem -n "style" -v "$stylesheet"\
       --subnode "/svg:svg/svg:defs/style" -t attr -n "type" -v "text/css"\
       --subnode "/svg:svg/svg:defs/style" -t attr -n "id" -v "current-color-scheme" $file.svg > temp.svg

for i in {0..4}
do
  xml ed --subnode "//*/*[contains(@style, '${colors[i]}')]" -t attr -n "class" -v "${colorNames[i]}" temp.svg > temp2.svg

  mv temp2.svg temp.svg

  sed -i 's/\(style=".*\)fill:'${colors[i]}'/\1fill:currentColor/g' temp.svg
done

mv temp.svg $file.svg
gzip $file.svg
mv $file.svg.gz $file.svgz
