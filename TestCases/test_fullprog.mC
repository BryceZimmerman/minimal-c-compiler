/* should pass syntax check */

int i;
char c;
void v;

int i[17];
char c[17];
void v[17];

void foo (int i[], char c, void v) { 
  int i;
  char c;
  void v;
  
  {
    i = 17;
  }

  c[17] = 1;
  
  if (i >= 1)  
     i = 17; 


  if (i <= 17)
    i = 2;
  else
    if (i == 0)
      i = c + 17;


  while (v > 17 ) 
    v = 13;

  return 0;
}


int main() {
  foo(i, x + y, foo());
  return;
}
