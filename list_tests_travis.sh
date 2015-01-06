#!/usr/bin/env sh

echo "Compiling with xctool"

xctool -project _Build_/Xcode6/Einstein.xcodeproj -scheme CLITest build

echo "Running CLITest"
echo "Command is :"

echo /Users/travis/Library/Developer/Xcode/DerivedData/Einstein-*/Build/Products/Debug/CLITest

/Users/travis/Library/Developer/Xcode/DerivedData/Einstein-*/Build/Products/Debug/CLITest

echo "Ran CLITest"
