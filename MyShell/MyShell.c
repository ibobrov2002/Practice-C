#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>

int contrlc(char c){
	if ((c=='&')||(c=='|')||(c==';')||(c=='>')||(c=='<')||(c=='(')||(c==')'))
		return 0;
	else 
		return 1;
}

int conv(char** str, int f0, int f1){
	int fd[2], p;
	if(!contrlc(str[0][0])){
		fprintf(stderr, "ошибка");
		return 1;
	};
	int i=0, pr=0, status;
	char* cc=NULL;
	for(;(str[i]!=NULL);){
		int j=0, knc=1, knc2;
		for(;((str[i]!=NULL)&&(contrlc(str[i][0])));i++,j++);
		if ((str[i]!=NULL)&&((str[i][0]=='&')||(str[i][0]==';')||(str[i][0]=='(')||(str[i][0]==')')||(!(strcmp(str[i], "||"))))){
			cc=(char*)malloc(3);
			cc[0]=str[i][0];
			if ((cc[1]=str[i][1])!=0)
				cc[2]=0;
		}
		knc2=i-j;
		if((str[i]!=NULL)&&(str[i][0]=='|')&&(strcmp(str[i], "||"))){
			free(str[i]);
			str[i]=NULL;
			i++;
		}else{
			knc=0;
			if (str[i]){
				free(str[i]);
				str[i]=NULL;
			}
		}
		pipe(fd);
		pr++;
		p=fork();
		if (p==-1){
			perror("ошибка");
			return 1;
		}else if (p==0){
				if(f0!=-1){
					dup2(f0,0);
				}
				close(fd[0]);
				close(f0);
				if(knc){
					dup2(fd[1], 1);
				}else if (f1!=-1)
						dup2(f1, 1);
				close(fd[1]);
				execvp(str[knc2], &(str[knc2]));
				perror("ошибка");
				exit(1);
			}else{
				close(f0);
				f0=fd[0];
				close(fd[1]);
			}
	}
	if (cc)
		str[i]=cc;
	close(f0);
	for(i=0;i<pr; i++, wait(&status));
	return status;
}
int main(int argc, char *argv[]){
	FILE* inp=stdin;
	char* home=getenv("HOME");
	if(argc>1){
		if (!(inp=fopen(argv[1], "r"))){
			printf("Cannot open input file.\n");
			return 1;
		}
	}
	int c, lw=10, i, lp=2, status=777;
	int* pid=(int*)malloc(sizeof(int)*10);
	printf("$");
	while ((c=fgetc(inp))!=EOF){
		char** str;
		int nw=1;
		str=(char**)malloc(sizeof(char*)*1);
		while ((c!='\n')&&(c!=EOF)){
			while((c=='\t')||(c==' ')){
				c=fgetc(inp);
			};
			i=1;
			char* wrd;
			wrd=(char*)malloc(sizeof(char)*lw);
			while((c!='\n')&&(c!='\t')&&(c!=' ')&&(c!=EOF)&&(contrlc(c))){
				if (!(i%lw))
					wrd=(char*)realloc(wrd, sizeof(char)*(i+lw));
				if (c=='"'){
					c=fgetc(inp);
					while((c!='"')&&(c!=EOF)&&(c!='\n')){
						if (!(i%lw))
							wrd=(char*)realloc(wrd, sizeof(char)*(i+lw));
						wrd[i-1]=c;
						i++;
						c=fgetc(inp);
					}
				}
				if (c!='\n'){
					if (c!='"'){
						wrd[i-1]=c;
						i++;
					};
					c=fgetc(inp);
				}
			}
			wrd[i-1]=0;
			if (wrd[0]!=0){
				str[nw-1]=wrd;
				nw++;
				str=(char**)realloc(str,sizeof(char*)*nw);
			}else
				free(wrd);
			if (!contrlc(c)){
				int p=fgetc(inp);
				str[nw-1]=(char*)malloc(sizeof(char)*3);
				str[nw-1][0]=c;
				if (((c=='&')&&(p=='&'))||((c=='|')&&(p=='|'))||((c=='>')&&(p=='>'))){
					str[nw-1][1]=p;
					str[nw-1][2]=0;
					c=fgetc(inp);
				}else {
					str[nw-1][1]=0;
					c=p;
				}
				nw++;
				str=(char**)realloc(str,sizeof(char*)*nw);
			}
		}
		int f0=-1, f1=-1;
		str[nw-1]=NULL;
		if(str[0]){
			if(strcmp(str[0], "cd")){
				int i=0;
				int j=0;
				for(;((str[j]!=NULL)&&(contrlc(str[j][0]))); j++);
				if((str[j]!=NULL)&&(!(strcmp(str[j], "&")))){
					int p;
					free(str[j]);
					str[j]=NULL;
					p=fork();
					if (p==-1){
						perror("ошибка");
						return 1;
					}else if (p==0){
						int fnull;
						if ((fnull=open("/dev/null", O_RDWR))==-1)
							perror("Ошибка");
						dup2(fnull, 0);
						dup2(fnull, 1);
						signal(SIGINT, SIG_IGN);
						execvp(str[0], str);
						perror("ошибка");
						exit(1);
						}else{
							if(!lp%10)
								pid=(int*)realloc(pid, sizeof(int)*(lp+10));
							pid[lp-2]=p;
							lp++;
						}
				}else{
					int stat2=1;//1-запуск, 0 - пропуск
					for(;str[i]!=NULL;){
						if(stat2){
							if (!(contrlc(str[i][0]))){
								if((str[i][0]=='>')||(str[i][0]=='<')){
									if (str[i][0]=='<'){
										if ((f0=open(str[i+1], O_RDONLY, 0777))==-1)
											perror("Ошибка");
									}else if(strcmp(str[i], ">>"))
											f1=open(str[i+1], O_CREAT | O_WRONLY | O_TRUNC, 0777);
										else
											f1=open(str[i+1], O_CREAT | O_WRONLY | O_APPEND, 0777);
									i=i+2;
								}else 
									fprintf(stderr, "Ошибка1");
							}
							if (!(contrlc(str[i][0]))){
								if((str[i][0]=='>')||(str[i][0]=='<')){
									if (str[i][0]=='<'){
										if (f0==-1){
											if ((f0=open(str[i+1], O_RDONLY, 0777))==-1)
												perror("Ошибка");
										}else 
											fprintf(stderr, "Ошибка");
									}else if (f1==-1){
											if(strcmp(str[i], ">>"))
												f1=open(str[i+1], O_CREAT | O_WRONLY | O_TRUNC, 0777);
											else
												f1=open(str[i+1], O_CREAT | O_WRONLY | O_APPEND, 0777);
										}else 
											fprintf(stderr, "Ошибка");
										i=i+2;
								}else 
									fprintf(stderr, "Ошибка2");
							};
							status=conv(&(str[i]), f0, f1);
							f0=-1;
							f1=-1;
						}
						for(;((i!=nw-1)&&((str[i]==NULL)||((str[i][0]!='&')&&(str[i][0]!=';')&&(str[i][0]!='(')&&(str[i][0]!=')')&&(strcmp(str[i], "||"))))); i++);	
						if((str[i]!=NULL)&&(!strcmp(str[i], "&&"))){
							if(status==0)
								stat2=1;
							else
								stat2=0;
						}
						if((str[i]!=NULL)&&(!strcmp(str[i], "||"))){
							if(status)
								stat2=1;
							else
								stat2=0;
						}
						if((str[i]!=NULL)&&(!strcmp(str[i], ";"))){
							stat2=1;
						}
						if (i!=nw-1){
							i++;
						}
					}
				}
				
				
			}else{
				if (str[1]==NULL)
					chdir(home);
				else 
					if (chdir(str[1])==-1)
						perror("ошибка");
			}
		}
		int i;
		for(i=0; i<lp-2; i++){
			if (pid[i])
				if (pid[i]==waitpid(pid[i], &status, WNOHANG)){
					printf("%d %d\n", pid[i], status);
					pid[i]=0;
				}
		};
		for (i=0; i<nw-1; i++)
			if (str[i])
				free(str[i]);
		free(str);
		printf("$");
	}
	free(pid);
	if (inp!=stdin)
		fclose(inp);
		
	return 0;
}
