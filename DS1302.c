#define _pin_ena  PC7
#define _pin_clk  PC5
#define _pin_dat  PC6
#define REG_SECONDS           0x80
#define REG_MINUTES           0x82
#define REG_HOUR              0x84
#define REG_DATE              0x86
#define REG_MONTH             0x88
#define REG_DAY               0x8A
#define REG_YEAR              0x8C
#define REG_WP                0x8E
#define REG_BURST             0xBE
uint8_t second, minute, hour, day, mouth, dow, year;

void _nextBit()
{
  digitalWrite(_pin_clk, HIGH);
  delayMicroseconds(1);

  digitalWrite(_pin_clk, LOW);
  delayMicroseconds(1);
}
uint8_t _readByte()
{
  uint8_t byte = 0;

  for (uint8_t b = 0; b < 8; b++)
  {
    if (digitalRead(_pin_dat) == HIGH) byte |= 0x01 << b;
    _nextBit();
  }

  return byte;
}
void _writeByte(uint8_t value)
{
  for (uint8_t b = 0; b < 8; b++)
  {
    digitalWrite(_pin_dat, (value & 0x01) ? HIGH : LOW);
    _nextBit();
    value >>= 1;
  }
}
void _setDirection(int direction) {
  pinMode(_pin_dat, direction);
}
void _prepareRead(uint8_t address)
{
  _setDirection(OUTPUT);
  digitalWrite(_pin_ena, HIGH);
  uint8_t command = 0b10000001 | address;
  _writeByte(command);
  _setDirection(INPUT);
}
void _prepareWrite(uint8_t address)
{
  _setDirection(OUTPUT);
  digitalWrite(_pin_ena, HIGH);
  uint8_t command = 0b10000000 | address;
  _writeByte(command);
}
void _end()
{
  digitalWrite(_pin_ena, LOW);
}


uint8_t _dec2bcd(uint8_t dec)
{
  return ((dec / 10 * 16) + (dec % 10));
}
uint8_t _bcd2dec(uint8_t bcd)
{
  return ((bcd / 16 * 10) + (bcd % 16));
}
void get_time() {
  _prepareRead(REG_BURST);
  second = _bcd2dec(_readByte() & 0b01111111);
  minute = _bcd2dec(_readByte() & 0b01111111);
  hour   = _bcd2dec(_readByte() & 0b00111111);
  day    = _bcd2dec(_readByte() & 0b00111111);
  mouth  = _bcd2dec(_readByte() & 0b00011111);
  dow    = _bcd2dec(_readByte() & 0b00000111);
  year   = _bcd2dec(_readByte() & 0b01111111);
  _end();
  Serial_print_s("Now time is ");
  Serial_print_i(year);
  Serial_print_s("-");
  Serial_print_i(mouth);
  Serial_print_s("-");
  Serial_print_i(day);
  Serial_print_s("-");
  Serial_print_i(hour);
  Serial_print_s(":");
  Serial_print_i(minute);
  Serial_print_s(":");
  Serial_println_i(second);
}
void set_time(int sec, int minute, int hour, int day, int mouth, int dow, int year) {
  _prepareWrite(REG_WP);
  _writeByte(0b00000000);
  _end();
  _prepareWrite(REG_BURST);
  _writeByte(_dec2bcd(sec % 60 ));
  _writeByte(_dec2bcd(minute % 60 ));
  _writeByte(_dec2bcd(hour   % 24 ));
  _writeByte(_dec2bcd(day    % 32 ));
  _writeByte(_dec2bcd(mouth  % 13 ));
  _writeByte(_dec2bcd(dow    % 8  ));
  _writeByte(_dec2bcd(year   % 100));
  _writeByte(0b10000000);
  _end();
}
void setup() {
  pinMode(_pin_ena, OUTPUT);
  pinMode(_pin_clk, OUTPUT);
  pinMode(_pin_dat, INPUT);

  digitalWrite(_pin_ena, LOW);
  digitalWrite(_pin_clk, LOW);
  Serial_begin(9600);
  set_time(1,2,3,4,5,6,7);
}

void loop() {
  get_time();
  delay(2000);
}
