#!/bin/bash
assert() {
	expected="$1"
	input="$2"

	./9cc "$input" >tmp.s
	cc -o tmp tmp.s
	./tmp
	actual="$?"

	if [ "$actual" = "$expected" ]; then
		echo "$input => $actual"
	else
		echo "$input => $expected expected, but got $actual"
		exit 1
	fi
}

assert 0 "0;"
assert 70 "  12 + 	3 + 55	 ;"
assert 20 "  55 - 	31 -4	; "
assert 4 "(4 + 5) / 2;"
assert 10 "-10+20;"
assert 0 "-3*+5+15;"
assert 8 "-2*-(1+1)*((-1*-2));"
assert 10 '- - +10;'

assert 0 '0==1;'
assert 1 '42==42;'
assert 1 '0!=1;'
assert 0 '42!=42;'

assert 1 '0<1;'
assert 0 '1<1;'
assert 0 '2<1;'
assert 1 '0<=1;'
assert 1 '1<=1;'
assert 0 '2<=1;'

assert 1 '1>0;'
assert 0 '1>1;'
assert 0 '1>2;'
assert 1 '1>=0;'
assert 1 '1>=1;'
assert 0 '1>=2;'

assert 14 'a = 3;
b = 5 * 6 - 8;
b = a + b / 2;
1;
return b;'

# if
assert 2 '
a = 5;
if (a = 3) return 2;
return a;
'

# if
assert 5 '
a = 5;
if (a == 3) return 2;
return a;
'

# if-else
assert 2 '
a = 3;
if (a == 3) return 2;
else return 5;
'
assert 5 '
a = 1;
if (a == 3) return 2;
else return 5;
'

# while
assert 45 '
s = 0;
a = 10;
while (a = a - 1) s = s + a;
return s - a;
'

# for
assert 45 '
a=10;
for (s=0;a=a-1;)s=s+a;
return s;
'

# for
assert 0 '
a=10;
for (s=0;a=a-1;s=s+a)
return s;
'

# compound statement
assert 25 '
sum = 0;
for (i = 0; i < 5; i = i + 1) {
  sum = sum + i;
  sum = sum + (i + 1);
}
return sum;
'

# function call
# 別途、test() を作る必要はある
assert 11 '
return test(1,5,8);
'

echo OK
