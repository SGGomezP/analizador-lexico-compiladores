// Programa de prueba con todas las categorias de tokens
int suma(int a, int b) {
    return a + b;
}

void main() {
    int notas[5];
    int i;
    float total = 0.0;
    float promedio;
    char letra;
    boolean aprobado;

    for (i = 0; i < 5; i = i + 1) {
        scanf(notas[i]);
        total = total + notas[i];
    }

    promedio = total / 5;   // promedio simple
    aprobado = promedio >= 10.5 && promedio != 0 || !aprobado;

    while (i > 0) {
        i = i - 1;
    }

    if (i % 2 == 0) {
        println("par");
    } else {
        println("impar");
    }

    println("Promedio:", promedio);
    return;
}
