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
        stop-color:#31363b;
      }
      .ColorScheme-Background {
        color:#eff0f1;
        stop-color:#eff0f1;
      }
      .ColorScheme-Highlight {
        color:#3daee9;
        stop-color:#3daee9;
      }
      .ColorScheme-ViewText {
        color:#31363b;
        stop-color:#31363b;
      }
      .ColorScheme-ViewBackground {
        color:#fcfcfc;
        stop-color:#fcfcfc;
      }
      .ColorScheme-ViewHover {
        color:#93cee9;
        stop-color:#93cee9;
      }
      .ColorScheme-ViewFocus{
        color:#3daee9;
        stop-color:#3daee9;
      }
      .ColorScheme-ButtonText {
        color:#31363b;
        stop-color:#31363b;
      }
      .ColorScheme-ButtonBackground {
        color:#eff0f1;
        stop-color:#eff0f1;
      }
      .ColorScheme-ButtonHover {
        color:#93cee9;
        stop-color:#93cee9;
      }
      .ColorScheme-ButtonFocus{
        color:#3daee9;
        stop-color:#3daee9;
      }
      '
colors=(\#caced0 \#ffffff \#93cee9 \#fcfcfc \#a3cee9)
colorNames=(ColorScheme-Text ColorScheme-Background ColorScheme-Highlight ColorScheme-ViewBackground ColorScheme-ViewHover)


reorderXslt='
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:svg="http://www.w3.org/2000/svg">
 <xsl:output omit-xml-declaration="yes" indent="yes"/>
 <xsl:strip-space elements="*"/>

  <xsl:template match="@* | node()">
    <xsl:copy>
      <xsl:apply-templates select="@* | node()"/>
    </xsl:copy>
  </xsl:template>

  <xsl:template match="svg:defs">
    <xsl:copy>
      <xsl:apply-templates select="@*" />
      <xsl:apply-templates select="*">
        <xsl:sort select="name()" data-type="text" order="descending"/>
      </xsl:apply-templates>
    </xsl:copy>
  </xsl:template>

</xsl:stylesheet>
'
echo $reorderXslt > transform.xsl

if grep -q '"current-color-scheme"' $file.svg; then
    echo replacing the stylesheet
    xml ed --update "/svg:svg/svg:defs/_:style" -v "$stylesheet" $file.svg > temp.svg
else
    echo adding the stylesheet
xml ed --subnode "/svg:svg/svg:defs" -t elem -n "style" -v "$stylesheet"\
       --subnode "/svg:svg/svg:defs/style" -t attr -n "type" -v "text/css"\
       --subnode "/svg:svg/svg:defs/style" -t attr -n "id" -v "current-color-scheme" $file.svg > temp.svg
fi

xml tr transform.xsl temp.svg > temp2.svg
mv temp2.svg temp.svg

for i in {0..4}
do
  xml ed --subnode "//*/*[contains(@style, '${colors[i]}') and not (@class)]" -t attr -n "class" -v "${colorNames[i]}" temp.svg > temp2.svg

  mv temp2.svg temp.svg

  sed -i 's/\(style=".*\)fill:'${colors[i]}'/\1fill:currentColor/g' temp.svg
  sed -i 's/\(style=".*\)stop-color:'${colors[i]}'/\1stop-color:currentColor/g' temp.svg
done

rm transform.xsl

mv temp.svg $file.svg
gzip $file.svg
mv $file.svg.gz $file.svgz
