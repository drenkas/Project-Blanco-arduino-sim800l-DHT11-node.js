#include <DHT.h>		// бібліотека датчика
#include <Regexp.h> // бібліотека регулярних виразів (їх тут нема , ставив коли дебажив, але видалять не буду)
										//як то кажуть "Працює - не чіпай"
#include <SoftwareSerial.h>                     // Библиотека програмной реализации обмена по UART-протоколу
SoftwareSerial SIM800(9, 8);                    // RX, TX

String _response = "";                          // Переменная для хранения ответа модуля
#define DHTPIN A0 //пін підключення датчика
#define DHTTYPE DHT11 //DHT 11
DHT dht(DHTPIN, DHTTYPE); //ініціалізуємо датчик


String json;
String stLed;
String msg;
int		check = 0; //змінна, яка не дає модулю повторно відправляти смс

//LED RGB PINS
#define r_pin 4
#define g_pin 2
#define b_pin 3

//LIMIT VALUE
int t_top = 31;
int t_bot = 22;
int h_top = 60;
int h_bot = 20;

String phone = "+380хххххххххх"; //вводимо номер на який буде присилатись смс

//LED RGB VALUE INITIAL
int R = 255;
int G = 0;
int B = 0;

void setup() {
	Serial.begin(9600);
	SIM800.begin(9600);                           // Скорость обмена данными с модемом

	//LED RGB
	pinMode(r_pin, OUTPUT);
	pinMode(g_pin, OUTPUT);
	pinMode(b_pin, OUTPUT);
	delay(10);
	dht.begin();
	sendATCommand("AT", true);                    // Отправили AT для настройки скорости обмена данными

	// Команды настройки модема при каждом запуске
	//_response = sendATCommand("AT+CLIP=1", true);  // Включаем АОН
	//_response = sendATCommand("AT+DDET=1", true);  // Включаем DTMF
	_response = sendATCommand("AT+CMGF=1;&W", true); // Включаем текстовый режима SMS (Text mode) и сразу сохраняем значение (AT&W)!

	
}

String sendATCommand(String cmd, bool waiting) {
	String _resp = "";                            // Переменная для хранения результата
	SIM800.println(cmd);                          // Отправляем команду модулю
	if (waiting) {                                // Если необходимо дождаться ответа...
		_resp = waitResponse();                     // ... ждем, когда будет передан ответ
		// Если Echo Mode выключен (ATE0), то эти 3 строки можно закомментировать
		if (_resp.startsWith(cmd)) {                // Убираем из ответа дублирующуюся команду
			_resp = _resp.substring(_resp.indexOf("\r", cmd.length()) + 2);
		}
	}
	return _resp;                                 // Возвращаем результат. Пусто, если проблема
}

String waitResponse() {                         // Функция ожидания ответа и возврата полученного результата
	String _resp = "";                            // Переменная для хранения результата
	long _timeout = millis() + 10000;             // Переменная для отслеживания таймаута (10 секунд)
	while (!SIM800.available() && millis() < _timeout)  {}; // Ждем ответа 10 секунд, если пришел ответ или наступил таймаут, то...
	if (SIM800.available()) {                     // Если есть, что считывать...
		_resp = SIM800.readString();                // ... считываем и запоминаем
	}
	
	return _resp;                                 // ... возвращаем результат. Пусто, если проблема
}

void loop() {

	//GSM CHECK
	if (SIM800.available())   {                   // Если модем, что-то отправил...
		_response = waitResponse();                 // Получаем ответ от модема для анализа
		_response.trim();                           // Убираем лишние пробелы в начале и конце
		//....
		if (_response.startsWith("+CMGS:")) {       // Пришло сообщение об отправке SMS
			int index = _response.lastIndexOf("\r\n");// Находим последний перенос строки, перед статусом
			String result = _response.substring(index + 2, _response.length()); // Получаем статус
			result.trim();                            // Убираем пробельные символы в начале/конце
		}
	}
	if (Serial.available())  {                    // Ожидаем команды по Serial...
		SIM800.write(Serial.read());                // ...и отправляем полученную команду модему
	};

	//ALARM CHECK
	if ((dht.readTemperature() >= t_top && check == 0) || ( dht.readTemperature() <= t_bot && check == 0)
	|| ( dht.readHumidity() >= h_top && check == 0)|| ( dht.readHumidity() <= h_bot && check == 0)){

		analogWrite(r_pin, R);// запалюємо діод
		analogWrite(g_pin, G);
		analogWrite(b_pin, B);
		msg = "ALARM! "; //будуємо повідомлення
		msg += "\"Humidity\"";
		msg += ":";
		msg += dht.readHumidity();
		msg += ",";
		msg += "\"Temp\"";
		msg += ":";
		msg += dht.readTemperature();
		msg += "!";
		sendSMS(phone, msg); //функція відправки запита на модуль
		check = 1; //міняємо змінну
	}
	if(dht.readTemperature() < t_top  && dht.readTemperature() > t_bot 
	&& dht.readHumidity() < h_top && dht.readHumidity() > h_bot && check == 1){ //якщо показники у нас в нормі
		analogWrite(r_pin, 0);
		analogWrite(g_pin, 0);
		analogWrite(b_pin, 0);
		check = 0;
	}
	//JSON повідомлення , для відправки на сервер
	json = "{";
	json += "\"Humidity\"";
	json += ":";
	json += dht.readHumidity();
	json += ",";
	json += "\"Temp\"";
	json += ":";
	json += dht.readTemperature();
	json += ",";
	json += "\"LED\"";
	json += ":";
	json += "\""+stLed+"\"";
	json += ",";
	json += "\"LedRGB\"";
	json += ":";
	json += "{";
	json += "\"r\"";
	json += ":";
	json += R;
	json += ",";
	json += "\"g\"";
	json += ":";
	json += G;
	json += ",";
	json += "\"b\"";
	json += ":";
	json += B;
	json += "}";
	json += "}";

	Serial.println(json); //робимо вивід на серіал-порт
	delay(100);
}

void sendSMS(String phone, String message)
{
	sendATCommand("AT+CMGS=\"" + phone + "\"", true);             // Переходим в режим ввода текстового сообщения
	sendATCommand(message + "\r\n" + (String)((char)26), true);   // После текста отправляем перенос строки и Ctrl+Z
}
