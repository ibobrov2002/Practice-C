#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

struct tree{
	char* word;
	int count;
	struct tree *left, *right;
};
struct tree* sort(char* a, struct tree *p){
	int cmp;
	if (p){
		cmp=strcmp(a, p->word);
		if (cmp>0)
			p->right=sort(a, p->right);
		else 
			if (cmp<0)
				p->left=sort(a, p->left);
			else {
				(p->count)++;
				free(a);
			}
	}else{
		p=(struct tree*)malloc(sizeof(struct tree));
		p->word=a;
		p->count=1;
		p->right=NULL;
		p->left=NULL;
	};
	return p;	
};
int fdmax(struct tree *p, int max){
	if (p){
		max=fdmax(p->right, max);
		max=fdmax(p->left, max);
		if ((p->count)>max)
			max=p->count;
	}
	return max;
}
void bypass(struct tree *p, struct tree*** a){
	if (p){
		int i;
		for(i=0; a[(p->count)][i]!=NULL; i++);
		a[(p->count)]=(struct tree**)realloc(a[(p->count)],sizeof(struct tree*)*(i+2));
		a[(p->count)][i]=p;
		a[(p->count)][i+1]=NULL;
		bypass(p->right, a);
		bypass(p->left, a);
	}
}
void write(int length, struct tree *dict, FILE *out){
	struct tree ***a;
	int max=0, i, j;
	max=fdmax(dict, max);
	a=(struct tree***)malloc(sizeof(*a)*(max+1));
	for(i=0; i<max; i++){
		a[i+1]=(struct tree**)malloc(sizeof(*(a[i+1]))*1);
		a[i+1][0]=NULL;
	}
	bypass(dict, a);
	for(i=max; i>0; i--){
		for(j=0; a[i][j]!=NULL; j++){
			fprintf(out, "%s %d %f\n", a[i][j]->word, i, i/(double)length);
			free(a[i][j]->word);
			free(a[i][j]);
		}
		free(a[i]);
	}
	free(a);
}
int main(int argc, char *argv[]){
	int i=1, lw, c, length=0;
	struct tree *dict=NULL;
	FILE *inp=stdin, *out=stdout;
	for(; i<argc; i++) { 
		if (strcmp(argv[i], "-i")== 0){
			if (!(inp=fopen(argv[i+1], "r"))){
				printf("Cannot open input file.\n");
				return 1;
			};
		};
		if (strcmp(argv[i], "-o")== 0){
			if (!(out=fopen(argv[i+1], "w"))){
				printf("Cannot open output file.\n");
				return 1;
			};
		};
	};
	c=fgetc(inp);
	while(c!=EOF){
		while((c=='\n')||(c=='\t')||(c==' ')){
			c=fgetc(inp);
		};
		i=1;
		lw=20;
		char* a=(char*)malloc(sizeof(char)*lw);
		while((c!='\n')&&(c!='\t')&&(c!=' ')&&(c!=EOF)&&(!ispunct(c))){
			if (!(i%lw))
				a=(char*)realloc(a, sizeof(char)*(i+lw));
			a[i-1]=c;
			c=fgetc(inp);
			i++;
		};
		a[i-1]=0;
		if (a[0]!=0){
			dict=sort(a, dict);
			length++;
		}else
			free(a);
		if (ispunct(c)){
			a=(char*)malloc(sizeof(char)*2);
			a[0]=c;
			a[1]=0;
			dict=sort(a, dict);
			length++;
			c=fgetc(inp);
		}
	}
	write(length, dict, out);
	if (inp!=stdin)
		fclose(inp);
	if (out!=stdout)
		fclose(out);
	return 0;
}
