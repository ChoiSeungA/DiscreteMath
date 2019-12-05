#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <ctype.h>

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
	int * d = value ;
  strwrSub(t);

	printf("(%s, %d)\n", t, *d) ;
}

int 
main () 
{
	FILE * f = fopen("../data/train.negative.csv", "r") ;

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
				d = g_hash_table_lookup(counter, t) ;
				if (d == NULL) {
					d = malloc(sizeof(int)) ;
					*d = 1 ;
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

	g_hash_table_foreach(counter, print_counter, 0x0) ;
	printf("worst: %d\n", *((int *) g_hash_table_lookup(counter, "is"))) ;
	printf("skip count : %d\n", skip_count);

	fclose(f) ;
}
