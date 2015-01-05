#!/usr/bin/env sh

echo "Compiling with xctool"

xctool -project _Build_/Xcode6/Einstein.xcodeproj -scheme Einstein build

echo "Invoking otest-query-osx"
echo "Command is :"

echo OBJC_DISABLE_GC=YES \
__CFPREFERENCES_AVOID_DAEMON=YES \
NSUnbufferedIO=YES \
DYLD_FRAMEWORK_PATH=/Users/travis/Library/Developer/Xcode/DerivedData/Einstein-*/Build/Products/Debug \
DYLD_FALLBACK_FRAMEWORK_PATH=/Applications/Xcode.app/Contents/Developer/Library/Frameworks:/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/Library/Frameworks \
DYLD_LIBRARY_PATH=/Users/travis/Library/Developer/Xcode/DerivedData/Einstein-*/Build/Products/Debug \
/usr/local/Cellar/xctool/0.2.1/libexec/libexec/otest-query-osx \
/Users/travis/Library/Developer/Xcode/DerivedData/Einstein-*/Build/Products/Debug/EinsteinTests.xctest

OBJC_DISABLE_GC=YES \
__CFPREFERENCES_AVOID_DAEMON=YES \
NSUnbufferedIO=YES \
DYLD_FRAMEWORK_PATH=/Users/travis/Library/Developer/Xcode/DerivedData/Einstein-*/Build/Products/Debug \
DYLD_FALLBACK_FRAMEWORK_PATH=/Applications/Xcode.app/Contents/Developer/Library/Frameworks:/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/Library/Frameworks \
DYLD_LIBRARY_PATH=/Users/travis/Library/Developer/Xcode/DerivedData/Einstein-*/Build/Products/Debug \
/usr/local/Cellar/xctool/0.2.1/libexec/libexec/otest-query-osx \
/Users/travis/Library/Developer/Xcode/DerivedData/Einstein-*/Build/Products/Debug/EinsteinTests.xctest

echo "Invoked otest-query-osx"
