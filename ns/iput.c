#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <queue>
#include <dirent.h>
#include "Cgetopt.h"
#include "Cns_api.h"
#include "u64subr.h"
#define VERSION 0.10

int cmflag=0;
int rflag=0;
int Vflag=0;
int Rflag=0;
int vflag=0;

int helpflg=0;

struct timeval first, last, finish;

float computelapse(struct timeval ftime,struct timeval etime)
{
        struct timeval lapsed;
        long start, end;
        float tsend;
        start =  ftime.tv_sec*1000000 + ftime.tv_usec;
        end =  etime.tv_sec*1000000 + etime.tv_usec;
        tsend = (float) (end - start)/1000000;
        //fprintf(stdout,"tsend is \t%.3fs\n",tsend);
        return tsend;
}

int procput(char *sourcepath, char *targetpath)
{
	int res;
	struct timezone tzp;
	float tsend;
	char path_buff[1024];
	u_signed64 total_bytes=0;
	struct stat statbuf;
	if(realpath(sourcepath, path_buff)==NULL){
		fprintf(stderr, "%s: no such file/directory\n", sourcepath);
		res=-1;
		return res;
	}
        stat(path_buff, &statbuf);
        total_bytes=statbuf.st_size;
	if(Rflag){
		char *path_tmp=(char *)malloc(strlen(path_buff)+1);
		char *basename_tmp=(char *)malloc(strlen(path_buff)+1);
		char *path_tmp2=(char *)malloc(strlen(targetpath)+1);
		strcpy(path_tmp, path_buff);
		splitname(path_tmp, basename_tmp);
		sprintf(basename_tmp, "%s%s", basename_tmp, "_ul");
		strcpy(path_tmp2, targetpath);
		res=xrd_unlink(path_tmp2, basename_tmp);
		free(path_tmp);
		free(basename_tmp);
		free(path_tmp2);
		if(res){
			fprintf(stderr,"xrd_refresh cache %s failed\n",path_buff);
			return res;
		}
	}
	gettimeofday(&first, &tzp);
	res=xrd_rfsync(path_buff, targetpath);
	gettimeofday(&finish, &tzp);
	tsend=computelapse(first, finish);
	if(vflag){
		if(res==0){
			char tmpbuf[21];
			bzero(tmpbuf, 21);
			u64tostr(total_bytes, tmpbuf, 0);
			fprintf(stdout, "Upload %s success:\n", path_buff);
			fprintf(stdout,"%s bytes transferred in %.2f seconds (%.2f MB/s)\n", tmpbuf,
                        tsend, (float)total_bytes/(tsend*1024*1024));
		}
	}
	if(res==0)
		return res;		
        else if(res==ENOENT){
        	fprintf(stderr, "%s: no such file/directory\n", sourcepath);
		res=-1;
        }else if(res==EISDIR){
                res=1;
        }else if(res==-ENOTDIR){
                fprintf(stderr, "%s: must be a directory\n", targetpath);
                exit(USERR);
        }else if(res==-ENOENT){
                fprintf(stderr, "%s: no such direcoty\n", targetpath);
                exit(USERR);
        }else if(res==-EEXIST){
                fprintf(stderr, "%s: file already upload\n", sourcepath);
                res=-1;
        }else if(res==EEXIST){
		fprintf(stderr, "%s: file already created\n", sourcepath);
		res=-1;
	}else if(res==EACCES){
		fprintf(stderr, "%s: permission denied\n", targetpath);
		res=-1;
	}else if(res==-1){
                fprintf(stderr, "Error: iput to %s failure\n", targetpath);
                exit(USERR);
        }else{
                fprintf(stderr, "%s: input failure\n", sourcepath);
                res=-1;
        }
	return res;
}

int procdir(char *sourcepath, char *targetpath)
{
	int res;
	queue <string> dir;
	string path;
	char fullpath[CA_MAXPATHLEN+1];
	DIR *directory;
	struct dirent *ptr;
	struct stat st;
	int len=strlen(sourcepath);
	if(sourcepath[len-1]=='/')
		sourcepath[len-1]='\0';
	dir.push(sourcepath);
	while(!dir.empty()){
		path=dir.front();
		dir.pop();
		if((directory=opendir(path.c_str()))==NULL){
			fprintf(stderr, "%s: open dir error\n", path.c_str());
			res=-1;
			continue;	
		}else{
			while((ptr=readdir(directory))!=NULL){
				if(strcmp(ptr->d_name, ".")==0 || strcmp(ptr->d_name, "..")==0)
					continue;
				else{
					sprintf(fullpath, "%s/%s", path.c_str(), ptr->d_name);
					stat(fullpath, &st);
					if(S_ISDIR(st.st_mode))
						dir.push(fullpath);
					else
						res=procput(fullpath, targetpath);
				}
			}
		}
		closedir(directory);
	}
	return res;
}

int main(int argc, char * argv[])
{
	int c;
	int i;
	int errflg=0;
	Coptind=1;
	Copterr=1;
	int res;
	char targetpath[CA_MAXPATHLEN+1];
	char sourcepath[CA_MAXPATHLEN+1];
	char actual_path[CA_MAXPATHLEN+1];
	int filesize;
	struct stat s_buf;
	static struct Coptions longopts[]={
		{"comment", NO_ARGUMENT, &cmflag, 1},
		{"version", NO_ARGUMENT, &Vflag, 1},
		{"help",    NO_ARGUMENT, &helpflg, 1},
		{"verbose", NO_ARGUMENT, &vflag, 1},
		{"refresh", NO_ARGUMENT, &Rflag, 1},
		{0, 0, 0, 0}
	};
	/*for args*/
	while((c=Cgetopt_long (argc, argv, "rvVRH", longopts, NULL)) != EOF){
		switch(c){
			case 'r':
				rflag++;
				break;
			case 'V':
				Vflag++;
				break;
			case 'H':
				helpflg++;
				break;
			case 'v':
				vflag++;
				break;
			case 'R':
				Rflag++;
				break;
			case '?':
				errflg++;
				break;
			default:
				break;
		}
	}
	if(Vflag){
		fprintf(stdout,"Version: %.2f\n", VERSION);
		exit(0);
	}
	if(Coptind >= argc-1 && !errflg){
		errflg++;	
	}
	if(errflg){
		fprintf (stderr,
                    "usage: %s [-r] [--comment] files/directories r_directory...\ninfo: upload files to the remote station\n",
                        argv[0]);
		exit(USERR);
	}
	/*for targetpath*/
	strcpy(targetpath, argv[argc-1]);
	/*for sourcepath*/
	for(i=Coptind; i<argc-1; i++){
		strcpy(sourcepath, argv[i]);
		res=procput(sourcepath, targetpath);
		if(res==0)
			continue;
		else if(res==-1){
			errflg++;
		}else if(res==1){
			if(!rflag){
				fprintf(stderr, "%s: directory must be with -r\n", sourcepath);
				errflg++;
			}else{
				res=procdir(sourcepath, targetpath);
				if(res!=0)
					errflg++;
			}	
		}
	}

	if(errflg)
		exit(USERR);
	exit(0);
}
