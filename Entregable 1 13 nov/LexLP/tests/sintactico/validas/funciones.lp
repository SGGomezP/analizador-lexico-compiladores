int suma(int a, int b) {
    return a + b;
}

float promedio(float notas[], int n) {
    float total = 0.0;
    return total / n;
}

void saludar() {
    return;
}

void main() {
    int r;
    r = suma(2, 3);
    r = suma(suma(1, 2), 4) * 2;
    saludar();
    return;
}
