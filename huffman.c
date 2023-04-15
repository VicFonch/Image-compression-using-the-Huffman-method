#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "Imagen.h"

typedef struct node_t {
	struct node_t *left, *right;
	int freq;
	char c;
} *node;

int n_nodes = 0, qend = 1;
struct node_t pool[256] = {{0}};
node qqq[255], *q = qqq-1;
char *code[128] = {0}, buf[1024];
int input_data=0,output_data=0;


node new_node(int freq, char c, node a, node b)
{
	node n = pool + n_nodes++;
	if (freq != 0){
		n->c = c;
		n->freq = freq;
	}
	else {
		n->left = a, n->right = b;
		n->freq = a->freq + b->freq;
	}
	return n;
}

void qinsert(node n)
{
	int j, i = qend++;
	while ((j = i / 2)) {
		if (q[j]->freq <= n->freq) break;
		q[i] = q[j], i = j;
	}
	q[i] = n;
}

node qremove()
{
	int i, l;
	node n = q[i = 1];

	if (qend < 2) return 0;
	qend--;
	while ((l = i * 2) < qend) {
		if (l + 1 < qend && q[l + 1]->freq < q[l]->freq) l++;
		q[i] = q[l], i = l;
	}
	q[i] = q[qend];
	return n;
}

void build_code(node n, char *s, int len)
{
	static char *out = buf;
	if (n->c) {
		s[len] = 0;
		strcpy(out, s);
		code[(int)n->c] = out;
		out += len + 1;
		return;
	}

	s[len] = '0'; build_code(n->left,  s, len + 1);
	s[len] = '1'; build_code(n->right, s, len + 1);
}

FILE *hcode;

void import_file(FILE *fp_in, unsigned int *freq){
	char c,s[16]={0};
	int i = 0;
	while((c = fgetc(fp_in))!= EOF){
                freq[(int)c]++;
	}
	for (i = 0; i < 128; i++)
		if (freq[i]) qinsert(new_node(freq[i], i, 0, 0));
	while (qend > 2)
		qinsert(new_node(0, 0, qremove(), qremove()));
	build_code(q[1], s, 0);
}


void encode(FILE* fp_in, unsigned int *freq){

	char in,c,temp[20] = {0};
	int i,j = 0,k = 0,lim = 0;
	for(i = 0; i < 128; i++){
		if(freq[i])	lim += (freq[i]*strlen(code[i]));
	}
	output_data = lim;
	for(i=0; i < lim; i++){
		if(temp[j] == '\0'){
			in = fgetc(fp_in);
			strcpy(temp, code[in]);
			fprintf(hcode, "%s",code[in]);
			j = 0;
		}
		if(temp[j] == '1')
                        c = c|(1<<(7-k));
                else if(temp[j] == '0')
                        c = c|(0<<(7-k));
                else
                        fprintf(hcode,"ERROR: Wrong input!\n");
                k++;
		j++;
		if(((i + 1)%8 == 0) || (i == lim-1)){
                        k = 0;
                        c = 0;
                }
	}
}

void print_code(unsigned int *freq){
	int i;
	printf("\n---------CODE TABLE---------\n----------------------------\nCHAR  FREQ  CODE\n----------------------------\n");
	for(i=0; i<128; i++){
		if(isprint((char)i)&&code[i]!=NULL&&i!=' ')
			printf("%-4c  %-4d  %16s\n",i,freq[i],code[i]);
		else if(code[i]!=NULL){
			switch(i){
				case '\n':
					printf("\\n  ");
					break;
				case ' ':
					printf("\' \' ");
					break;
				case '\t':
					printf("\\t  ");
					break;
				default:
					printf("%0X  ",(char)i);
					break;
			}
			printf("  %-4d  %16s\n", freq[i], code[i]);
		}
	}
	printf("----------------------------\n");

}

bool cmp(char a[], char b[], int sz)
{
	for(int i = 0; i < sz; ++i)
		if(a[i] != b[i])return false;
	return true;
}

int check(char tmp[], int *freq, int sz)
{
	for(int i = 0; i < 128; ++i)
		if(freq[i] && sz == strlen(code[i]) && cmp(tmp,code[i],sz))return i;
	return -1;
}

void decode_text(FILE *fp_in, FILE *fp_out, unsigned int *freq, int lim)
{
	char cd, tmp[50] = {0};
	int sz = 0;

	for(int it = 0; it < lim; ++it)
	{
		cd = fgetc(fp_in);
		tmp[sz++] = cd;
		int c = check(tmp,freq,sz);
		if(c != -1)
		{
			fprintf(fp_out, "%c", (char)c);
			for(int i = 0; i < 50; ++i)tmp[i] = '\0';
			sz = 0;
		}
	}
}

int ch(char *t, int sz)
{
	int s = 0, p = 1;
	for(int i = sz-1; i>-1; --i)
	{
		s += p*(t[i]-'0');
		p *= 10;
	}
	return s;
}

void decode_img(FILE *fp_in, unsigned char **img, unsigned int *freq, int lim, int L1, int L2)
{
	char cd, tmp[50] = {0}, tmp2[10] = {0};
	int sz = 0, sz2 = 0;
	int r_i = 0, c_i = 0;

	for(int it = 0; it < lim; ++it)
	{
		cd = fgetc(fp_in);
		tmp[sz++] = cd;
		int c = check(tmp,freq,sz);
		if(c != -1)
		{
			char nxt = (char)c;
			if(nxt >='0' && nxt <= '9'){
				tmp2[sz2++] = nxt;
			}
			else if(sz2 > 0)
			{
				int x = ch(tmp2,sz2);
				img[r_i][c_i] = (unsigned char)x;
				++c_i;
				if(c_i==L2){
					c_i = 0;
					++r_i;
					if(r_i == L1)break;
				}
				for(int i = 0; i < sz2; ++i)tmp2[i] = '\0';
				sz2 = 0;

			}
			for(int i = 0; i<50; ++i)tmp[i] = '\0';
			sz = 0;
		}
	}
}

int fix_c(int x, char c)
{
	int s = c-'0', x1 = x;
	while(x > 0)
	{
		s = s*10;
		x/=10;
	}
	return s + x1;
}

int main(){
	FILE *fp_in, *fp_out;
	char file_name[50]={0};
	unsigned int freq[128] = {0}, i;
	int rows = 0, cols = 0, mx;

	system("clear");

	printf("Entre el archivo a comprimir:\n ");
	scanf("%s", file_name);


	if(strlen(file_name) >= 50){
		printf("ERROR: Nombre demasiado largo\n");
		return 0;
	}

	if((fp_in = fopen(file_name,"r"))==NULL){
		printf("\nERROR: Archivo vacio\n");
		return 0;
	}
	char c1 = fgetc(fp_in);
	char c2 = fgetc(fp_in);
	if(c1 == 'P' && c2 == '2')
	{
		while(c1 == 'P' || c1 == '\n' || c1 == '#')
		{
			while((c1 = fgetc(fp_in))!=EOF)
				if(c1 == '\n')break;
			c1 = fgetc(fp_in);

		}
		fscanf(fp_in, "%d%d", &cols, &rows);
		fscanf(fp_in, "%d\n", &mx);
		cols = fix_c(cols, c1);
	}
	else rewind(fp_in);
	import_file(fp_in,freq);


	print_code(freq);


	hcode = fopen("Huffman_Code.txt", "w");
	rewind(fp_in);
	c1 = fgetc(fp_in);
	c2 = fgetc(fp_in);
	if(c1=='P' && c2=='2')
	{
		while(c1 == 'P' || c1 == '\n' || c1 == '#')
		{
			while((c1 = fgetc(fp_in))!=EOF)
				if(c1=='\n')break;
			c1 = fgetc(fp_in);

		}
		fscanf(fp_in, "%d%d", &cols, &rows);
		fscanf(fp_in, "%d\n", &mx);
		cols = fix_c(cols, c1);
	}
	else rewind(fp_in);
	encode(fp_in,freq);

	fclose(fp_in);
	fclose(hcode);

	hcode = fopen("Freq.txt", "w");
	for(int i = 0; i<128; ++i)if(freq[i])
	{
		fprintf(hcode, "%c: %d\n", (char)i, freq[i]);
	}
	fclose(hcode);

	for(i = 0; i < 128; i++)	input_data += freq[i];
	double exp = 0;
	for(i = 0; i < 128; ++i)
		if(freq[i])exp += (1.0*freq[i]/input_data)*strlen(code[i]);
	printf("\nCodigo Esperado lenght:\t%lf\n", exp);
	printf("\nBits Per Pixel:\t%lf\n", (1.0*output_data)/input_data);
	double bpp = (1.0*output_data)/input_data;

	printf("\nTotal en bytes de la entrada:\t\t%d\n",input_data);
	int lim = output_data;
	output_data = (output_data%8)? (output_data/8)+1 : (output_data/8);
	printf("Total en bytes de la salida:\t\t%d\n",output_data);

	printf("\nRadio de Compresion:\t%.2f%%\n\n\n",(double)input_data/(double)output_data);
	printf("\n(8-bpp)/8 = %lf\n\n", ((8-bpp)/8)*100);

	fp_in = fopen("Huffman_Code.txt", "r");
	if(fp_in==NULL){
		printf("\nERROR: No such file\n");
		return 0;
	}
	if(rows != 0 && cols != 0)
	{
		unsigned char **img = (unsigned char**)calloc(rows,sizeof(unsigned char*));
		for(int i = 0; i<rows; ++i)
			img[i] = (unsigned char*)calloc(cols,sizeof(unsigned char));
		for(int i = 0; i<rows; ++i)
			for(int j = 0; j<cols; ++j)
				img[i][j] = '\0';
		decode_img(fp_in,img,freq,lim,rows,cols);
		pgmWrite("Huffman_Decode.pgm",rows,cols,img,NULL);
	}
	else{
		fp_out = fopen("Huffman_Decode.txt", "w");
		decode_text(fp_in,fp_out,freq, lim);
		fclose(fp_out);
	}
	fclose(fp_in);

	return 0;
}
