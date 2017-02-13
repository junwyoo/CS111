#!/bin/sh

if [ "${PATH:0:16}" == "/usr/local/cs/bin" ]
then
    true
else
    PATH=/usr/local/cs/bin:$PATH
fi

touch testText.txt
touch targetText.txt
touch errText.txt

dd if=/dev/urandom of=testText.txt bs=2048 count=1000

cat testText.txt | tr "A-Za-z" "a-zA-Z" | sort > targetText.txt

times



rm testText.txt
rm targetText.txt
rm errText.txt
