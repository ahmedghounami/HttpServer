#!/bin/bash

echo "Starting search..."

ldapsearch -x -H ldap://ldap.1337.ma -b "dc=1337,dc=ma" "(objectClass=person)" uid | \
grep "^uid:" | awk '{print $2}' | while read uid; do
    echo "Checking $uid..."
    
    result=$(ldapsearch -x -H ldap://ldap.1337.ma -b "dc=1337,dc=ma" "(uid=$uid)")

    if echo "$result" | grep -iq "come to bocal"; then
        cn=$(echo "$result" | grep "^cn:" | head -n 1 | cut -d ' ' -f2-)
        echo "📌 $cn ($uid) come to bocal"
    fi
done
