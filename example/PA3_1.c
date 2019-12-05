#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <ctype.h>
#include <math.h>
#include "../include/libstemmer.h"


/*
	 일단 , k=50이라고 가정
smoothing : 적당한 k 값 구하기
 P(X|S) : the probability that a Negative messages contains X
 P(X|ㄱS): the probability that a non-negative messages contains X
 단어당해당하는 확률이랑 같이 나열해주
 𝑃 𝑋 𝑆) = (𝑘 + #-negatives-with-𝑤 ) / (2𝑘 + #-negatives)
 확률 계산까지 완료 이제 해야하는거는 
 	1. stop word 제외하기
	2. 특정 threshold값 정해서 너무 빈도 적은거랑 많은거 제외하기
	3. predictor 곱하기 계산하
 */


int total=0;
int neg_total=0;
int nonneg_total=0;
float sum=0;
int count=0;
int k=30;

struct Table {
	char *key;
	float neg;
	float nonneg;
};

struct Table * wordtable;

void strwrSub(char str[]) {
	int i=0;
	while(str[i]){
		if(str[i]>='A' && str[i]<='Z'){
			str[i]=str[i]+32;
		}
		i++;
	}
}

void total_counter (gpointer key, gpointer value, gpointer userdata) {
	int *d = value;
	if(d[0] != 30) nonneg_total++;
	else if(d[1] != 30) neg_total++;
	else if(d[0] != 30 && d[1] != 30)	total++;
}

void 
print_counter (gpointer key, gpointer value, gpointer userdata) 
{
	FILE *fp = fopen("model.csv", "a");
	count++;
	char * t = key ;
	int * d = value; // k=30
	float prob_neg, prob_nonneg;
	prob_nonneg	= (float)(d[0]) / (float)(nonneg_total+60);
	//double prob_nonneg1 = log10(prob_nonneg);
	prob_neg = (float)(d[1]) / (float)(neg_total+60);
	double prob_neg1;
  //prob_neg1= log10(prob_neg);
	double result = log10(10.0);
  strwrSub(t);
	if(d[0]<40 || d[1]<40)
	printf("(%s, %d, %d)-%f %f\n", t, d[0],d[1], prob_nonneg, prob_neg) ;
	if(((d[0] > 31 && d[1]>31)) && strlen(t)>2)
		fprintf(fp, "%s - %f %f\n", t, prob_nonneg, prob_neg);
	fclose(fp);
}


void probability(GHashTable *counter, char str[]) {
	struct sb_stemmer *stemmer;
	stemmer = sb_stemmer_new("english", 0x0);
	int check_alpha = 0;
	char *line;
	float non_guess = 1, neg_guess = 1;
	printf("%s\n", str);
	line = strtok(str, " \n\t");
//	printf("%s\n", t);
	while(line != NULL) {
		printf("%s\n", line);
		line = strtok(NULL, " \n\t");
	}
	for(line = strtok(str, " \n\t") ; line != 0x0 ; line = strtok(0x0, " \n\t")) {
		int *d ;
//		printf("%s\n", line);
		for(int i=0 ; i<strlen(line) ; i++) {
			if(!isalpha(line[i])) {
				check_alpha=1;
				break;
			}
		}
		if(check_alpha==0) {
			const char* s;
			s = sb_stemmer_stem( stemmer, line, strlen(line) );
			line = (char*) s;
			printf("%s ", line);
			d = g_hash_table_lookup(counter, line) ;
			if(d == NULL) {
				non_guess *= 1;
				neg_guess *= 1;
				printf("/");
			}
			else {
				non_guess *= ((float)d[0]/(float)nonneg_total);
				printf("%f %f \n", (float)d[0]/(float)nonneg_total, (float)d[1]/(float)neg_total);
				neg_guess *= ((float)d[1]/(float)neg_total);
			}
			
		//	printf("5\n");
		}
		printf("%f %f\n", non_guess, neg_guess);
	}
	//	printf("%d %d\n", nonneg_total, neg_total);
//	printf("guess non: %f, guess neg: %f", non_guess, neg_guess);
}


int 
main () 
{
	FILE * f = fopen("../data/train.non-negative.csv", "r") ;
	FILE * f_neg = fopen("../data/train.negative.csv", "r") ;
//	FILE *test = fopen("../data/test.non-negative.csv");
	neg_total=0;
	nonneg_total=0;

	int i;
	int *d;
	struct sb_stemmer *stemmer;
	stemmer = sb_stemmer_new("english", 0x0);

	GHashTable * counter = g_hash_table_new(g_str_hash, g_str_equal) ;

	char * line = 0x0 ;
	size_t r ; 
	size_t n = 0 ;
	int skip_count=0;

	while (getline(&line, &n, f) >= 0) {
		int check_alpha=0;
		char * t ;
		char * _line = line ;

		for (t = strtok(line, " \n\t") ; t != 0x0 ; t = strtok(0x0, " \n\t")) {
				if(!isalpha(t[i])) {
					check_alpha=1;
					skip_count++;
					break;
				}
			if(check_alpha==0){	
				 // nonneg_total++;			
			//////////stemmer하는 과정//////////
					const	char* s;
					s = sb_stemmer_stem(stemmer, t , strlen(t));
					t = (char*)s;
			/////////////////////////////
				d = g_hash_table_lookup(counter, t) ;
				if (d == NULL) {
					d = malloc(sizeof(int)*2) ;
					d[0] = k+1 ; // k=30
					d[1] = k ;
					g_hash_table_insert(counter, strdup(t), d) ;
				}
				else {
					d[0] = d[0] + 1 ;
				}
			}
		}
		free(_line) ;
		line = 0x0 ;
	}

	while(getline(&line, &n, f_neg)>= 0) {
		int check_alpha = 0;
		char *t;
		char * _line = line;
		
		for(t = strtok(line, " \n\t") ; t!=0x0 ; t=strtok(0x0, " \n\t")) {
			int *d;	
			for(int i=0 ; i<strlen(t) ; i++) {
				if(!isalpha(t[i])) {
					check_alpha = 1;
					break;
				}
			}

			if(check_alpha==0) {
				const char* s;
				s = sb_stemmer_stem(stemmer, t, strlen(t));
				t = (char*) s;
				d = g_hash_table_lookup(counter, t);

				if(d == NULL) {
					d = malloc(sizeof(int)*2);
					d[0]=k;
					d[1]=k+1;
					g_hash_table_insert(counter, strdup(t), d);
				}
				else 
					d[1] = d[1]+1;
			}
		}
		free(_line);
		line=0x0;
	}
	/*
	 table에 이미 단어가 있으면 이어 붙이기 --> 그럼 앞에거를 string형태로 저장
	 단어가 없으면null & 확률로 저장

	 */
	nonneg_total=0;
	neg_total=0;
	total=0;
	g_hash_table_foreach(counter, total_counter, 0x0);
	g_hash_table_foreach(counter, print_counter,0x0) ;
	printf("worst: %d\n", *((int *) g_hash_table_lookup(counter, "with"))) ;
	printf("hash table size: %d", g_hash_table_size(counter));
	printf("neg_total: %d, nonneg: %d", neg_total, nonneg_total);
//	char str[256] = "Flight 2646, Four hours in the plane on the ground at BWI";
//	probability(counter, str);

	fclose(f) ;
	fclose(f_neg);
}
