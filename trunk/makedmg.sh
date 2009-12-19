#!/bin/bash

DMG_DIR=.build-dmg
VOL_NAME=Aorta
DMG_NAME=Aorta-`cat VERSION`.dmg

mkdir "$DMG_DIR"
cp -r build/Release/Aorta.app "$DMG_DIR"
cp COPYING "$DMG_DIR/COPYING.txt"
cp README "$DMG_DIR/README.txt"
rm -f "$DMG_NAME"
hdiutil create -format UDBZ -srcfolder .build-dmg -volname "$VOL_NAME" "$DMG_NAME"
rm -rf "$DMG_DIR"