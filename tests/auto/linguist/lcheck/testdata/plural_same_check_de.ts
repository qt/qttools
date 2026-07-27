<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="de_DE">
<context>
  <name>Main</name>
  <!-- A single plural message whose numerus forms all fail the same
       (punctuation) check. Regression test for QTBUG-148562: previously
       only one form was reported because Validator::validate() keyed its
       results by error type. -->
  <message numerus="yes">
    <location filename="main.cpp" line="10"/>
    <source>%n file.</source>
    <translation type="unfinished">
      <numerusform>%n Datei</numerusform>
      <numerusform>%n Dateien</numerusform>
    </translation>
  </message>
</context>
</TS>
