#define BEEPER		0	//PIN_PA0
#define BEEP_DUR	5	//ms
#define MOSFET		11	//PIN_PB3
#define BUTTON		12	//PIN_PB4
#define R_POT		  A8	//PIN_PB5

enum states{
	off,
	on,
	turn_off,
	turn_on
};

uint32_t t0 = 0;
uint32_t t1 = 0;
//uint32_t t2 = 0;

uint16_t ADC_Res;
uint8_t Exp_Res;
uint8_t state = off;
bool flag = false;

void beep() {
	digitalWrite(BEEPER, HIGH);
	delay(BEEP_DUR);
	digitalWrite(BEEPER, LOW);
}

void PWM_setup() {
	// Частота шим ~ 2 кГц
    //f_PWM = (f_PLL / (OCR1C + 1)) / div
    //f_PWM = (64 МГц / (255 + 1)) / 128 = 1.953 кГц
	TCCR1B = 0b1000;        // Предделитель = 128 0b0111
    OCR1A = OCR1B = 0x00;   // Сброс заполнения ШИМ
    OCR1C = 0xFF;           // Разрешение ШИМ 8 бит
}

void BTN_Handler() {
    bool btnState = digitalRead(BUTTON);
    if (btnState && !flag) {  // обработчик нажатия, включаем/выключаем ленту
        flag = true;
        switch(state) {
            case on: state = turn_off; break;
            case off: state = turn_on; break;
            case turn_off: state = turn_on; break;
            case turn_on: state = turn_off; break;
        }
        beep();
    }
    if (!btnState && flag) {  // обработчик отпускания
        flag = false;  
    }
}

void setup() {
    pinMode(BEEPER, OUTPUT);
    pinMode(MOSFET, OUTPUT);
    pinMode(BUTTON, INPUT);
    pinMode(R_POT, INPUT);
    PWM_setup();
}

void loop() {
    //основной цикл режимов
	if(millis() - t0 > 10) {
		t0 = millis();
        
		switch(state) {
			case on: {
				ADC_Res = analogRead(R_POT) >> 2;
				Exp_Res = (sq(ADC_Res) >> 8) + 1;
				analogWrite(MOSFET, Exp_Res);
			} break;
			case off: break;
			case turn_off: {
                if(ADC_Res == 0) state = off;
                else ADC_Res--;
				Exp_Res = (sq(ADC_Res) >> 8);
				analogWrite(MOSFET, Exp_Res);
			} break;
			case turn_on: {
				ADC_Res++;
				Exp_Res = (sq(ADC_Res) >> 8);
				analogWrite(MOSFET, Exp_Res);
				if(ADC_Res >= analogRead(R_POT) >> 2) state = on;
			} break;
		}
	}

    //цикл обработки кнопки
    if(millis() - t1 > 50) {
        t1 = millis();
        BTN_Handler();
    }

}
