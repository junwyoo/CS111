#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <signal.h>

void printParentUsage(struct rusage ru)
{
	  struct rusage temp;
	  if(getrusage(RUSAGE_SELF, &temp)<0)
	    {
	      fprintf(stderr, "Failed getting usage\n");
	      exit(1);
	    }

	  double utime = (double)((ru.ru_utime.tv_sec-ru.ru_utime.tv_sec)+(temp.ru_utime.tv_usec-ru.ru_utime.tv_usec)/1000000.0);
	  double stime = (double)((temp.ru_stime.tv_sec-ru.ru_stime.tv_sec)+(temp.ru_stime.tv_usec-ru.ru_stime.tv_usec)/1000000.0);

	  printf("User time: %f System time: %f\n", utime,stime);
}
void printChildUsage()
{

  struct rusage temp;
  if(getrusage(RUSAGE_CHILDREN, &temp)<0)
    {
      fprintf(stderr, "Failed getting usage\n");
      exit(1);
    }

  double utime = (double)(temp.ru_utime.tv_sec+temp.ru_utime.tv_usec/1000000.0);
  double stime = (double)(temp.ru_stime.tv_sec+temp.ru_stime.tv_usec/1000000.0);
  printf("Usage for Children: \n");
  printf("User time: %f System time: %f\n", utime, stime);
}

int checkArgument(int argc, char *argv[]) //1 if there is argument
{
  char *ptr = NULL;
  if(optind <argc)
    ptr = argv[optind];
  if(optind == argc || (ptr!=NULL && *(ptr) == '-'))
    return 1;
  return 0;
}
int isValid(long int fd, int fd_number)
{
  if(fd<0 || (fd>=fd_number))
    return 0;
  return 1;
}
void handler(int n_signal)
{
  fprintf(stderr, "%d caught\n", n_signal);
  exit(n_signal);
}


int main(int argc, char *argv[])
{
  int i, c;
  int n_fd = 0;
  int fd_number = 0;
  int oflag = 0;
  int exit_status = 0;
  int max_status = 0;
  int vflag = 0;
  int n_signal;
  int pflag = 0;

  struct rusage ru;

  struct timeval ou_time;
  struct timeval os_time;
  struct timeval nu_time;
  struct timeval ns_time;
  ou_time.tv_sec = 0;
  ou_time.tv_usec= 0;
  os_time.tv_sec = 0;
  os_time.tv_usec= 0;
  
  for(i=0; i<argc ; i++) // Count for the number of fd
    {
      if(!strcmp(argv[i],"--rdonly\0") || !strcmp(argv[i],"--wronly\0") ||
	 !strcmp(argv[i],"--rdwr\0"))
	n_fd++;
      else if(!strcmp(argv[i],"--pipe"))
	n_fd += 2;
    }

  int *fd = (int*) malloc(sizeof(int)* n_fd);
  int *isPipe = (int*) malloc(sizeof(int)*n_fd);
  int *proc_array = (int*) malloc(0);
  int child_number = 0;
  int *start = (int*) malloc(sizeof(int)* argc);
  int *end = (int*) malloc(sizeof(int)* argc);
  int command = 0;
  
      
  while (1)
    {
      static struct option long_options[]=
	{
	  {"rdonly", required_argument, 0, 'r'},
	  {"wronly", required_argument, 0, 'w'},
	  {"command", required_argument, 0, 'c'},
	  {"verbose", no_argument, 0, 'v'},

	  {"append", no_argument, 0, 'a'},
	  {"cloexec", no_argument, 0, 'b'},
	  {"creat", no_argument, 0, 'd'},
	  {"directory", no_argument, 0, 'e'},
	  {"dsync", no_argument, 0, 'f'},
	  {"excl", no_argument, 0, 'g'},
	  {"nofollow", no_argument, 0, 'h'},
	  {"nonblock", no_argument, 0, 'i'},
	  {"rsync", no_argument, 0, 'j'},
	  {"sync", no_argument, 0, 'k'},
	  {"trunc", no_argument, 0, 'l'},

	  {"pipe", no_argument, 0, 'p'},

	  {"wait", no_argument, 0, 'm'},
	  {"close", required_argument, 0, 'n'},
	  {"abort", no_argument, 0, 'o'},
	  {"catch", required_argument, 0, 'q'},
	  {"ignore", required_argument, 0, 's'},
	  {"default", required_argument, 0, 't'},
	  {"pause", no_argument, 0, 'u'},

	  {"profile", no_argument,0,'y'},
	  
	  {0,0,0,0}
       	};
      c = getopt_long(argc, argv, "", long_options, 0);

      if(c==-1)
	break;

      switch(c)
	{
	case 'a':
	case 'b':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	  //syntax check
	  if(argv[optind] == '\0')
	    {
	      fprintf(stderr, "Error: Requires a file.\n");
	      exit_status = 1;
	      break;
	    }
	  else if(!(argv[optind][0]=='-' && argv[optind][1]=='-'))
	    {
	      fprintf(stderr, "Error: Flags Do not need an argument.\n");
	      exit_status = 1;
	      break;
	    }
	  
	  if(vflag)//verbose option
	    fprintf(stdout,"%s\n", argv[optind-1]);

	  //setting flags
	  if(c == 'a')
	    oflag |= O_APPEND;
	  else if(c == 'b')
	    oflag |= O_CLOEXEC;
	  else if(c=='d')
	    oflag |= O_CREAT;
	  else if(c=='e')
	    oflag |= O_DIRECTORY;
	  else if(c=='f')
	    oflag |= O_DSYNC;
	  else if(c=='g')
	    oflag |= O_EXCL;
	  else if(c=='h')
	    oflag |= O_NOFOLLOW;
	  else if(c=='i')
	    oflag |= O_NONBLOCK;
	  else if(c=='j')
	    oflag |= O_RSYNC;
	  else if(c=='k')
	    oflag |= O_SYNC;
	  else if(c=='l')
	    oflag |= O_TRUNC;
	  break;
	  
	case 'r':
	case 'w':
	case 'x':
	  //rdonly and wronly option
	  if(pflag)
	    if(getrusage(RUSAGE_SELF,&ru)<0)
	      {
		fprintf(stderr, "Failed to call get usage.\n");
		exit(1);
	      }
	  
	  if(vflag)
	      {
		if(c== 'r')
		  fprintf(stdout, "--rdonly %s\n", optarg);
		else if(c== 'w')
		  fprintf(stdout, "--wronly %s\n", optarg);
		else
		  fprintf(stdout, "--rdwr %s\n", optarg);
	      }
	  //syntax check
	  if(optarg[0] == '-' && optarg[1] == '-')
	    {
	      fprintf(stderr, "Error: Requires an argument.\n");
	      exit_status = 1;
	      break;
	    }
	  else if(argv[optind] != '\0' && argv[optind][0] != '-' &&
		  argv[optind][1] != '-')
	    {
	      fprintf(stderr, "Error: More than one argument.\n");
	      exit_status = 1;
	      break;
	    }
	  //setting flag
	  if(c == 'r')
	    oflag |= O_RDONLY;
	  else if (c == 'w')
	    oflag |= O_WRONLY;
	  else //c=='x'
	    oflag |= O_RDWR;

	  //open file
	  fd[fd_number] = open(optarg,oflag);

	  if(fd[fd_number]==-1)
	    {
	      fprintf(stderr, "Cannot open the file.\n");
	      fd[fd_number] = -1;
	      fd_number++;
	      exit_status =1;
	      oflag=0;
	      break;
	    }
	  fd_number++;
	  oflag=0;

	  if(pflag)
	      printParentUsage(ru);

	  break;
	  
	  //pipe option
	case 'p':
	  if(pflag)
	    if(getrusage(RUSAGE_SELF,&ru)<0)
	      {
		fprintf(stderr, "Failed to call get usage.\n");
		exit(1);
	      }
	  //syntax check
	  if(!checkArgument(argc, argv))
	    {
	      fprintf(stderr, "Error: Pipes do not need an argument.\n");
	      exit_status = 1;
	      break;
	    }
	  int pipefd[2];
	  if(pipe2(pipefd, oflag) == -1)
	    {
	      fprintf(stderr, "Error: Cannot create a pipe.\n");
	      if(errno == EINVAL)//invalid flag
		fprintf(stderr, "Error: Invalid value in flags.\n");
	      exit_status = 1;
	      oflag = 0;
	    }
	  else
	    {
	      if(vflag)
		fprintf(stdout, "%s\n", argv[optind-1]);
	      fd[fd_number] = pipefd[0];
	      isPipe[fd_number] = 1;
	      fd_number++;
	      fd[fd_number] = pipefd[1];
	      isPipe[fd_number] = 1;
	      fd_number++;
	      oflag = 0;	      
	    }
	  if(pflag)
	    printParentUsage(ru);
	  break;
	      
	case 'c':
	  {
	    if(pflag)
	    if(getrusage(RUSAGE_SELF,&ru)<0)
	      {
		fprintf(stderr, "Failed to call get usage.\n");
		exit(1);
	      }
	    int cmd = optind + 2;
	    if(cmd >= argc)
	      {
		fprintf(stderr, "Need more arguments for --command.\n");
		exit_status = 1;
		break;
	      }
	    
	    long int c_in, c_out, c_err;
	    c_in = strtol(argv[cmd-3],NULL,10);
	    c_out = strtol(argv[cmd-2],NULL,10);
	    c_err = strtol(argv[cmd-1],NULL,10);
	  
	    if(!(isValid(c_in,fd_number) && isValid(c_out,fd_number)
		 && isValid(c_err,fd_number)))
	      {
		fprintf(stderr, "Invalid file descriptor.\n");
		exit_status = 1;
		break;
	      }
	    if((fcntl(fd[c_in],F_GETFL)==-1) ||
	       (fcntl(fd[c_out],F_GETFL) == -1) ||
	       (fcntl(fd[c_err],F_GETFL) == -1))
	      {
		fprintf(stderr, "Cannot access the file.\n");
		exit_status =1;
		break;
	      }
	    if(vflag) //if verbose option is set
	      fprintf(stdout, "--command %ld %ld %ld", c_in, c_out, c_err);
	    
	    char** cmd_arg = malloc(sizeof(char*)* argc);//
	    char** begin = cmd_arg;
	    start[command] = cmd;
	    
	    //arg of cmd
	    char* cmd_ptr = argv[cmd];
	    while((cmd<argc) && (*(cmd_ptr) != '-' || *(cmd_ptr+1) != '-'))
	      {
		if(vflag)
		  fprintf(stdout, " %s", argv[cmd]);
		*cmd_arg = argv[cmd]; //save arguments
		cmd_arg++;
		cmd++;
		cmd_ptr = argv[cmd];
	      }

	    *cmd_arg = NULL;	    //end of array
	    end[command] = cmd -1;
	    command++;
	    if(vflag)
	      fprintf(stdout, "\n");

	    //forking
	    int pid = fork();
	    int status;
	    if(pid==0) //child proc
	      {
		if(dup2(fd[c_in],0)<0)
		  {
		    fprintf(stderr, "Error duplicating input.\n");
		    exit_status = 1;
		  }
		if(dup2(fd[c_out],1)<0)
		  {
		    fprintf(stderr, "Error duplicating output.\n");
		    exit_status = 1;
		  }
		if(dup2(fd[c_err],2)<0)
		  {
		    fprintf(stderr, "Error duplicating stderr.\n");
		    exit_status = 1;
		  }
		for(i=0; i< fd_number ; i++)
		  close(fd[i]);
		  
		if(execvp(*begin,begin)<0)
		  {
		    fprintf(stderr, "Error executing the command.\n");
		    exit(1);
		  }

	      }
	    else if(pid>0)
	      {
		//parent
		proc_array = realloc(proc_array,(child_number+1)*sizeof(int*));
		proc_array[child_number] = pid;
		child_number++;

		if(isPipe[c_in]==1)
		  close(fd[c_in]);
		if(isPipe[c_out]==1)
		  close(fd[c_out]);
		if(isPipe[c_err]==1)
		  close(fd[c_err]);
		if(pflag)
		  printParentUsage(ru);
	      }
	    else
	      {
		fprintf(stderr, "Failed forking.\n");
		exit_status =1;
	      }
	      
	  }
	  break;
	case 'v':
	  if(checkArgument(argc,argv))
	    {
	      if(vflag)
		fprintf(stdout, "--verbose\n");
	      else
		vflag = 1;
	    }
	  else
	    {
	      fprintf(stderr, "Verbose does not require arguments.\n");
	      exit_status = 1;
	    }
	  break;

	  

	  //wait  (NOT FINISHED YET)
	case 'm':
	  if(pflag)
	    if(getrusage(RUSAGE_SELF,&ru)<0)
	      {
		fprintf(stderr, "Failed to call get usage.\n");
		exit(1);
	      }
	  if(!checkArgument(argc,argv))
	    {
	      fprintf(stderr, "Error: Wait does not require arguments.\n");
	      exit_status = 1;
	      break;
	    }
	  else
	    {
	      if(vflag)
		fprintf(stdout, "--wait\n");
	      for(i=0 ; i< fd_number; i++)
		close(fd[i]);

	      int status;
	      int exit_status;

	      for(i=0;i<child_number; i++)
		{
		  waitpid(proc_array[i], &status, 0);
		  if(WIFSIGNALED(status))
		    raise(WTERMSIG(status));
		  if(WIFEXITED(status))
		    {
		      exit_status = WEXITSTATUS(status);
		      fprintf(stdout, "%d", exit_status);
		      while(start[i]<=end[i])
			{
			  fprintf(stdout, " %s", argv[start[i]]);
			  start[i]++;
			}
		      fprintf(stdout, "\n");
		      if(exit_status>max_status)
			max_status = exit_status;
		    }
		}
	    }
	  if(pflag)
	    {
	      printParentUsage(ru);
	      printChildUsage();
	    }
	  break;

	  //close
	case 'n':
	  if(pflag)
	    if(getrusage(RUSAGE_SELF,&ru)<0)
	      {
		fprintf(stderr, "Failed to call get usage.\n");
		exit(1);
	      }
	  if(optarg[0] == '-' && optarg[1] == '-')
	    {
	      fprintf(stderr, "Error: Requires an argument.\n");
	      exit_status = 1;
	      break;
	    }
	  else if(argv[optind] != '\0' && argv[optind][0] != '-' &&
		  argv[optind][1] != '-')
	    {
	      fprintf(stderr, "Error: More than one argument.\n");
	      exit_status = 1;
	      break;
	    }
	  if(vflag)
	    fprintf(stdout, "%s %s\n", argv[optind-2], argv[optind-1]);
	  long N;
	  char* endptr = NULL;
	  N = strtol(argv[optind-1], &endptr, 10);
	  if(*endptr)
	    {
	      fprintf(stderr, "Error: Could not convert string to number.\n");
	      break;
	    }
	  if((N<0) || (N>(fd_number-1)))
	    {
	      fprintf(stderr, "Invalid file number.\n");
	      exit_status=1;
	      break;
	    }
	  close (fd[N]);
	  fd[N] = -1;
	  if(pflag)
	    printParentUsage(ru);
	  break;

	case 'o': // abort
	  if(vflag)
	    fprintf(stdout, "--abort\n");
	  if(checkArgument(argc,argv))
	    raise(SIGSEGV);
	  else
	    {
	      fprintf(stderr, "Error: Abort does not need arguments.\n");
	      exit_status = 1;
	    }
	  break;

	case 'q': // catch N
	  if(pflag)
	    if(getrusage(RUSAGE_SELF,&ru)<0)
	      {
		fprintf(stderr, "Failed to call get usage.\n");
		exit(1);
	      }
	  if(optarg[0] == '-' && optarg[1] == '-')
	    {
	      fprintf(stderr, "Error: Requires an argument.\n");
	      exit_status = 1;
	      break;
	    }
	  else if(argv[optind] != '\0' && argv[optind][0] != '-' &&
		  argv[optind][1] != '-')
	    {
	      fprintf(stderr, "Error: More than one argument.\n");
	      exit_status = 1;
	      break;
	    }
	  n_signal = atoi(argv[optind-1]);
	  if(vflag)
	    fprintf(stdout, "--catch %d\n", n_signal);
	  if(signal(n_signal, &handler) ==SIG_ERR)
	    fprintf(stderr, "Error handling signal.\n");
	  if(pflag)
	    printParentUsage(ru);
	  break;

	case 's': //ignore N
	  if(pflag)
	    if(getrusage(RUSAGE_SELF,&ru)<0)
	      {
		fprintf(stderr, "Failed to call get usage.\n");
		exit(1);
	      }
	  if(optarg[0] == '-' && optarg[1] == '-')
	    {
	      fprintf(stderr, "Error: Requires an argument.\n");
	      exit_status = 1;
	      break;
	    }
	  else if(argv[optind] != '\0' && argv[optind][0] != '-' &&
		  argv[optind][1] != '-')
	    {
	      fprintf(stderr, "Error: More than one argument.\n");
	      exit_status = 1;
	      break;
	    }
	  n_signal = atoi(argv[optind-1]);
	  if(vflag)
	    fprintf(stdout, "--ignore %d\n", n_signal);
	  if(signal(n_signal, SIG_IGN) ==SIG_ERR)
	    fprintf(stderr, "Error handling signal.\n");
	  if(pflag)
	    printParentUsage(ru);
	  break;

	case 't': //default N
	  if(pflag)
	    if(getrusage(RUSAGE_SELF,&ru)<0)
	      {
		fprintf(stderr, "Failed to call get usage.\n");
		exit(1);
	      }
	  if(optarg[0] == '-' && optarg[1] == '-')
	    {
	      fprintf(stderr, "Error: Requires an argument.\n");
	      exit_status = 1;
	      break;
	    }
	  else if(argv[optind] != '\0' && argv[optind][0] != '-' &&
		  argv[optind][1] != '-')
	    {
	      fprintf(stderr, "Error: More than one argument.\n");
	      exit_status = 1;
	      break;
	    }
	  n_signal = atoi(argv[optind-1]);
	  if(vflag)
	    fprintf(stdout, "--default %d\n", n_signal);
	  if(signal(n_signal, SIG_DFL) ==SIG_ERR)
	    fprintf(stderr, "Error handling signal.\n");
	  if(pflag)
	    printParentUsage(ru);
	  break;

	case 'u': // pause
	  if(pflag)
	    if(getrusage(RUSAGE_SELF,&ru)<0)
	      {
		fprintf(stderr, "Failed to call get usage.\n");
		exit(1);
	      }
	  if(checkArgument(argc,argv))
	    {
	      if(vflag)
		fprintf(stdout, "--pause\n");
	      if(pause() == -1)
		fprintf(stderr, "Failed to pause.\n");
	    }
	  else
	    {
	      fprintf(stderr, "Error: Pasue does not need arguments.\n");
	      exit_status = 1;
	    }
	  if(pflag)
	    printParentUsage(ru);

	  break;

	case 'y': //profile
	  if(checkArgument(argc,argv))
	    {
	      if(vflag)
		fprintf(stdout, "--profile\n");
	      pflag = 1;
	      
	    }
	  else
	    {
	      fprintf(stderr, "Error: Profile does not need arguments.\n");
	      exit_status = 1;
	    }
	case ':':
	case '?':
	default:
	  break;
	  
	}

      
    }
      
    
  if(max_status == 0)
    exit(exit_status);
  else
    exit(max_status);

}
