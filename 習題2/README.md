### 更動:


```c
void DOWHILE(){
  int doBegin = nextLabel();//產生一個標記
  int doEnd = nextLabel();
  emit("(L%d)\n", doBegin);
  skip("do");
  BLOCK();
  skip("while");
  skip("(");
  int e = E();
  emit("goto L%d if T%d\n", doBegin,e);
  skip(")");
  emit("(L%d)\n", doEnd);
}

// STMT = WHILE | BLOCK | ASSIGN
void STMT() {
  if (isNext("do"))
    return DOWHILE();
  else if (isNext("while")) 
    //...
}
```

### 執行結果

1. dowhile.c

```
E:\learningE\課程\系統程式_code\sp111b\習題2\03d-compiler4>compiler.exe test/dowhile.c
i = 1;
do {
    i = i + 1;
} while (i<10);
========== lex ==============
token=i
token==
token=1
token=;
token=do
token={
token=i
token==
token=i
token=+
token=1
token=;
token=}
token=while
token=(
token=i
token=<
token=10
token=)
token=;
========== dump ==============
0:i
1:=
2:1
3:;
4:do
5:{
6:i
7:=
8:i
9:+
10:1
11:;
12:}
13:while
14:(
15:i
16:<
17:10
18:)
19:;
============ parse =============
t0 = 1
i = t0
(L0)
t1 = i
t2 = 1
t3 = t1 + t2
i = t3
t4 = i
t5 = 10
t6 = t4 < t5
goto L0 if T6
(L1)
```

2. dowhile2.c

```
E:\learningE\課程\系統程式_code\sp111b\習題2\03d-compiler4>compiler.exe test/dowhile2.c
do {
    i = 4;
    i = i + 1;
} while (i<10);
========== lex ==============
token=do
token={
token=i
token==
token=4
token=;
token=i
token==
token=i
token=+
token=1
token=;
token=}
token=while
token=(
token=i
token=<
token=10
token=)
token=;
========== dump ==============
0:do
1:{
2:i
3:=
4:4
5:;
6:i
7:=
8:i
9:+
10:1
11:;
12:}
13:while
14:(
15:i
16:<
17:10
18:)
19:;
============ parse =============
(L0)
t0 = 4
i = t0
t1 = i
t2 = 1
t3 = t1 + t2
i = t3
t4 = i
t5 = 10
t6 = t4 < t5
goto L0 if T6
(L1)
```