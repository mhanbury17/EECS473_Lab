#include <string.h>

/* LCD Defines */
#define CLEAR_DISPLAY           0x01
#define RETURN_HOME             0x02
#define ENTRY_MODE_SET          0x04
#define DISPLAY_ON              0x0C
#define DISPLAY_ON_CURSOR       0x0F
#define DISPLAY_OFF             0x08
#define SET_CGRAM_ADDR          0x40
#define SET_DDRAM_ADDR          0x80
#define FUNCTION_SET            0x3F
#define SHIFT_RIGHT             0x14
#define SHIFT_LEFT              0x10

#define DB7         2
#define DB6         6
#define DB5         4
#define DB4         10
#define EN          11
#define REG_SEL     12

/* Motor Control Defines */
#define FORWARD     'F'
#define LEFT        'L'
#define BACKWARD    'B'
#define RIGHT       'R'
#define STOP        'S'
#define END         '/'
#define INCREASE    'J'
#define DECREASE    'K'

#define MESSAGE_MAX_SIZE 5

#define LEFT_MOTOR  true
#define RIGHT_MOTOR false

/* Motor Control Statics */
char mode = 'R';

const char SoP = 'C';
const char EoP = 'E';

const char nullTerminator = '\0';
unsigned char inByte;
char message[MESSAGE_MAX_SIZE];
char command;

const int IDLE  = 0;
volatile int SPEED = 150;

const int EN1 = 3; // pwm pin, control left motor
const int EN2 = 5; // pwm pin, control right motor
const int A_1 = 7; // control Y1 (left motor positive)
const int A_2 = 8; // control Y2 (left motor negative)
const int A_3 = 9; // control Y3 (right motor positive)
const int A_4 = 13; // control Y4 (right motor negative)

/* LCD Structure */
class HD44780_LCD
{
public:
    void init(void);
    void set_cursor_idx(uint8_t target);
    void write_special_char(uint8_t char_code);
    void write_char(char c);
    void write_string(String s);
    //void write_str_centered();
    void write_string(String s, uint8_t target);
    void clear_screen();
    void backspace(uint8_t num_chars = 1);
    void set_cursor(uint8_t x, uint8_t y);
    
private:
    uint8_t cursor_idx = 0;

    void write_instruction(uint8_t data);
    void write_data(uint8_t data);
};

static HD44780_LCD disp;



/* ----------- Robot Implementations ------------ */
void controlRobot()
{
    // 1. get legal message
    if (!parsePacket())
        return;

    /// 2. action, for now we only use option 1
    if (message[0] == '1') {
        // Move command
        command = message[1];
        if (command == '/')
        {
            mode = 'W';
            delay(1);
        }
        else
            moveRobot(command);
    }
    else if (message[0] == '2') {
        // Display Read
        // ...
    }
    else if (message[0] == '3') {
        // Distance Read
        // ...
    }
    else if (message[0] == '4') {
        // Display Write
        Serial.println(message);
        return;
    }
    else {
        Serial.println("ERROR: unknown message");
        return;
    }
}

bool parsePacket()
{
    // step 1. get SoP
    while (Serial.available() < 1) {};
    inByte = Serial.read();
    //Serial.print((byte)inByte);
    if (inByte != SoP)
    {
        Serial.print("ERROR: Expected SOP, got: ");
        Serial.write((byte)inByte);
        Serial.print("\n");
        return false;
    }
    while (inByte != SoP)
      inByte = Serial.read();

    // step 2. get message length
    while (Serial.available() < 1) {};
    inByte = Serial.read();
        //Serial.print((byte)inByte);
    if (inByte == SoP)
    {
      while (Serial.available() < 1) {};
      inByte = Serial.read();
          //Serial.print(inByte);
    }
    if (inByte == EoP || inByte == SoP) {
        Serial.println("ERROR: SoP/EoP in length field");
        return false;
    }
    int message_size = inByte - '0';
    if (message_size > MESSAGE_MAX_SIZE || message_size < 0) {
        Serial.println("ERROR: Packet Length out of range");
        return false;
    }

    // step 3. get message
    for (int i = 0; i < message_size; i++)
    {
        while (Serial.available() < 1) {};
        inByte = Serial.read();
        if ((inByte == EoP || inByte == SoP))
        {
            Serial.println("ERROR: SoP/EoP in command field");
            return false;
        }
        message[i] = (char)inByte;
            //Serial.print(message[i]);
            
    }
    message[message_size] = nullTerminator;

    // step 4. get EoP
    while (Serial.available() < 1) {};
    inByte = Serial.read();
    //Serial.print((byte)inByte);
        default:
            Serial.println("ERROR: Unknown command in legal packet");
            break;
    }
}

void motorControl(bool ifLeftMotor, char command)
{
    int enable   = ifLeftMotor ? EN1 : EN2;
    int motorPos = ifLeftMotor ? A_1 : A_3;
    int motorNeg = ifLeftMotor ? A_2 : A_4;
    switch (command) {
        case FORWARD:
            analogWrite(enable, SPEED);
            digitalWrite(motorPos, HIGH);
            digitalWrite(motorNeg, LOW);
            break;
        case BACKWARD:
            analogWrite(enable, SPEED);
            digitalWrite(motorPos, LOW);
            digitalWrite(motorNeg, HIGH);
            break;
        case STOP:
            digitalWrite(motorPos, LOW);
            digitalWrite(motorNeg, LOW);
            break;
        default:
            break;
    }
}

/* ------------ LCD Implementations ------------- */
void controlLCD()
{
    // 1. get legal message
    if (!parsePacket())
        return;
    //Serial.println(message[0]);
    //Serial.println(message[1]);
    //Serial.println(" ");
    /// 2. action, for now we only use option 1
    if (message[0] == '1') {
        command = message[1];
        if (command == '/')
        {
            mode = 'R';
            
            delay(1);
        }
        else if (command == '~')
            disp.backspace(1);
        else
            disp.write_char(command);
    }
    else if (message[0] == '2') {
        // Display Read
        // ...
    }
    else if (message[0] == '3') {
        // Distance Read
        // ...
    }
    else if (message[0] == '4') {
        // Display Write
        Serial.println(message);
        return;
    }
    else {
        Serial.println("ERROR: unknown message");
        return;
    }
}

void set_input(void)
{
    pinMode(DB7, INPUT);
    pinMode(DB6, INPUT);
    pinMode(DB5, INPUT);
    pinMode(DB4, INPUT);
}

void set_output(void)
{
    pinMode(DB7, OUTPUT);
    pinMode(DB6, OUTPUT);
    pinMode(DB5, OUTPUT);
    pinMode(DB4, OUTPUT);
}

void HD44780_LCD::init(void)
{
    digitalWrite(REG_SEL, LOW);
    
    write_instruction(DISPLAY_OFF);
    write_instruction(CLEAR_DISPLAY);
    write_instruction(FUNCTION_SET);
    write_instruction(RETURN_HOME);
    write_instruction(DISPLAY_ON_CURSOR);
}

void HD44780_LCD::write_instruction(uint8_t data)
{
    // ready upper 4 bits of message
    digitalWrite(DB7, (data >> 7) & 0x1);
    digitalWrite(DB6, (data >> 6) & 0x1);
    digitalWrite(DB5, (data >> 5) & 0x1);
    digitalWrite(DB4, (data >> 4) & 0x1);

    // write 4 bits to display
    digitalWrite(EN, LOW);
    delayMicroseconds(1);
    digitalWrite(EN, HIGH);
    delayMicroseconds(1);
    digitalWrite(EN, LOW);
    delayMicroseconds(100);

    // ready lower 4 bits of message
    digitalWrite(DB7, (data >> 3)&0x1);
    digitalWrite(DB6, (data >> 2)&0x1);
    digitalWrite(DB5, (data >> 1)&0x1);
    digitalWrite(DB4, (data >> 0)&0x1);

    // write 4 bits to display
    digitalWrite(EN, LOW);
    delayMicroseconds(1);
    digitalWrite(EN, HIGH);
    delayMicroseconds(1);
    digitalWrite(EN, LOW);

    // allow transaction to finish
    delay(1);
}

void HD44780_LCD::set_cursor_idx(uint8_t target)
{
    uint8_t col = target % 16;

    // if the target is to the right of the cursor_idx
    while(cursor_idx++ < col)
        write_instruction(SHIFT_RIGHT);

    // if the target is to the left of the cursor_idx
    while(cursor_idx-- > col)
        write_instruction(SHIFT_LEFT);

//     adjust cursor_idx row
    if (target > 16)
        cursor_idx |= 0x40;

}

void HD44780_LCD::write_char(char c)
{
//    if (cursor_idx == 31)
//    {
//       set_cursor_idx(0);
//       for (int i = 0; i < 32; ++i)
//          write_char(' ');
//    }
  
    // get the character pattern
    write_instruction(SET_CGRAM_ADDR);
    digitalWrite(REG_SEL, HIGH);
    write_instruction((int)c);
    digitalWrite(REG_SEL, LOW);

    // set target location
    uint16_t target = 0;
    target |= 0x80;
    target |= cursor_idx %16;

    ++cursor_idx;

    if (cursor_idx > 32)
    {
        cursor_idx = 0;
        write_instruction(CLEAR_DISPLAY);
        write_instruction(RETURN_HOME);
    }
    if (cursor_idx > 16)
    {
        target |= 0x40;
    }
    write_instruction(target);yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy
    
    digitalWrite(REG_SEL, HIGH);
    write_instruction((int)c);
    digitalWrite(REG_SEL, LOW);

    // move cursor_idx cursor_idxition
    //write_instruction(SHIFT_RIGHT);
    Serial.println(cursor_idx);

}

void HD44780_LCD::write_string(String s)
{
    for (char c : s)
        write_char(c);
}

void HD44780_LCD::write_string(String s, uint8_t target)
{
    set_cursor_idx(target);
    write_string(s);
}

void HD44780_LCD::backspace(uint8_t num_chars = 1)
{
    while(num_chars > 0)
    {
        if (cursor_idx == 16)
        {
          write_instruction(SHIFT_LEFT);
          write_char(' ');
          write_instruction(SHIFT_LEFT);
          set_cursor_idx(16);

        }
        else
        {
          --cursor_idx;
          write_instruction(SHIFT_LEFT);
          write_char(' ');
          --cursor_idx;
          write_instruction(SHIFT_LEFT);
        }
        --num_chars;
    }
}

void HD44780_LCD::set_cursor(uint8_t x, uint8_t y)
{
    if (x > 15 || y > 1)
        return;
  
    cursor_idx = y*16 + x;
}

/* -------------------- Main -------------------- */


void setup(void)
{
    // robot setup
    Serial.begin(9600);
    Serial.println("START");
    pinMode(EN1, OUTPUT);
    pinMode(EN2, OUTPUT);
    pinMode(A_1, OUTPUT);
    pinMode(A_2, OUTPUT);
    pinMode(A_3, OUTPUT);
    pinMode(A_4, OUTPUT);

    // LCD setup
    pinMode(REG_SEL, OUTPUT);
    pinMode(EN, OUTPUT);
    set_output();

    // initialize
    disp.init();
    moveRobot(STOP);
    mode = 'W';
}

void loop(void)
{
    switch(mode)
    {
        case 'R': // control Robot
            controlRobot();
            break;
        case 'W': // write to LCD
            controlLCD();
            break;
        default:
            break;
    }
}
