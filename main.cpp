#include "mbed.h"
#include "arm_book_lib.h"

// =====[Declaration and initialization of public global variables]===========

#define TIME_INCREMENT_MS 10
#define VISIT_TIME 10000 // habría que ver cómo podemos hacer para mandarle 2 minutos. Sin mucho más drama. Creería que se puede


#define BLINKING_TIME_TAKING_PICTURE 250 //si le ponemos medio segundo podriamos más o menos determinar cuantas veces en un segundo tiene que parpadear
#define BLINKING_TIME_PLAYING_AUDIO 500
#define BLINKING_TIME_RECORDING_AUDIO 500


#define TIME_SECONDS_TAKING_PICTURE 1
#define TIME_SECONDS_PLAYING_AUDIO 3
#define TIME_SECONDS_RECORDING_AUDIO 3


#define STRING_RECORDING_AUDIO "\r\n\r\nHa seleccionado la opción '2'. Presione la tecla '0' para grabar un mensaje"
#define STRING_RECORDING_AUDIO_OUTSIDE "Se ha habilitado el microfono del otro lado. Presione la tecla '0' para conocer la respuesta\r\n\r\n"

#define STRING_SUCCESS_MESSAGE_TX "\r\n\r\nSu mensaje de voz ha sido enviado.\r\n\r\n"
#define STRING_FAIL_MESSAGE_TX "\r\n\r\nLo siento, no se ha podido grabar el audio\r\n\r\n"

#define STRING_SUCCESS_MESSAGE_RX "\r\n\r\nTiene un nuevo mensaje de voz en bandeja de entrada.\r\n\r\n"
#define STRING_FAIL_MESSAGE_RX "\r\n\r\nSu mensaje de voz ha sido enviado.\r\n\r\n"

#define STRING_VISIT_TIME_IS_OVER "\r\n\r\nperdón, su tiempo de atención ha finalizado"
#define STRING_GOODBYE "\r\nMuchas gracias por su visita. Nos vemos pronto."


#define POTENTIOMETER_OVER_VOICE_LEVEL 0.5


DigitalIn doorBellButton(BUTTON1);
DigitalIn overVoiceDetector(D3);

DigitalOut CameraLed(LED1);
DigitalOut playingAudioLed(LED2);
DigitalOut recordingAudioLed(LED3); 

AnalogIn potentiometer(A0);

UnbufferedSerial uartUsb(USBTX, USBRX, 115200);

// =====[Declaration and initialization of public global variables]===========

bool buttonState = OFF;
bool doorBellState = OFF;

bool voiceDetected = OFF;
float potentiometerReading = 0.0;

int ellapsed_time = 0;

// =====[Declaration (prototypes) of public functions]===========

void inputsInit();
void outputsInit();
void updateDoorBellSystem();

void checkDoorBellBottonPress();
void startCameraLed();
void startVisitTimer();
void resetDoorBellSystem();
bool isVisitTimeOver();

void chooseOption();
void optionsMenu();
void option1();
void option2();

void option1Menu();
void blinkLedForTime(DigitalOut& led, int blinkingTime, float totalTimeInSeconds);
void keepLedOnForTime(DigitalOut& led, float timeInSeconds);

void handleVoiceMessage(const char* successMessage, const char* failMessage);
void updateVoiceDetected();

// =====[Main function, the program entry point after power on or reset]===========

int main()
{
    inputsInit();
    outputsInit();
    while (true) {
        updateDoorBellSystem();
        delay(TIME_INCREMENT_MS);
    }
}

// =====[Implementation of public functions]===========

void inputsInit()
{
    doorBellButton.mode(PullDown);
    overVoiceDetector.mode(PullDown);
}

void outputsInit()
{
    CameraLed = OFF;
    playingAudioLed = OFF;
    recordingAudioLed = OFF; 
}

void updateDoorBellSystem()
{
    checkDoorBellBottonPress();
    
    if(buttonState){
        startCameraLed();
        startVisitTimer();
        chooseOption(); 
        resetDoorBellSystem();
    }
}

void checkDoorBellBottonPress()
{
    if(doorBellButton){ //todo este sistema se dispara cuando alguien presiona el timbre por primera vez
        buttonState = ON; //la primera vez que alguien toca activamos esta variable
    }
}

void startCameraLed()
{
    if(buttonState){
        blinkLedForTime(CameraLed, BLINKING_TIME_TAKING_PICTURE, TIME_SECONDS_TAKING_PICTURE);
        buttonState = OFF;
    }
}

void startVisitTimer()
{
    CameraLed = ON; //se enciende el led porque ya ha empezado a contar
    ellapsed_time = ellapsed_time + TIME_INCREMENT_MS; //si ya hemos presionado el botón empieza a contar
}

void resetDoorBellSystem()
{
    if(isVisitTimeOver() || doorBellState == OFF){ //si nadie ha vuelto a presionar el timbre y ya se ha vencido el timer el sistema termina su ejecución
        CameraLed = OFF;
        ellapsed_time = 0;
        buttonState = OFF;
        
        uartUsb.write(STRING_GOODBYE, strlen(STRING_GOODBYE));
    }
}

bool isVisitTimeOver()
{
    if(ellapsed_time>=VISIT_TIME){
        return true;
    }
    return false;
}

void chooseOption() 
{   
    if(!isVisitTimeOver()){

        doorBellState = ON;

        optionsMenu();

        char receivedChar = '\0';
        
            uartUsb.read( &receivedChar, 1 );

            switch (receivedChar) {
                case '1':  // Opción 1
                    option1();
                    break;

                case '2':  // Opción 2
                    option2();
                    break;

                default:  // Opción inválida
                    char errorMessage[50];  // Buffer para almacenar el mensaje
                    sprintf(errorMessage, "El caracter '%c' no es una opción válida\r\n\r\n", receivedChar);
                    uartUsb.write(errorMessage, strlen(errorMessage));  // Enviar el mensaje con el carácter ingresado
                    break;
            }
    }
    
    else{
        uartUsb.write(STRING_VISIT_TIME_IS_OVER, strlen(STRING_VISIT_TIME_IS_OVER));
        doorBellState = OFF;
    }
}

void optionsMenu()
{
    uartUsb.write("Elija una de las opciones disponibles:\r\n\r\n", 40);
    uartUsb.write("Presione '1' para enviar una respuesta pre-grabada\r\n\r\n", 53);
    uartUsb.write("Presione '2' para enviar un mensaje de voz\r\n\r\n", 47);

}

void option1()
{
    option1Menu();

    char receivedChar = '\0';

    uartUsb.read( &receivedChar, 1 );

    if(receivedChar == '1' || receivedChar == '2' || receivedChar == '3'){
        blinkLedForTime(playingAudioLed, BLINKING_TIME_PLAYING_AUDIO, TIME_SECONDS_PLAYING_AUDIO);
        doorBellState = OFF;
    }

    else if(receivedChar == '0'){
        chooseOption(); 
    }

    else{
        char errorMessage[50];  // Buffer para almacenar el mensaje
        sprintf(errorMessage, "El caracter '%c' no es una opción válida\r\n\r\n", receivedChar);
        uartUsb.write(errorMessage, strlen(errorMessage));  // Enviar el mensaje con el carácter ingresado
        option1();
    }
}

void option2()
{
    uartUsb.write(STRING_RECORDING_AUDIO, strlen(STRING_RECORDING_AUDIO));

    // Manejamos la grabación del primer mensaje
    handleVoiceMessage(STRING_SUCCESS_MESSAGE_TX,
                       STRING_FAIL_MESSAGE_TX);

    uartUsb.write(STRING_RECORDING_AUDIO_OUTSIDE, strlen(STRING_RECORDING_AUDIO_OUTSIDE));

    // Manejamos la grabación del segundo mensaje
    handleVoiceMessage(STRING_SUCCESS_MESSAGE_RX,
                       STRING_FAIL_MESSAGE_RX);

    doorBellState = OFF;
}

void option1Menu()
{
    uartUsb.write("\r\n\r\nUsted ha elegido la opción 1. Elija una de las respuestas disponibles:\r\n\r\n", 79);
    uartUsb.write("'1' - Buen día, si ¿qué necesita?\r\n\r\n", 39);
    uartUsb.write("'2' - El señor #### lo atenderá en un momento\r\n\r\n", 50);
    uartUsb.write("'3' - La señora #### lo atenderá en un momento\r\n\r\n", 51);
    uartUsb.write("'0' - VOLVER AL MENU PRINCIPAL\r\n\r\n", 35);
}

void blinkLedForTime(DigitalOut& led, int blinkingTime, float totalTimeInSeconds) 
{
    int totalTime = totalTimeInSeconds * 1000;  // Convertir segundos a milisegundos
    int elapsedTime = 0;
    
    while (elapsedTime < totalTime) {
        led = !led;  // Cambiar el estado del LED
        delay(blinkingTime);  // Esperar el tiempo de parpadeo
        elapsedTime += blinkingTime;  // Aumentar el tiempo transcurrido
    }
    
    led = OFF;  // Asegurarse de que el LED quede apagado al final
}

void keepLedOnForTime(DigitalOut& led, float timeInSeconds) 
{
    led = ON;  // Encender el LED
    delay(timeInSeconds * 1000);  // Mantenerlo encendido por el tiempo especificado (convertido a milisegundos)
    led = OFF;  // Apagar el LED después del tiempo transcurrido
}

void handleVoiceMessage(const char* successMessage, const char* failMessage) 
{
    char receivedChar = '\0';
    uartUsb.read(&receivedChar, 1);

    if(receivedChar == '0'){
        updateVoiceDetected();
        if(voiceDetected){
            uartUsb.write(successMessage, strlen(successMessage));
        } else {
            uartUsb.write(failMessage, strlen(failMessage));
        }
    }
}

void updateVoiceDetected() 
{
    potentiometerReading = potentiometer.read();
    
    if (potentiometerReading > POTENTIOMETER_OVER_VOICE_LEVEL) {
        voiceDetected = ON;
    } else {
        voiceDetected = OFF;
    }
}