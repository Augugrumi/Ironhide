#!/bin/sh

set -e

if [ -n "$KUBERNETES_PORT" ] && [ -n "$DNS_IP" ]
then
    tmpfile=$(mktemp)
    (echo "nameserver $DNS_IP" && tac /etc/resolv.conf) > $tmpfile
    cat $tmpfile > /etc/resolv.conf
    rm $tmpfile
fi

ironhide $@
