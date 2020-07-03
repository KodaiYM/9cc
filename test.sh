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

assert 0 "
main(){
	return 0;
}"
assert 70 "main(){  12 + 	3 + 55	 ;}"
assert 20 "main(){  55 - 	31 -4	; }"
assert 4 "main(){(4 + 5) / 2;}"
assert 10 "main(){-10+20;}"
assert 0 "main(){-3*+5+15;}"
assert 8 "main(){-2*-(1+1)*((-1*-2));}"
assert 10 'main(){- - +10;}'

assert 0 'main(){0==1;}'
assert 1 'main(){42==42;}'
assert 1 'main(){0!=1;}'
assert 0 'main(){42!=42;}'

assert 1 'main(){0<1;}'
assert 0 'main(){1<1;}'
assert 0 'main(){2<1;}'
assert 1 'main(){0<=1;}'
assert 1 'main(){1<=1;}'
assert 0 'main(){2<=1;}'

assert 1 'main(){1>0;}'
assert 0 'main(){1>1;}'
assert 0 'main(){1>2;}'
assert 1 'main(){1>=0;}'
assert 1 'main(){1>=1;}'
assert 0 'main(){1>=2;}'

assert 14 'main(){a = 3;
b = 5 * 6 - 8;
b = a + b / 2;
1;
return b;}'

# if
assert 2 'main(){
a = 5;
if (a = 3) return 2;
return a;
}'

# if
assert 5 'main(){
a = 5;
if (a == 3) return 2;
return a;
}'

# if-else
assert 2 'main(){
a = 3;
if (a == 3) return 2;
else return 5;
}'
assert 5 'main(){
a = 1;
if (a == 3) return 2;
else return 5;
}'

# while
assert 45 'main(){
s = 0;
a = 10;
while (a = a - 1) s = s + a;
return s - a;
}'

# for
assert 45 'main(){
a=10;
for (s=0;a=a-1;)s=s+a;
return s;
}'

# for
assert 0 'main(){
a=10;
for (s=0;a=a-1;s=s+a)
return s;
}'

# compound statement
assert 25 'main(){
sum = 0;
for (i = 0; i < 5; i = i + 1) {
  sum = sum + i;
  sum = sum + (i + 1);
}
return sum;
}'

# function call
assert 3 '
main () {
	return add(1, 2);
}
add (first, second) {
	return first + second;
}
'

# function call
assert 41 'main(){
	return test(1, 5, 8);
}
test (first, second, third) {
	return first + second * third;
}'

# address, indirection
assert 1 '
main () {
	x;
	y = &x;
	return *y**y++1+*y*-*y;
}
'

echo OK
