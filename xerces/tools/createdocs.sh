#!/bin/sh
java -Djava.awt.headless=true -classpath "jars/stylebook-1.0-b2.jar:jars/xalan.jar:jars/xerces.jar" org.apache.stylebook.StyleBook "targetDirectory=../doc/html" ../doc/xerces-c_book.xml ../doc/style
