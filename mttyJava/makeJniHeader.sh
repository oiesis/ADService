#!/bin/bash

JNI_HEADER_DIR=$(pwd)/../jni/src

cd src/main/java 
javac com/mtty/cypher/AliBinaryEscaper.java
javah -o $JNI_HEADER_DIR/AliBinaryEscaper.h com.mtty.cypher.AliBinaryEscaper
