int global, a[10];
int b;


int print();

int scan();
void exit(int i);

int scan(){


}

int funcy(int arr[10], int len, int threshold) {
  int i, sum;
  i = 1;
  sum = 0;
  while (i < len) {
    if (arr[i] <= threshold) sum = sum + 1;
    i = i + 1;
    print(i);
  }
  return sum;
}



void exit(int i){

}

int print(int gerd){

}

int main() {
  global = print(1);
  return funcy(a,5+5,5);
  print(scan());

  exit(0);
}
