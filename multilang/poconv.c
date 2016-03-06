#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void sub(char *buf, char *from, char *to) {

  char *p,*posfix;
  int fs;

  fs = strlen(from);

  printf("sub [%s] for [%s], fs=%d\n",from,to,fs);

  for(;;) {

    p = strstr(buf,from);
    if (p==NULL) return;

    *p = 'X';
    posfix = strdup(p+fs);
    strcpy(p,to);
    strcat(p,posfix);
    free(posfix);

  }
}

int main(int argc, char **argv) {

  FILE *f;
  char *buf,*p;
  int c;

  if (argc!=2) {
    fprintf(stderr,"usage: poconv po-file\n\n");
    return 1;
  }

  f = fopen(argv[1],"r");
  
  buf = (char *) malloc(500000);
  p = buf;

  for(;;) {
    c = fgetc(f);
    if (c==EOF) break;
    *p = (char) c;
    ++p;
  }
  *p = 0;
  fclose(f);

  sub(buf,"\302\200","#L#");
  sub(buf,"\302\201","#M#");
  sub(buf,"\302\202","#S#");
  sub(buf,"\302\203","#B#");
  sub(buf,"\302\204","#K#");
  sub(buf,"\200","#L#");
  sub(buf,"\201","#M#");
  sub(buf,"\202","#S#");
  sub(buf,"\203","#B#");
  sub(buf,"\204","#K#");

  sub(buf,"bergo@seul.org","fbergo@gmail.com");

  f = fopen(argv[1],"w");
  for(p=buf;*p!=0;p++)
    fputc(*p,f);
  fclose(f);
  return 0;
}
