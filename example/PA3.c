#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <ctype.h>
#include "../include/libstemmer.h"

/*
	 일단 , k=50이라고 가정
smoothing : 적당한 k 값 구하기
 P(X|S) : the probability that a Negative messages contains X
 P(X|ㄱS): the probability that a non-negative messages contains X
 단어당해당하는 확률이랑 같이 나열해주
 𝑃 𝑋 𝑆) = (𝑘 + #-negatives-with-𝑤 ) / (2𝑘 + #-negatives)
 */

int total;
float sum=0;

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
	char * t = key ;
	int * d = value; // k=30
	float prob_neg;
 	prob_neg	= (float)(*d) / (float)(total+60);

  strwrSub(t);
	printf("(%s, %d)-%f \n", t, *d, prob_neg) ;
	sum += prob_neg;
}

int 
main () 
{
	FILE * f = fopen("../data/train.non-negative.csv", "r") ;

	struct sb_stemmer *stemmer;
//	char buf[1024];

	stemmer = sb_stemmer_new("english", 0x0);

	GHashTable * counter = g_hash_table_new(g_str_hash, g_str_equal) ;
	/*
	 지금 negative data들의 단어장을 만들었다.
		우리는 하나의 hashtable에 단어 nega, non-nega의 정보를 담고 싶은데
		value가 두개 들어가는게 가능한가?
	 */
	char * line = 0x0 ;
	size_t r ; 
	size_t n = 0 ;
	int skip_count=0;

	while (getline(&line, &n, f) >= 0) {
		int check_alpha=0;
		char * t ;
		char * _line = line ;

		for (t = strtok(line, " \n\t") ; t != 0x0 ; t = strtok(0x0, " \n\t")) {
			int * d ;
			for(int i=0 ; i<strlen(t) ; i++) {
				if(!isalpha(t[i])) {
					check_alpha=1;
					skip_count++;
					break;
				}
			}		
			// 알파벳을 제외한 문자가 들어가면 제외하고, 그 외의것만 table에 저장
			if(check_alpha==0){	
							
			//////////stemmer하는 과정//////////
					const	char* s;
				//	buf = t;
				//	strcpy(buf, t);
					s = sb_stemmer_stem(stemmer, t , strlen(t));
					t = (char*)s;
			/////////////////////////////

			
				d = g_hash_table_lookup(counter, t) ;
				if (d == NULL) {
					d = malloc(sizeof(int)) ;
					*d = 30 ; // k=30
					g_hash_table_insert(counter, strdup(t), d) ;
				}
				else {
					*d = *d + 1 ;
				}
			}
		}
		free(_line) ;
		line = 0x0 ;
	}
	/*
	 table에 이미 단어가 있으면 이어 붙이기 --> 그럼 앞에거를 string형태로 저장
	 단어가 없으면null & 확률로 저장

	 */

	total = g_hash_table_size(counter);
	g_hash_table_foreach(counter, print_counter,0x0) ;
	printf("worst: %d\n", *((int *) g_hash_table_lookup(counter, "is"))) ;
//	printf("skip count : %d\n", skip_count);
//	printf("total: %d\n", total);
	printf("hash table size: %d", total);
	printf("prob sum: %f\n", sum);

	fclose(f) ;
}
