int global, a[10];
int b;
void print (int gerd, int urban);

int scan ();
int func(int arr[10], int thereshold, int en[2], int oen);
void exit (int i);

int func (int arr[10], int len, int threshold, int doener) {
  int i, sum;
  i = 0;
  sum = 0;
  while (i < len) {
    if (arr[i] <= threshold) sum = sum + 1;
    i = i + 1;
    print(i); 
  }
  return sum;
}

int main () {
  global = 1;
  return func(a,5+5,5);

  print(scan());

  exit(0);
}
