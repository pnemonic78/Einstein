#!/usr/bin/env sh

echo "Overriding access restrictions on kTCCServiceAddressBook"
echo "http://stackoverflow.com/questions/17693408/enable-access-for-assistive-devices-programmatically-on-10-9"

if [ -e "${HOME}/Library/Application Support/com.apple.TCC/TCC.db" ]; then
    sqlite3 "${HOME}/Library/Application Support/com.apple.TCC/TCC.db" "INSERT INTO access_overrides VALUES ('kTCCServiceAddressBook');"
else
    echo "Could not find TCC.db !"
fi

echo "Compiling with xctool"

xctool -project _Build_/Xcode6/Einstein.xcodeproj -scheme CLITest build

echo "Running CLITest"

/Users/travis/Library/Developer/Xcode/DerivedData/Einstein-*/Build/Products/Debug/CLITest host-info

echo "Ran CLITest"
