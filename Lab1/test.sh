#!/bin/sh

#Junwhan Yoo
#404037837
#Lab1a Test Cases

if [ -e "simpsh" ]
then
    rm -rf simpsh
fi

make || exit

chmod 744 simpsh

cat > input.txt <<EOF
Test input!
EOF

#copying
cat input.txt > i1.txt
cat input.txt > i3.txt
cat input.txt > i4.txt
cat input.txt > i5.txt
cat input.txt > i6.txt
cat input.txt > i7.txt
#1b tests
cat input.txt > i8.txt




#test case 1
./simpsh --rdonly i1.txt > /dev/null 2>&1
diff input.txt i1.txt
if [ $? == 0 ]
then
    echo "test passed"
else
    echo "test case 1 failed"
fi
./simpsh --wronly i1.txt > /dev/null 2>&1
diff input.txt i1.txt
if [ $? == 0 ]
then
    echo "test passed"
else
    echo "test case 1 failed"
fi

#test case 2
./simpsh --rdonly i1_nonexist.txt > /dev/null 2>&1
if [ -e "i1_nonexist.txt" ]
then
    echo "test case 2 failed"
else
    echo "test passed"
fi
#test case 3
touch o3.txt
./simpsh --verbose --verbose > o3.txt
cat>t3.txt<<EOF
--verbose
EOF
diff t3.txt o3.txt
if [ $? == 0 ]
then
    echo "test passed"
else
    echo "test case 3 failed"
fi

#test case 4
touch o4.txt
touch e4.txt
./simpsh --rdonly i4.txt --wronly o4.txt --wronly e4.txt --command 0 1 2 cat > /dev/null 2>&1

sleep 1
diff i4.txt o4.txt
if [ $? == 0 ]
then
    echo "test passed"    
else
    echo "test case 4 failed"
fi

#test case 5
touch o5.txt
touch e5.txt
./simpsh --rdonly i5.txt --wronly o5.txt --wronly e5.txt --command 0 1 2 ls > /dev/null 2>&1

sleep 1
ls > t5.txt
if [ $? == 0 ]
then
    echo "test passed"    
else
    echo "test case 5 failed"
fi

#test case 6
touch o6.txt
touch e6.txt
touch t6.txt

./simpsh --rdonly i6.txt --wronly o6.txt --verbose --wronly e6.txt --command 0 1 2 echo hello > t6.txt 2> /dev/null

sleep 1
cat> std6.txt<<EOF
--wronly e6.txt
--command 0 1 2 echo hello
EOF
diff std6.txt t6.txt
if [ $? == 0 ]
then
    echo "test passed"    
else
    echo "test case 6 failed"
fi

#test case 7 : few argument
touch o7.txt
touch e7.txt
touch t7.txt

./simpsh --rdonly i7.txt --wronly o7.txt --wronly e7.txt --command 0 1 2 > /dev/null 2> t7.txt

echo "should see error message below."
cat t7.txt

##1b test cases
#test case 8
touch o8.txt
touch e8.txt

./simpsh --rdonly i8.txt --pipe --wronly o8.txt --wronly e8.txt --command 0 2 4 cat --command 1 3 4 cat --close 1 --close 2 --wait > /dev/null 2>&1
diff o8.txt i8.txt
if [ $? == 0 ]
then
    echo "test passed"
else
    echo "test case 8 failed"
fi




#cleaning
rm -rf input.txt
rm -rf i1.txt i3.txt i4.txt i5.txt i6.txt i7.txt i8.txt
rm -rf o3.txt o4.txt o5.txt o6.txt o7.txt o8.txt
rm -rf e4.txt e5.txt e6.txt e7.txt e8.txt
rm -rf t3.txt t5.txt t6.txt t7.txt std6.txt

