#ifndef CUSTOM_FILETYPES_H

#include "custom_filetypes.h"

#endif



#ifndef NOTE_QUEUE_H

#define NOTE_QUEUE_H


#define Q_LEN 16

class noteQueue

{

public:

	note * addNote(note *in);

	note * removeNote(note *in);

	byte getLastPos();

	byte getQLen();

	noteQueue();

	void clearQ();

	void recordAndRemoveNote(note *in);

	note * getNote(byte index);

private:

	byte last_position;


	note * master_q[Q_LEN];
};

#endif