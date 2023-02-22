#include <stdio.h>

double integrate(double (*f)(double), double x, double y) {
    double sum=0.0;
    for(double i = x;i<y;i+=0.0001){
        sum+=f(i)*0.0001;
    }
    return sum;
}

double square(double x) {
    return x*x;
}

int main() {
    printf("integrate(square, 0.0, 2.0)=%f\n", integrate(square, 0.0, 2.0));
}

/*
>gcc 習題1.c

sp111b>a.exe       
integrate(square, 0.0, 2.0)=2.666867
*/


//step=0.0001

//0.0~2.0