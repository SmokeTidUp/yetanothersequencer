#include "noteQueue.h"

#ifndef NULL

#define NULL nullptr

#endif


noteQueue::noteQueue(){
	clearQ();
}

void noteQueue::clearQ(){

	last_position = 0;

	for (int i = 0; i < Q_LEN; ++i)
	{
		master_q[i] = NULL;
	}
}

byte noteQueue::getQLen() {
	return Q_LEN;
}

note * noteQueue::addNote(note *in){

	if(in != NULL)
		if(last_position + 1 != Q_LEN) {
			master_q[last_position] = in;

			last_position += 1;

			return in;
		}

	return NULL;

}

note * noteQueue::removeNote(note *in){
	note * found = NULL;

	for(int i = 0; i < Q_LEN; i++) {
		if(found != NULL && master_q[i] != NULL){
			master_q[i-1] = master_q[i];

		} else if(found != NULL && master_q[i] == NULL){
			last_position = i - 1;
			master_q[last_position] = NULL;
			
			return found;

		} else if(in->pitch == master_q[i]->pitch) {
			found = master_q[i];
		}
	}

	return NULL;

}

byte noteQueue::getLastPos(){

	return last_position;
}

note * noteQueue::getNote(byte position){
	return master_q[position];
}
