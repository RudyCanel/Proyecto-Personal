#define output(pin)       pinMode(pin,OUTPUT)
#define input(pin)        pinMode(pin,INPUT)
#define pullup(pin)       pinMode(pin,INPUT_PULLUP)
#define ON(pin)           digitalWrite(pin,HIGH)
#define OFF(pin)          digitalWrite(pin,LOW)
#define leer(pin)         analogRead(pin)
#define r1                2
#define r2                3
#define r3                4
#define r4                5
#define c1                A0
#define c2                A1
#define c3                A2
#define c4                A3
#define lednum            12
#define pixelpin          7
#define bright            20
#include <OneWire.h>
#include <Adafruit_NeoPixel.h>
#include <DS3231.h>
#include <Ticker.h>
#include <LedControl.h>
//#include <SparkFun_ADXL345.h>
#include <LiquidCrystal_I2C.h>
DS3231 RTC;
LiquidCrystal_I2C pantalla(0x27, 16, 2);
LedControl matrix = LedControl(12, 11, 10, 1);
Adafruit_NeoPixel   neopixel(lednum, pixelpin, NEO_GRB + NEO_KHZ800);

const static byte rows[] = {r1, r2, r3, r4};
const static byte columns[] = {c1, c2, c3, c4};

enum ModoOperacion {      // Variables con modos de operacion de codigo
  Modo_Estatico,
  Modo_Ejecucion,
  Modo_Alerta,
  Modo_Configuracion,
  Modo_Intruso
};

enum ModoVisualizacion {  // Variables con modo de visualizacion
  Modo_Hora,
  Modo_UltimaVez,
  Modo_ImpresionLCD,
  Modo_Cargando,
};

struct Datos_Generales {     //Variables para el uso de datos
  byte MQ2;
  String dato_Bluetooth;
  byte sens_Obstaculos;
  byte hora;
  byte temperatura;
  byte movimiento;

};

struct Configuracion {        //Configuracion de los pines a usar
  struct Pines_Sensores {
    static const int pinMQ2 = A6; // Pin del MQ2
    static const int pin_Obstaculos = A7; //Pin del sensor de obstaculos
  };
};

static const PROGMEM byte numeros_decena[7][8] = {
  {
    B11111111,
    B10000001,
    B11111111,
    B00000000,
  }, {
    B00100000,
    B01000000,
    B11111111,
    B00000000
  }, {
    B10011111,
    B10010001,
    B11110001,
    B00000000

  }, {
    B10010001,
    B10010001,
    B11111111,
    B00000000

  }, {
    B11110000,
    B00010000,
    B11111111,
    B00000000

  }, {
    B11110001,
    B10010001,
    B10011111,
    B00000000

  }, {
    B11111111,
    B10010001,
    B10011111,
    B00000000

  }
};
static const PROGMEM byte numeros_unidades [10][8] =
{
  {
    B01111110,
    B10000001,
    B10000001,
    B01111110
  }, {
    B00100000,
    B01000000,
    B11111111,
    B00000000

  }, {
    B10001111,
    B10010001,
    B10010001,
    B11110001,

  }, {
    B10000001,
    B10010001,
    B10010001,
    B11111111

  }, {
    B11110000,
    B00010000,
    B00010000,
    B11111111

  }, {
    B11110001,
    B10010001,
    B10010001,
    B10011111,

  }, {
    B11111111,
    B10010001,
    B10010001,
    B10011111

  }, {
    B10000000,
    B10010000,
    B11111111,
    B00010000

  }, {
    B11111111,
    B10010001,
    B10010001,
    B11111111

  }, {
    B11110000,
    B10010000,
    B10010000,
    B11111111

  }
};


static const PROGMEM byte horas[8] =
{
  B11111111,
  B00010000,
  B00010000,
  B11111111,
  B00000000,
  B11111111,
  B10010000,
  B01101111

};
static const PROGMEM byte minutos[8] =
{
  B00000000,
  B00000000,
  B10101001,
  B11101101,
  B10101011,
  B10101001,
  B00000000,
  B00000000
};
class modulos {
  public:
    modulos() {
      input(Configuracion::Pines_Sensores::pinMQ2);
      input(Configuracion::Pines_Sensores::pin_Obstaculos);
    }
    int leer_obtaculos() {
      return leer(Configuracion::Pines_Sensores::pin_Obstaculos);
    }
    int leer_MQ2()
    {
      int lectura;
      int entradaMQ2 = leer(Configuracion::Pines_Sensores::pinMQ2);

      lectura = map(entradaMQ2, 0, 1024, 0, 100);
      return lectura;
    }
    int leer_temperatura()
    {
      return round(RTC.getTemperature());
    }
    int leer_hora()
    {
      bool h24, AM_PM;

      return RTC.getHour(h24, AM_PM);
    }
    int leer_minuto()
    {
      return RTC.getMinute();
    }
    int leer_segundo()
    {
      return RTC.getSecond();
    }
    int leer_dia()
    {
      return RTC.getDate();
    }
    int leer_mes()
    {
      bool siglo_error;
      return RTC.getMonth(siglo_error);
    }
    int leer_ano()
    {
      return RTC.getYear();
    }
    int leer_acelerometro()
    {
    }
};
modulos modulos;

class visualizadores {
  public:
    visualizadores() {};

    void  imprimir_matriz_hora()
    {
      byte hora = modulos.leer_hora();
      byte buff;

      for (int k = 0; k < 4; k++)
      {
        buff = hora / 10;
        matrix.setColumn(0, k, pgm_read_byte(&numeros_decena[buff][k]));
        buff = hora % 10;
        matrix.setColumn(0, k + 4, pgm_read_byte(&numeros_unidades[buff][k]));
      }
    }

    void imprimir_matriz_minutos()
    {
      byte minutos = modulos.leer_minuto();
      byte buff;
      for (int k = 0; k < 4; k++)
      {
        buff = minutos / 10;
        matrix.setColumn(0, k, pgm_read_byte(&numeros_decena[buff][k]));
        buff = minutos % 10;
        matrix.setColumn(0, k + 4, pgm_read_byte(&numeros_unidades[buff][k]));
      }
    }

    void imprimir_pantalla()
    {

    }
};
visualizadores visualizadores;

void setup() {
  Wire.begin();
  Serial.begin(9600);
  neopixel.begin();
  neopixel.show();
  neopixel.setBrightness(bright);
  for (byte i = 0; i < 4; i++)
  {
    output(rows[i]);
    OFF(rows[i]);
    pullup(columns[i]);
  }
  pantalla.init();
  pantalla.backlight();
  pantalla.setCursor(0, 0);
  pantalla.print("Hola piruelin");
  Serial.print(modulos.leer_temperatura());
  Serial.println("°C");
  Serial.print(modulos.leer_hora());
  Serial.println("Horas");
  matrix.shutdown(0, false);
  matrix.setIntensity(0, 5);

}

void loop() {
  visualizadores.imprimir_matriz_hora();
  matrix.shutdown(0,true);
  delay(1000);
  matrix.shutdown(0,false);
  delay(1000);
  //keypad();
  //neopixel1();
}



void keypad() {
  for (byte k = 0; k < 4; k++)
  {
    output(rows[k]);
    pullup(columns[k]);
    OFF(rows[k]);
  }

  static const char key[4][4] PROGMEM = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
  };

  for (byte i = 0; i < 4; i++)
  {
    bool local;

    if (digitalRead(columns[i]) == LOW)
    {
      for (byte k = 0; k < 4; k++)
      {
        output(columns[k]);
        pullup(rows[k]);
        OFF(columns[k]);
      }
      delay(1);

      for (byte j = 0; j < 4; j++)
      {
        if (digitalRead(rows[j]) == LOW)
        {
          Serial.println(char(pgm_read_byte(&key[j][i])));
          local = true;
          break;
        }
      }
      if (local)
      {
        break;
      }
    }
  }
}

void neopixel1 () {
  static byte color[] = {0, 0, 0};

  for (int i = 0; i < 4 ; i++)
  {
    static byte color = 0;
    static byte R = 0 , G = 0, B = 0;

    for (int i = 0; i < lednum ; i++)
    {
      neopixel.setPixelColor(i, neopixel.Color(R, G, B));
      neopixel.show();
      delay(50);
    }

    color++;

  }
}
