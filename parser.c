#include <ctype.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "parser.h"
#include "synthesis.h"

int string_to_semitone(char *note);

int parse(const char *msg, size_t len, char **answ, size_t *len_answ){
	fprintf(stderr,"parsing %s\n",msg);
	*answ = NULL;
	*len_answ = 0;
	unsigned int amplitude;
	int halftone;
	if(len == 0){
		return -1;
	}
	char note[4];
	switch(tolower(msg[0])){
		case 's':
			if( sscanf(msg,"set %3s %2x",note,&amplitude) != 2 ){
				return -1;
			}
			halftone = string_to_semitone(note);
			synthesis_set_frequency(halftone,(uint8_t) amplitude);
			fprintf(stderr,"set halftone %d to amplitude %d\n",halftone,amplitude);
			break;
/*		case 'g':
			if( sscanf(msg,"get %3s",note) != 1){
				return -1;
			}
			halftone = string_to_semitone(note);
			synthesis_get_frequency(halftone, &amplitude);
			*answ = calloc(8,sizeof(char));
			int ret = snprintf(*answ,8,"%3s %2x\n",note,amplitude);
			if( ret < 0 ){
				return -1;
			}
			*len_answ = ret+1;
			break;*/
		default:
			return -1;
	}
	return 0;
}

int string_to_semitone(char *note){
	int halftone;
	switch(*note){
		case 'c':
			halftone=0;
			break;
		case 'd':
			halftone=2;
			break;
		case 'e':
			halftone=4;
			break;
		case 'f':
			halftone=5;
			break;
		case 'g':
			halftone=7;
			break;
		case 'a':
			halftone=9;
			break;
		case 'b':
			halftone=11;
			break;
		default:
			return -1;
	}
	note++;
	switch(*note){
		case '#':
			halftone++;
			note++;
			break;
		case 'b':
			halftone--;
			note++;
			break;
	}
	int reg = *note-'0';
	if( reg < 0 ){
		return -1;
	}
	halftone += 12*reg;
	return halftone <= MAXIMUM_HALFTONE ? halftone : -1;
}
