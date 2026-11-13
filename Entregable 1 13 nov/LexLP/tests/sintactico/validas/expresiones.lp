// Precedencia: * / % antes que + -, luego COMP, luego &&, luego ||
void main() {
    int a = 1;
    int b = 2;
    boolean r;
    a = a + b * 3 - 4 / 2 % 5;
    a = (a + b) * (a - b);
    r = a > b && b <= 10 || !r;
    r = a == b;
    r = a != b && !(a < b);
    a = - -a;
    a = 10 % 3 + 2.5;
}
