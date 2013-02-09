#!/bin/sh

# creates massive numbers of dummy "test" accounts
#
# WARNING: please backup your users directory before
# running this script as it will assume that UIDs
# ara not used.  If you start bnetd with accounts
# with duplicate UIDs you are likely to lose your
# existing files. 


# number of accounts to connect with
numaccts=400

# "prefix" of account names
name="bob"

# account password
pass="bob"

# number of zero-padded columns in suffix
padding=6

# "users" directory
users=/usr/local/bnetd/var/users

# bnpass command
bnpass=/usr/local/bnetd/bin/bnpass


hash="`echo \"${pass}\" | \"${bnpass}\" | sed -e 's/^.*"\([0-9a-f]*\)"/\1/'`"


num=0
while [ "${num}" -lt "${numaccts}" ]; do
    num="`expr \"${num}\" '+' '1'`"
    form="`printf \"%0${padding}d\" \"${num}\"`"
    (
        echo '"BNET\\acct\\username"="'"${name}${form}"'"'
        echo '"BNET\\acct\\passhash1"="'"${hash}"'"'
        echo '"BNET\\acct\\userid"="'"${num}"'"'
    ) > "${users}/${form}"
done

exit 0
