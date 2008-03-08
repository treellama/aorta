#!/bin/bash

BUILD_ZIP_DIR=.build-zip
ZIP_DIR=Aorta
ZIP_NAME=Aorta-`cat VERSION`.zip

mkdir -p "$BUILD_ZIP_DIR/$ZIP_DIR"
cp Aorta.exe "$BUILD_ZIP_DIR/$ZIP_DIR/"
cp COPYING "$BUILD_ZIP_DIR/$ZIP_DIR/COPYING.txt"
cp README "$BUILD_ZIP_DIR/$ZIP_DIR/README.txt"
cd "$BUILD_ZIP_DIR"
zip -r "../$ZIP_NAME" "$ZIP_DIR"
cd ..
rm -rf "$BUILD_ZIP_DIR"
