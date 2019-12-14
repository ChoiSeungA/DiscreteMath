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
int neg_total;
int nonneg_total;
float sum=0;
int count=0;
int k=100;


void strwrSub(char str[]) {
	int i=0;
	while(str[i]){
		if(str[i]>='A' && str[i]<='Z'){
			str[i]=str[i]+32;
		}
		i++;
	}
}


void 
print_counter (gpointer key, gpointer value, gpointer userdata) 
{
	FILE *fp = fopen("model.csv", "a");
	count++;
	char * t = key ;
	float * d = value; 
	float prob_neg, prob_nonneg;
	prob_nonneg	= (d[0]) / (nonneg_total+k);
	double prob_nonneg1 = log10(prob_nonneg);
	prob_neg = (d[1]) / (neg_total+2*k);
	double prob_neg1;
  prob_neg1= log10(prob_neg);
	double result = log10(10.0);
  strwrSub(t);
	d[2] = prob_nonneg;
	d[3] = prob_neg;
	if(d[0]<40 || d[1]<40)
	printf("(%s, %.0f, %.0f)-%f %f\n", t, d[0],d[1], prob_nonneg, prob_neg) ;

	fprintf(fp, "%s %f %f  \n", t, d[2], d[3]);
	fclose(fp);
}


void total_counter (gpointer key, gpointer value, gpointer userdata) {
	float *d = value;
	if(d[0] > k/2) nonneg_total++;
	if(d[1] > k) neg_total++;
}

int 
main () 
{
	FILE * f = fopen("../data/train.non-negative.csv", "r") ;
	FILE * f_neg = fopen("../data/train.negative.csv", "r") ;
	FILE *test = fopen("../data/test.negative.csv", "r");
	FILE *test_nonneg = fopen("../data/test.non-negative.csv", "r");
	FILE *stop = fopen("stop.txt", "r");
	neg_total=0;
	nonneg_total=0;

	int empty = 0, empty_non = 0, positive=0, negative=0, positive_non=0, negative_non=0;
	int i;
	float *d;
	struct sb_stemmer *stemmer;
	stemmer = sb_stemmer_new("english", 0x0);

	GHashTable * counter = g_hash_table_new(g_str_hash, g_str_equal) ;

	char * line = 0x0 ;
	size_t r ; 
	size_t n = 0 ;
	int skip_count=0;


//non-negative data
	while (getline(&line, &n, f) >= 0) {
		int check_alpha=0;
		char * t ;
		char * _line = line ;

		for (t = strtok(line, " \n\t") ; t != 0x0 ; t = strtok(0x0, " \n\t")) {
			for(int i=0 ; i<strlen(t) ; i++ ){
					if(!isalpha(t[i])){
						check_alpha = 1;
						break;					
					}	
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
					d[0] = (k/2)+1 ; // k=30
					d[1] = (k);
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

//negative_data

	while(getline(&line, &n, f_neg)>= 0) {
		int check_alpha = 0;
		char *t;
		char * _line = line;
		
		for(t = strtok(line, " \n\t") ; t!=0x0 ; t=strtok(0x0, " \n\t")) {
			float *d;	
			for(int i=0 ; i<strlen(t) ; i++) {
				if(!isalpha(t[i])) {
					if(i==strlen(t) -1 && ((t[i]=='!') || (t[i] == ',') || (t[i] == '?') || (t[i] == '.'))) {
						check_alpha = 0;
						t[i] = '\0';
						break;
					}				
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
					d[0]=k/2;
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

	//////////////////////////////////////
  char *lin = 0x0;
	size_t na = 0;

	while(getline(&lin, &na, stop) >=0 ) {
		lin[strlen(lin)-1] = '\0';
		if(g_hash_table_contains(counter, lin) >= 0) {
			g_hash_table_remove(counter, lin);
		}
	}

	//////////////////////////////////
	char *test_line = 0x0;

	g_hash_table_foreach(counter, total_counter, 0x0);
	g_hash_table_foreach(counter, print_counter,0x0) ;


	printf("\n\n\n///////////////////////////negative data set//////////////////////\n\n\n");
	while(getline(&test_line, &n, test) >= 0) {
	//라인을 한 줄씩 받음
		char *t;
		char* _test = test_line;
		int check_alpha=0;
		float neg_prob=0, nonneg_prob=0;
		printf("%s", test_line);
		for(t = strtok(test_line, " \n\t") ; t!=0x0 ; t=strtok(0x0, " \n\t")) {
			check_alpha = 0;
			for(int i=0 ; i<strlen(t) ; i++)
			{
				if(!isalpha(t[i])) {
					if(i==strlen(t) -1 && ((t[i]=='!') || (t[i] == ',') || (t[i] == '?') || (t[i] == '.'))) {
						check_alpha = 0;
						t[i] = '\0';
						break;
					}				
					check_alpha=1;
				}	
			}

			if(check_alpha == 0) {
				strwrSub(t);

				const char *s;
				s = sb_stemmer_stem(stemmer, t, strlen(t));
				t = (char*) s;

				if(g_hash_table_contains(counter, t)) {
					float* temp = g_hash_table_lookup(counter, t);
				//	printf("%s:  %.3f %.3f, %.0f, %.0f\n",t,  temp[2], temp[3], temp[0], temp[1]); 
					if(temp[2] != 0)	nonneg_prob+=log10(temp[2]);
					if(temp[2] == 0) nonneg_prob += 0;
					if(temp[3] != 0) neg_prob += log10(temp[3]);
					if(temp[3] == 0) neg_prob += 0;
				}
			}
		}
		double result_nonneg = pow(10, nonneg_prob) / (pow(10, nonneg_prob) + pow(10, neg_prob)); // message가 nonneg문장일 확률
		double result_neg = pow(10, neg_prob) / (pow(10, neg_prob) + pow(10, nonneg_prob)); // message가 nonneg문장일 확률
		printf("nonneg: %f, neg: %f  \n",result_nonneg, result_neg);
		if(result_neg > 0.65)	{
						negative++;
						printf("result: negative\n\n");
		}
		else if( result_nonneg ==  result_neg ) {
						empty++;
						printf("without empty case\n\n");
		}
		else	{
						positive++;
						printf("result: nonnegative\n\n");
		}
	}
	
	float final_negative_neg = negative / (float)(100-empty);
	float final_nonneg_neg = positive / (float)(100-empty);

//	printf("negative : %f ,  positive: %f, empty: %d\n", final_negative, final_nonneg, empty);
//	printf("neg count: %d, nonneg count: %d\n", negative, positive);

	printf("\n\n\n//////////////nonneg data set///////////////////\n\n\n");

	while(getline(&test_line, &n, test_nonneg) >= 0) {
	//라인을 한 줄씩 받음
		char *t;
		char* _test = test_line;
		int check_alpha=0;
		float neg_prob=0, nonneg_prob=0;
		printf("%s", test_line);
		for(t = strtok(test_line, " \n\t") ; t!=0x0 ; t=strtok(0x0, " \n\t")) {
			check_alpha = 0;
			for(int i=0 ; i<strlen(t) ; i++)
			{
				if(!isalpha(t[i])) {
					if(i==strlen(t) -1 && ((t[i]=='!') || (t[i] == ',') || (t[i] == '?') || (t[i] == '.'))) {
						check_alpha = 0;
						t[i] = '\0';
						break;
					}				
					check_alpha=1;
				}	
			}

			if(check_alpha == 0) {
				strwrSub(t);

				const char *s;
				s = sb_stemmer_stem(stemmer, t, strlen(t));
				t = (char*) s;

				if(g_hash_table_contains(counter, t)) {
					float* temp = g_hash_table_lookup(counter, t);
				//	printf("%s:  %.3f %.3f, %.0f, %.0f\n",t,  temp[2], temp[3], temp[0], temp[1]); 
					if(temp[2] != 0)	nonneg_prob+=log10(temp[2]);
					if(temp[2] == 0) nonneg_prob += 0;
					if(temp[3] != 0) neg_prob += log10(temp[3]);
					if(temp[3] == 0) neg_prob += 0;
				}
			}
		}
		double result_nonneg = pow(10, nonneg_prob) / (pow(10, nonneg_prob) + pow(10, neg_prob)); // message가 nonneg문장일 확률
		double result_neg = pow(10, neg_prob) / (pow(10, neg_prob) + pow(10, nonneg_prob)); // message가 nonneg문장일 확률
		printf("nonneg: %f, neg: %f  \n",result_nonneg, result_neg);
		if(result_neg > 0.65)	{
						negative_non++;
						printf("result: negative\n\n");
		}
		else if( result_nonneg ==  result_neg ) {
						empty_non++;
						printf("without empty case\n\n");
		}
		else	{
						positive_non++;
						printf("result: nonnegative\n\n");
		}
	}
	
	float final_negative_nonneg = negative_non / (float)(100-empty_non);
	float final_nonneg_nonneg = positive_non / (float)(100-empty_non);
	
	printf("------------negative test data set result------------\n");
	printf("negative : %f ,  positive: %f, empty: %d\n\n\n", final_negative_neg, final_nonneg_neg, empty);
	
	
	printf("------------nonnegative test data set result------------\n");
	printf("negative : %f ,  positive: %f, empty: %d\n\n\n", final_negative_nonneg, final_nonneg_nonneg, empty_non);


//////////////////////////////////
	fclose(f) ;
	fclose(f_neg);
	fclose(test);
	fclose(stop);
	fclose(test_nonneg);

}
