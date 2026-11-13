// if / while / for / scanf / println se reconocen y se omiten con un aviso
void main() {
    int i;
    int n = 3;
    scanf(n);
    for (i = 0; i < n; i = i + 1) {
        println(i);
    }
    while (n > 0) { n = n - 1; }
    if (n == 0) { println("cero"); } else if (n > 0) { n = 1; } else { n = 2; }
    n = 7;
}
