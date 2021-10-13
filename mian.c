/*
  Define value
*/
#define scl   PC3
#define sda   PA1
#define light PD3
#define buz   PD4
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
#define REG_RAM               0xFE
#define debug       1 /*0：关闭调试；1：开启调试*/
int a = 1 ;           /*亮度等级*/
/*
  时间变量；月份数据
*/
uint8_t second, minute, hour, day, mouth, dow, year;
uint8_t DOo[] = {/*2021/1*/1, 2, 3, 4, 5, 6, 7, 8, 9, 5, 1, 3,/*2022/01*/6, 1, 2, 5, 7, 3, 5, 1, 4, 6, 2, 4,/*2023/1*/7, 3, 3, 6, 1, 4, 6, 2, 5, 7, 3, 5};
/*
  DS1302驱动库
*/
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
  if (debug) {
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

void set_ram(uint8_t add, uint8_t data) {
  _prepareWrite(REG_WP);
  _writeByte(0b00000000);
  _end();
  _prepareWrite(add);
  _writeByte(data);
  _writeByte(0b10000000);
  _end();
}

uint8_t read_ram(uint8_t add) {
  _prepareRead(add);

  uint8_t readen = _readByte();
  _end();
  if (debug) {
    Serial_print_s("I has read a byte :");
    Serial_print_i(readen);
  }
}
/*
  TM1640驱动库
*/
void i2c_start() {
  digitalWrite(scl, 1);
  delayMicroseconds(2);
  digitalWrite(sda, 1);
  delayMicroseconds(2);
  digitalWrite(sda, 0);
  delayMicroseconds(2);
  digitalWrite(scl, 0);
  delayMicroseconds(2);
}

void i2c_stop() {
  digitalWrite(scl, 0);
  delayMicroseconds(2);
  digitalWrite(sda, 0);
  delayMicroseconds(2);
  digitalWrite(scl, 1);
  delayMicroseconds(2);
  digitalWrite(sda, 1);
  delayMicroseconds(2);
}

void i2c_write(uint8_t data) {
  for (int i = 0; i <= 7; i++) {
    if (data % 2) {
      digitalWrite(sda, 1);
      delayMicroseconds(2);
      digitalWrite(scl, 0);
      delayMicroseconds(2);
      digitalWrite(scl, 1);
      delayMicroseconds(2);
      digitalWrite(scl, 0);
      delayMicroseconds(2);
      digitalWrite(sda, 0);
    }
    else
    {
      delayMicroseconds(2);
      digitalWrite(sda, 0);
      delayMicroseconds(2);
      digitalWrite(scl, 0);
      delayMicroseconds(2);
      digitalWrite(scl, 1);
      delayMicroseconds(2);
      digitalWrite(scl, 0);
      delayMicroseconds(2);
      digitalWrite(sda, 0);
    }
    data /= 2;
  }
}

/*
  日期计算
*/
int nW(int Day) {
  int nD = DOo[(year - 21) * 12 + mouth - 1 ];
  int Nn = Day + nD - 1;
  int n_W = Nn / 7 + 1;
  int n_D = Nn % 7;
  //if (!n_D){
  //n_D = 7;}
  if (debug) {
    Serial_print_i(Day);
    Serial_print_s("是第");
    Serial_print_i(n_W);
    Serial_print_s("周的星期");
    Serial_println_i(n_D);
    return n_W * 10 + n_D;
  }
}

/*
  蜂鸣器
*/
void dot() {
  for (int n = 0; n < 80; n++) {
    digitalWrite(buz, 1);
    delay(1);
    digitalWrite(buz, 0);
    delay(1);
  }
}
/*
  显示时间
*/

dis_time() {
  i2c_start();
  i2c_write(0x40);
  i2c_stop();
  i2c_start();
  i2c_write(0xc0);
  i2c_write(0x01 << (nW(day) % 10));
  i2c_stop();
  i2c_start();
  i2c_write(0xc0 + (nW(day) / 10) );
  i2c_write(0x01 << (nW(day) % 10));
  i2c_stop();
  i2c_start();
  i2c_write(0x88 + a);
  i2c_stop();
}

/*
  加载动画
*/
void loading() {
  for (int i = 0; i < 12; i++) {
    for (int j = 0; j < 8; j++) {
      i2c_start();
      i2c_write(0x44);
      i2c_stop();
      i2c_start();
      i2c_write(0xc0 + i);
      i2c_write(0x01 << j);
      i2c_stop();
      i2c_start();
      i2c_write(0x8a);
      delay(5);
    }
  }
}
void setup() {
  pinMode(_pin_ena, OUTPUT);
  pinMode(_pin_clk, OUTPUT);
  pinMode(buz, OUTPUT);
  pinMode(_pin_dat, INPUT);
  digitalWrite(_pin_ena, LOW);
  digitalWrite(_pin_clk, LOW);
  pinMode(scl, OUTPUT);
  pinMode(sda, OUTPUT);
  Serial_begin(9600);
  //set_time(10,27,0,12,10,2,21);
  loading();
  Serial_println_s("I am here");
}

void loop() {
  get_time();
  //dot();

  if (minute == 0) {
    for (int h = 0; h < hour % 12; h++) {
      dot();
      delay(200);
    }
    loading();
  }
  //dot();
  dis_time();
  delay(29600);
  delay(30000);

}
