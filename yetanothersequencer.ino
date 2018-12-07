#include <Math.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Encoder.h>

#include "custom_filetypes.h"
#include "noteQueue.h"

const int RED = 1;
const int BLUE = 2;
const int GREEN = 3;
const int WHITE = 4;


LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
/*
uint8_t symbols[8][8] = {
  {
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111
  }, {
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000
  },
  {
    B00000,
    B00000,
    B01110,
    B01110,
    B01110,
    B01110,
    B00000,
    B00000
  },
  {
    B00000,
    B00000,
    B00000,
    B00100,
    B00100,
    B00000,
    B00000,
    B00000
  },
  {
    B00000,
    B00000,
    B00000,
    B01110,
    B01110,
    B01110,
    B00000,
    B00000
  },
  {
    B00000,
    B01000,
    B01100,
    B01110,
    B01110,
    B01100,
    B01000,
    B00000
  },
  {
    B00000,
    B00000,
    B01110,
    B11111,
    B11111,
    B01110,
    B00000,
    B00000
  }
};
*/
bool YES = true;
bool NO = false;

bool seq_state = false;
bool seq_state_control = false;
int seq_position = 0;

bool recording = false;
bool recording_control = false;



note default_note;

int seq_steps = 32;
int new_seq_steps = seq_steps;
const byte polyphony = 1;
const int seq_steps_tot = 256;

note sequencer[seq_steps_tot];


note incoming_note;

//byte automation[3][64 * 24];
//int automation_position = 0;
//int automation_map[3] = {-1, -1, -1};

byte command_byte;
byte note_byte;
byte velocity_byte;

byte midi_channel = 3;

byte input_midi_channel;
byte midi_in;

const byte midi_start = 0xfa;
const byte midi_stop = 0xfc;
const byte midi_clock = 0xf8;
const byte midi_continue = 0xfb;
const byte midi_active_sense = 0xfe;
const byte midi_reset = 0xff;
const byte midi_song_pos_pointer = 0xf2;

//int note = 0;

unsigned long prev_millis = 0;
unsigned long prev_millis_loop = 0;
unsigned long prev_millis_seq = 0;
int midi_clock_count = 0;
int seq_clock_count = 0;
double prev_bpm = 120;
double bpm = 120;
int bpm_chg_count = 0;

byte data;
int print_row = 0;

byte midi_in_channel;
byte midi_in_event;
byte midi_in_pitch;
byte midi_in_velocity;

byte midi_in_note_buffer[2];

int note_count = 0;

bool note_sent = false;

int current_beat = 1;
int current_beat_control = 0;
int last_recorded_beat = 0;
int last_played_note = 0;


const byte keypad_dim = 4;
const byte keypad_rows[keypad_dim] = {11, 10, 9, 8};
const byte keypad_columns[keypad_dim] = {7, 6, 5, 4};

String notes[] = {"A", "A#", "B", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#"};

char keys[keypad_dim][keypad_dim] = {
  {'d', 'c', 'b', 'a'},
  {'h', 'g', 'f', 'e'},
  {'l', 'k', 'j', 'i'},
  {'+', '-', 'p', 'r'}
};

Keypad kpd = Keypad (makeKeymap(keys), keypad_rows, keypad_columns, keypad_dim, keypad_dim);
char key;

bool synced = false;

const int led_red = 44;
const int led_green = 12;
const int led_blue = 45;

int led_red_val = 0;
int led_green_val = 0;
int led_blue_val = 0;

const int rot_click = 22;
const int rot_plus = 23;
const int rot_minus = 24;

Encoder mighty_knob(rot_plus, rot_minus);
int mighty_knob_status = 0;
int mighty_knob_status_helper = 0;

note note_to_display;

bool editing = false;
bool editing_certain_step = false;
bool rot_clicked = false;
unsigned long rot_hold_millis = 0;
int rot_long_press_limit = 300;
bool rot_long_pressed = false;

int step_to_edit = 1;

int last_tick = 0;

int blinking_factor = 250;
unsigned long blinking_last = 0;
bool blinking_blinked = false;

int bpm_change_count = 0;

note * note_off_helper;
int note_off_helper_i = 0;

noteQueue recording_queue;
noteQueue playing_queue;

note * next_note_ptr = NULL;

note * last_seq_note_ptr = NULL;

void setup() {

  pinMode(44, OUTPUT);
  pinMode(43, OUTPUT);
  pinMode(12, OUTPUT);

  pinMode(rot_click, INPUT);

  led_color(RED);

  for (int i = 0; i < keypad_dim; i++) {
    pinMode(keypad_rows[i], INPUT);
    pinMode(keypad_columns[i], INPUT);
  }

  lcd.begin(16, 2);
  //lcd.autoscroll();
  //smart_print("_______AHOJ_______JSEM_SEKVENCER_", YES);

  Serial.begin(19200);

  Serial1.begin(31250);
  Serial2.begin(31250);

  /*
  for (int i = 0; i <= sizeof(symbols) / sizeof(*symbols); i++) {
    lcd.createChar(i, symbols[i]);
  }
  */


  lcd.home();

  incoming_note.pitch = 0;
  incoming_note.velocity = 0;
  incoming_note.length = 0;

  /*  WIPE AUTOMATION
  //  for(int i = 0; i < 3; i++)
  //    for(int j = 0; j < 64*24; j++)
  //      automation[i][j] = -1;

  */

  lcd.clear();

  //print_sequencer();
  //print_status_row();
  //print_seq_length();

  /* PRINT BEAT NUMBER */
  //lcd.setCursor(15, 1);
  //lcd.print(1);

  wipe_seq();

  last_seq_note_ptr = sequencer;

  default_note.pitch = 69;
  default_note.velocity = 69;
  default_note.is_playing = false;
  default_note.length = 150;
  default_note.millis_started = millis();




  //delay(500);
}

void loop() {

  if (Serial1.available() > 0) {
    Serial1_interrupt();
  }

  if (Serial2.available() > 0) {
    Serial2_interrupt();
  }


  /*
     CHECK FOR NOTES THAT SHOULD BE SWITCHED OFF
  */
  
  for (int i = 0; i < playing_queue.getQLen(); i++) {
    if (playing_queue.getNote(i)->length >= millis() - sequencer[i].millis_started) {
      //print_step(&sequencer[i]);
      note_off(playing_queue.removeNote(&sequencer[i]), true);
    }
  }

  /*
     CHECK FOR KEYBOARD INPUT
  */

  key = kpd.getKey();

  /*
     KEYBOARD INPUT
  */

  if (key) {
    switch (key) {
      case 'r':
        if (recording) {
          recording = false;
          Serial.println("UNREC");

        } else {
          recording = true;
          Serial.println("REC NOW");
        }

        editing = false;
        //print_seq_state();
        break;

      case 'p':
        wipe_seq();
        
        break;

      case '-':
        if (new_seq_steps >= 16) {
          new_seq_steps /= 2;
          //print_seq_length();
        }
        break;

      case '+':
        if (new_seq_steps <= 128) {
          new_seq_steps *= 2;
          //print_seq_length();
        }
        break;

      case 'a':
        print_sequencer();
      break;

      case 'd':
        panic();
        break;

      case 'e':
        //half_sequencer();
        break;

      case 'f':
        //duplicate_sequencer();
        break;
      case 'j':
        if (editing) {
          wipe_step(step_to_edit);
        }
        break;
    }
  }

  /*
     LED STATE
  */
  if (recording) {
    led_color(RED);
  } else if (seq_state) {
    led_color(GREEN);
  } else if (editing) {
    led_color(BLUE);
  } else {
    led_color(WHITE);
  }

  /*
  //   THE ALMIGHTY KNOB WORK GOES HERE
  
  mighty_knob_status_helper = mighty_knob.read() / 4;

  if (editing) {

    if (mighty_knob_status != mighty_knob_status_helper) {

      if (!editing_certain_step) {

        if (mighty_knob_status > mighty_knob_status_helper) {
          if (step_to_edit == seq_steps)
            step_to_edit = 1;
          else
            step_to_edit += 1;
        } else {
          if (step_to_edit == 1)
            step_to_edit = seq_steps;
          else
            step_to_edit -= 1;
        }

        // print_step(step_to_edit);
      } else {

        if (mighty_knob_status > mighty_knob_status_helper) {
          if (is_empty_note(sequencer[step_to_edit - 1][0])) {
            sequencer[step_to_edit - 1][0] = default_note;
          } else {
            sequencer[step_to_edit - 1][0].pitch += 1;
            default_note = sequencer[step_to_edit - 1][0];
          }
        } else {
          if (is_empty_note(sequencer[step_to_edit - 1][0])) {
            sequencer[step_to_edit - 1][0] = default_note;
          } else {
            sequencer[step_to_edit - 1][0].pitch -= 1;
            default_note = sequencer[step_to_edit - 1][0];
          }
        }

        sequencer[step_to_edit - 1][0].is_recorded = true;
        //print_step(step_to_edit);
      }
    }


    mighty_knob_status = mighty_knob_status_helper;


    
    //   MAKE STEP NR BLINK
    
    if (editing_certain_step) {
      if (millis() - blinking_last >= blinking_factor && blinking_blinked) {

        //lcd.setCursor(13, 1);
        //lcd.print("   ");

        blinking_last = millis();
        blinking_blinked = false;

      } else if (millis() - blinking_last >= blinking_factor && !blinking_blinked) {
        //display_step_nr(step_to_edit);

        blinking_last = millis();
        blinking_blinked = true;
      }
    }
  }
  */

  /*
     ALMIGHTY KNOB CLICK
  */

  if (digitalRead(rot_click) == LOW && !rot_clicked && !rot_long_pressed) {

    rot_clicked = true;
    rot_hold_millis = millis();

  } else if (digitalRead(rot_click) == HIGH && rot_clicked && !rot_long_pressed) {

    /*
       ROTARY WAS CLICKED
    */

    if (editing)
      editing_certain_step = !editing_certain_step;

    rot_clicked = false;

  } else if (digitalRead(rot_click) == HIGH && rot_clicked && rot_long_pressed) {

    /*
       ROTARY WAS LONG-PRESSED BUT CHECK NEXT IF FOR ACTION
    */
    rot_clicked = false;
    rot_long_pressed = false;

  }

  /*
     ALMIGHTY KNOB HOLD (THE NEXT IF)
  */

  if (millis() - rot_hold_millis > rot_long_press_limit && rot_clicked && !rot_long_pressed) {
    /*
       THIS IS THE NEXT IF

    */
    if (!editing) {
      editing = true;
      recording = false;
      //print_seq_state();
    } else {
      editing_certain_step = false;
      editing = false;


      //print_seq_state();
    }

    rot_long_pressed = true;
  }
}

void Clock() {

  if (seq_clock_count == 0 && seq_state) {
    seq_tick();
  }

  if (seq_clock_count < 2 && seq_state) {
    seq_clock_count += 1;
  } else if (seq_clock_count == 2) {
    seq_clock_count = 0;
  }

  //  if(seq_state){
  //    play_automation();
  //  }

  if (midi_clock_count == 24) {
    midi_clock_count = 0;

    bpm = round((double)60 / ((double)(millis() - prev_millis) / 1000));

    prev_millis = millis();
  }

  if (bpm - prev_bpm) {
    bpm_chg_count += 1;

    if(bpm_chg_count == 4) {
      prev_bpm = bpm;
      print_bpm();
      bpm_chg_count = 0;
    }
  } else {
    bpm_chg_count = 0;
  }

  midi_clock_count += 1;
}

void seq_tick() {

  if (current_beat > seq_steps) {
    current_beat = 1;
    seq_steps = new_seq_steps;
  }


  if (seq_state) {

    play_step(current_beat - 1);

    //if (!editing)
      // print_step(current_beat);

  }

  current_beat += 1;

  last_tick = millis();
}


note * record_step(note * in, int step) {
  Serial.print("old biggest adress");

  Serial.println((unsigned int)last_seq_note_ptr, HEX);
  
  if(step > last_seq_note_ptr->beat){


    if(last_seq_note_ptr != sequencer){
      Serial.println("SAY WHAT");
      last_seq_note_ptr += 1;
    }

    *last_seq_note_ptr = *in;

    return last_seq_note_ptr;
  }

  for(int i=0; i < sizeof(sequencer)/sizeof(sequencer[0]); i++){


    Serial.print("seq[].beat");
    Serial.println(sequencer[i].beat);

     if (sequencer[i].beat >= step){
      Serial.print("yes");
      Serial.println(i);
      //memmove(sequencer + i + 2, sequencer + i + 1, (last_seq_note_ptr - (sequencer + i + 1)) * sizeof(sequencer[0]));
      memmove(sequencer + i + 1, sequencer + i, (last_seq_note_ptr - (sequencer + i - 1)) * sizeof(sequencer[0]));
      //memmove(sequencer + i + 1, sequencer + i, sizeof(sequencer) - sizeof(sequencer[0] * i));
      
      sequencer[i] = *in;

      Serial.println(pitch_to_note(sequencer[i].pitch));

      last_seq_note_ptr += 1;

      Serial.print("new biggest adress");
      Serial.println((unsigned int)last_seq_note_ptr, HEX);
      return &sequencer[i];
    } 
  }

  Serial.println("NOTHING");


  //sequencer[last_recorded_beat] = *in;

  //Serial.print("Last recorded: ");
  //Serial.println(last_recorded_beat);

  //last_recorded_beat +=1;
  //return &sequencer[last_recorded_beat-1];
}

void play_step(int step) {
  if(step == 0){
    next_note_ptr = sequencer;
  }

  while(next_note_ptr->beat == step && next_note_ptr->is_recorded) {
    note_on(playing_queue.addNote(next_note_ptr), true);

    next_note_ptr += 1;

    if(next_note_ptr >= sequencer+sizeof(note)*seq_steps){
      next_note_ptr = 0;
      Serial.println("REPEAT");

    }
  }
}

void print_bpm() {
  /* if (bpm >= 100)
    lcd.setCursor(1, 1);

  else if (bpm < 10) {
    lcd.setCursor(1, 1);
    lcd.print("  ");
    lcd.setCursor(3, 1);

  } else {
    lcd.setCursor(1, 1);
    lcd.print(" ");
    lcd.setCursor(2, 1);

  }
  */
  lcd.setCursor(0, 1);

  lcd.print((int)bpm);
}

bool is_empty_note(note in) {
  if (in.pitch == 0)
    return true;
  else return false;
}

void set_note_empty(note *in) {
  in->pitch = 0;
  in->velocity = 0;
  in->length = 0;
  in->millis_started = 0;
  in->beat = -1;
}

void Serial1_interrupt() {

  //do {
  data = Serial1.read();

  Serial2.write(data);

  //} while ( (data >> 7) & 0 );

  //Serial.println(data);
  switch (data) {
    case midi_clock:
      Clock();
      break;

    case midi_stop:
      if (seq_state) {
        Serial.println("STOP");
        seq_state = false;
        //print_seq_state();

        //lcd.setCursor(14, 1);
        //lcd.print(" 1");
        synced = false;

        current_beat = 1;

        //automation_position = 0;


        //print_seq_state();

      }
      break;

    case midi_start:
      Serial.println("START");
      seq_state = true;
      synced = true;


      seq_clock_count = 0;
      current_beat = 1;
      bpm_chg_count = 0;

      //print_seq_state();
      break;

    case midi_song_pos_pointer:
      Serial.print("SPP ");
      Serial.print(Serial1.read());
      Serial.print(" ");
      Serial.println(Serial1.read());
      break;

    case midi_continue:
      Serial.println("CONTINUE");
      break;

    case midi_active_sense:
      break;

    default:
      /*
        if (data > B10000000 && data < B10011111){
          midi_in_channel = (data & 0xF) + 1;
          midi_in_event = (data >> 4) & 0xF;
        }

        Serial.println("                   OTHER: ");
        Serial.print("                 CHANNEL: ");
        Serial.println(midi_in_channel);
        Serial.print("                 COMMAND: ");
        Serial.println(midi_in_event);
        Serial.print("                   PITCH: ");
        Serial.println(Serial1.read());
        Serial.print("                    VELO: ");
        Serial.println(Serial1.read());
      */
      break;

  }
}

void Serial2_interrupt() {

  data = Serial2.read();

  midi_in_channel = (data & 0xF) + 1;
  midi_in_event = (data >> 4) & 0xF;

  //Serial.println(data);


  if (midi_in_event == 9) {

    /*
       NOTE ON
    */

    Serial2.readBytes(midi_in_note_buffer, 2);
    incoming_note.pitch = midi_in_note_buffer[0];
    incoming_note.velocity = midi_in_note_buffer[1];
    incoming_note.millis_started = millis();
    incoming_note.is_playing = false;
    incoming_note.is_recorded = false;
    incoming_note.beat = current_beat - 1;


    note_on(&incoming_note, true);

    if (editing_certain_step) {
      record_step(&incoming_note, step_to_edit - 1);
      

    } else if (recording) {
      recording_queue.addNote( record_step(&incoming_note, current_beat - 1) );
    }

  } else if (midi_in_event == 8) {

    /*
       NOTE OFF
    */

    Serial2.readBytes(midi_in_note_buffer, 2);
    incoming_note.pitch = midi_in_note_buffer[0];
    incoming_note.velocity = midi_in_note_buffer[1];
    incoming_note.millis_started = millis();
    incoming_note.is_playing = false;
    incoming_note.is_recorded = false;

    note_off(&incoming_note, true);
    
    if (recording) {

      recording_queue.removeNote(&incoming_note);

    } 

  }  else if (midi_in_event == B1110) {
    /*
       PITCHWHEEL
    */
    Serial2.write(data);

    while (Serial2.available() == 0) {
      true;
    }
    Serial2.write(Serial2.read());
    while (Serial2.available() == 0) {
      true;
    }
    Serial2.write(Serial2.read());

  } else if (midi_in_event == B1011) {
    /*
       CONTROL CHANGE
    */

    /*Serial.print("CC: ");

      while (Serial2.available() == 0) {
      true;
      }
      Serial.print(Serial2.read());
      Serial.print(" VAL: ");
      while (Serial2.available() == 0) {
      true;
      }
      Serial.println(Serial2.read());
    */

    Serial2.write(data);

    while (Serial2.available() == 0) {
      true;
    }
    Serial2.write(Serial2.read());
    while (Serial2.available() == 0) {
      true;
    }
    Serial2.write(Serial2.read());


  }
}

void note_on(note * in, bool verbose) {
  if (verbose){
    Serial.print("ON [");
    Serial.print(in->beat);
    Serial.print("]  ");

    Serial.println(pitch_to_note(in->pitch));
  } else
    Serial.println("ON");

  in->millis_started = millis();

  in->is_playing = true;



  // print_step(in);

  send_note(midi_channel, 9, in->pitch, in->velocity);
}

void note_off(note * in, bool verbose) {
  if (verbose) {

    Serial.print("OFF ");

    Serial.print(pitch_to_note(in->pitch));
    Serial.print(" ");
    Serial.println(in->velocity);
  } else
    Serial.println("OFF");
    
  if(!in->is_recorded){
    in->length = millis() - in->millis_started;
  }
    
  in->is_playing = false;
  in->is_recorded = true;

  //if (verbose)
  //  print_step(in);

  send_note(midi_channel, 8, in->pitch, in->velocity);
}

void send_note(byte channel, byte onoff, byte pitch, byte velocity) {

  Serial2.write((onoff << 4) + (midi_channel - 1));
  Serial2.write(pitch);
  Serial2.write(velocity);
}

/*
void duplicate_sequencer() {
  if (seq_steps * 2 <= sizeof(sequencer) / sizeof(*sequencer)) {

    for (int i = 0; i < seq_steps; i++) {
      for (int j = 0; j < polyphony; j++){
        sequencer[i + seq_steps][j] = sequencer[i];
  
        sequencer[i + seq_steps][j].is_playing = false;
      }
    }

    Serial.println("seq duped");

    seq_steps *= 2;
    //print_seq_length();
  }
}
*/

void wipe_seq() {
  panic();
  
  for (int i = 0; i < seq_steps_tot; i++)
   wipe_step(i);

  recording_queue.clearQ();
  playing_queue.clearQ();
  last_seq_note_ptr = sequencer;
  Serial.println("Seq empty");
}

void panic() {
  Serial2.write((B1011 << 4) | midi_channel - 1);
  Serial2.write(120);
  Serial2.write(0);
}

void display_note() {
  //lcd.setCursor(9, 1);

  //lcd.print("   ");
}

note return_note(int p, int v, int l) {
  note to_return;
  to_return.pitch = p;
  to_return.velocity = v;
  to_return.length = l;

  return to_return;
}

int return_len_in_ms(int note_len) {
  return (bpm / 60) * (16 / note_len);
}

/*
int empty_pos_in_step(int step) {
  if (sequencer[step].pitch == 0){
    
    return i;
  }

  return -1;
}
*/

void wipe_step(int step) {
  set_note_empty(&sequencer[step]);
}
  
String pitch_to_note(int pitch) {
  if(pitch < 1)
    return String("THIS AIN'T NOTE");

  int octave;
  String note;

  pitch -= 21;

  octave = (pitch / 12);

  if (pitch % 12 > 2) {
    octave += 1;
  }

  return String(notes[pitch % 12]) + String(octave);

}

void print_sequencer(){
  Serial.println(current_beat);

  for (int j = 0; j < seq_steps; j++){
    if(sequencer[j].pitch > 0){
      Serial.print(sequencer[j].beat);
      Serial.print(",");
      Serial.print(pitch_to_note(sequencer[j].pitch));
      Serial.print(",");
      Serial.print(sequencer[j].length);
      Serial.print(sequencer[j].is_recorded ? "R" : "_");
      Serial.print(sequencer[j].is_playing ? "P" : "_");
    } else Serial.print("     ");


    Serial.print("|"); 

  }
  Serial.println(" ");

  Serial.print("CB: ");
  Serial.print(current_beat);
  Serial.print(" CBC: ");
  Serial.println(current_beat_control);

  Serial.print("Q(");
  Serial.print(recording_queue.getLastPos());
  Serial.print("): ");
  for(int i = 0; i < recording_queue.getQLen(); i++){
    Serial.print(pitch_to_note(recording_queue.getNote(i)->pitch));
    Serial.print("\t");
  }
  Serial.println();
}

void led_color(int color) {
  switch (color) {
    case RED:
      led_red_val = 255;
      led_green_val = 0;
      led_blue_val = 0;
      break;

    case GREEN:
      led_red_val = 0;
      led_green_val = 255;
      led_blue_val = 0;
      break;

    case BLUE:
      led_red_val = 0;
      led_green_val = 0;
      led_blue_val = 255;
      break;

    case WHITE:
      led_red_val = 255;
      led_green_val = 255;
      led_blue_val = 255;
      break;
  }

  led_color();
}

void led_color() {

  analogWrite(led_red, led_red_val);
  analogWrite(led_green, led_green_val);
  analogWrite(led_blue, led_blue_val);
}

/* void print_seq_state() {
  //lcd.setCursor(0, 1);

  if (editing)
    //lcd.print("E");
  else if (recording)
    //lcd.print(char(6));
  else if (seq_state)
    //lcd.print(char(5));
  else
    //lcd.print(char(2));

}
*/

/* void print_seq_length() {
  //lcd.setCursor(5, 1);
  //lcd.print("    ");

  if (seq_steps < 10) {
    //lcd.setCursor(7, 1);
  } else {
    //lcd.setCursor(6, 1);
  }
  if (seq_steps != new_seq_steps)
    //lcd.print(new_seq_steps);
  else
    //lcd.print(seq_steps);
}
*/

/* void print_step(int step) {

  //display_note(&sequencer[step - 1][1]);

  display_step_nr(step);
}
*/

/* void display_note (note * in) {
  //lcd.setCursor(9, 1);

  if (is_empty_note(*in)) {
    //lcd.print("   ");
  } else {
    //lcd.print( pitch_to_note( in->pitch ).length() < 3 ? pitch_to_note( in->pitch ) + " " : pitch_to_note( in->pitch ) );
  }
}

*/

/* void display_step_nr(int step) {
  String to_print = "";

  if (step == 1) {
    to_print = "  ";
  } else if (step < 10)
    to_print = "  ";
  else
    to_print = " ";

  //lcd.setCursor(13, 1);

  to_print += String(step);

  //lcd.print(to_print);
}

*/

/* void smart_print(String text, bool wipe) {
  if (wipe)
    lcd.clear();

  print_row = 0;

  for (int i = 0; i <= text.length(); i++) {
    //lcd.print(text[i]);

    //lcd.setCursor((i > 15) ? i - 16 : i, print_row);

    if (i == 15)
      print_row = 1;
  }

  //lcd.setCursor(0, 0);
}
*/

/* void print_sequencer() {
  bool prvni = true;

  //lcd.setCursor(0, 0);

  for (int i = 0; i < 64; i++) {
    if ((i - 1) % 4 == 0 || i == 0)
      prvni = true;
    else prvni = false;


    if (is_empty_note(sequencer[i][1]) && !prvni)
      //lcd.print(char(3));

    else if (prvni)
      //lcd.print(char(4));
    else
      //lcd.print(char(2));


    //lcd.setCursor(i, 0);
  }

}
*/

/* void print_status_row() {

  // PRINT SEQUENCER STATE: PLAY/STOP/REC 
  if (seq_state) {
    //lcd.setCursor(0, 1);
    //lcd.print(char(5));
  }
  else {
    //lcd.setCursor(0, 1);
    //lcd.print(char(2));
  }

  // PRINT BPM 
  if (bpm >= 100)
    //lcd.setCursor(2, 1);
  else
    //lcd.setCursor(3, 1);


  //lcd.print(bpm);

}
*/


/* void play_automation() {
  for(int i = 0; i < 3; i++) {
    if(automation_map[i] >= 0){
      Serial.print("AUTO! ");
      Serial.print(automation_map[i]);
      Serial.print(" ");
      Serial.println(i);
      Serial2.write( (B1101 << 4) | midi_channel);
      Serial2.write(automation_map[i]);
      Serial2.write(automation[i][automation_position]);
    }
  }
}
*/

/* int return_empty_automation_lane() {
  for(int i = 0; i < 4; i++) {
    if(automation_map[i] == -1)
      return i;
  }

  return -1;
}
*/

/* void print_step(note * to_print) {
  Serial.print("ptc: ");
  Serial.print(pitch_to_note(to_print->pitch));
  Serial.print("\tvel: ");
  Serial.print(to_print->velocity);
  Serial.print("\tlen: ");
  Serial.print(to_print->length);
  Serial.print("\tply: ");
  Serial.print(to_print->is_playing);
  Serial.print("\tstd: ");
  Serial.print(to_print->millis_started);
  Serial.print("\trcd: ");
  Serial.println(to_print->is_recorded);
}
*/
