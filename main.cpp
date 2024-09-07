#include "mbed.h"
#include "arm_book_lib.h"

// =====[Declaration and initialization of public global variables]===========

#define TIME_MS 500 //lo vamos a tener que hacer bloqueante a proposito...
#define TIME_INCREMENT_MS 10
#define VISIT_TIME 4000 // habría que ver cómo podemos hacer para mandarle 2 minutos. Sin mucho más drama. Creería que se puede

#define BLINKING_TIME_TAKING_PICTURE 250 //si le ponemos medio segundo podriamos más o menos determinar cuantas veces en un segundo tiene que parpadear
#define BLINKING_TIME_PLAYING_AUDIO 500
#define BLINKING_TIME_RECORDING_AUDIO 500

#define TIME_SECONDS_TAKING_PICTURE 1
#define TIME_SECONDS_PLAYING_AUDIO 3
#define TIME_SECONDS_RECORDING_AUDIO 3

DigitalIn doorBellButton(D7);

DigitalOut ringBellLed(LED1); //esto ya lo vamos a hacer con el Doxygen
DigitalOut playingAudioLed(LED2);
DigitalOut recordingAudioLed(LED3); 

UnbufferedSerial uartUsb(USBTX, USBRX, 115200);

// =====[Declaration and initialization of public global variables]===========

bool doorBellButtonState = OFF;
bool cameraState = OFF;
bool optionState = OFF;


bool recordingAudioState = OFF;
int ellapsed_time = 0;
int accumulatedTimeRecordedAudio = 0;


// =====[Declaration (prototypes) of public functions]===========

void inputsInit();
void outputsInit();

void doorBellUpdate();
void checkDoorBellPress();
void startCameraLed();
void startVisitTimer();
void checkVisitTimer();

void optionsMenu();
void chooseOption();
void option1();
void option1Menu();
void blinkLedForTime(DigitalOut& led, int blinkingTime, float totalTimeInSeconds);
void option2();
void keepLedOnForTime(DigitalOut& led, float timeInSeconds);

// =====[Main function, the program entry point after power on or reset]===========

int main()
{
    inputsInit();
    outputsInit();
    while (true) {
        doorBellUpdate();
        delay(TIME_INCREMENT_MS);
    }

}

// =====[Implementation of public functions]===========

void inputsInit()
{
    doorBellButton.mode(PullDown); //la idea es que este botón solo haga las transiciones de estado
}

void outputsInit()
{
    ringBellLed = OFF; // podría hacer que el led parpadee y de ahi que se quede prendido
    playingAudioLed = OFF; // 
    recordingAudioLed = OFF; 
}

void doorBellUpdate()
{
    checkDoorBellPress();
    
    if(doorBellButtonState){
        startCameraLed();
        startVisitTimer();
        checkVisitTimer(); //acá si vamos a tener una cosa bloqueante asi que hay que ver bien...
        //chooseOption(); 
    }
}

void checkDoorBellPress(){
    if(doorBellButton){ //todo este sistema se dispara cuando alguien presiona el timbre por primera vez
        doorBellButtonState = ON; //la primera vez que alguien toca activamos esta variable
    }
}

void startCameraLed(){
    if(doorBellButtonState && ellapsed_time==0){
        blinkLedForTime(ringBellLed, BLINKING_TIME_TAKING_PICTURE, TIME_SECONDS_TAKING_PICTURE);
    }
}

void startVisitTimer(){
    
    ringBellLed = ON; //se enciende el led porque ya ha empezado a contar
    ellapsed_time = ellapsed_time + TIME_INCREMENT_MS; //si ya hemos presionado el botón empieza a contar
}

void checkVisitTimer(){
    if(ellapsed_time>=VISIT_TIME){ //si nadie ha vuelto a presionar el timbre y ya se ha vencido el timer el sistema termina su ejecución
        ringBellLed = OFF;
        ellapsed_time = 0;
        doorBellButtonState = OFF;
        optionState = ON;
    }
}

void chooseOption() 
{
    optionsMenu();

    char receivedChar = '\0';
    if( uartUsb.readable() ) {
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
    if( uartUsb.readable() ) {
        uartUsb.read( &receivedChar, 1 );

        if(receivedChar == '1' || receivedChar == '2' || receivedChar == '3'){
            blinkLedForTime(playingAudioLed, BLINKING_TIME_PLAYING_AUDIO, TIME_SECONDS_PLAYING_AUDIO);
        }

        else if(receivedChar == '0'){
            chooseOption(); 
        }

        else{
            char errorMessage[50];  // Buffer para almacenar el mensaje
            sprintf(errorMessage, "El caracter '%c' no es una opción válida\r\n\r\n", receivedChar);
            uartUsb.write(errorMessage, strlen(errorMessage));  // Enviar el mensaje con el carácter ingresado
        }
    }
}

void option1Menu()
{
    uartUsb.write("Usted ha elegido la opción 1. Elija una de las respuestas disponibles:\r\n\r\n", 73);
    uartUsb.write("'1' - Buen día, si ¿qué necesita?\r\n\r\n", 39);
    uartUsb.write("'2' - El señor #### lo atenderá en un momento\r\n\r\n", 50);
    uartUsb.write("'3' - La señora #### lo atenderá en un momento\r\n\r\n", 51);
    uartUsb.write("'0' - VOLVER AL MENU PRINCIPAL\r\n\r\n", 35);
}

void blinkLedForTime(DigitalOut& led, int blinkingTime, float totalTimeInSeconds) {
    int totalTime = totalTimeInSeconds * 1000;  // Convertir segundos a milisegundos
    int elapsedTime = 0;
    
    while (elapsedTime < totalTime) {
        led = !led;  // Cambiar el estado del LED
        delay(blinkingTime);  // Esperar el tiempo de parpadeo
        elapsedTime += blinkingTime;  // Aumentar el tiempo transcurrido
    }
    
    led = OFF;  // Asegurarse de que el LED quede apagado al final
}

void option2()
{
     uartUsb.write("Ha seleccionado la opción '2'. Grabando mensaje de voz...\r\n\r\n", 59);
     blinkLedForTime(recordingAudioLed, BLINKING_TIME_RECORDING_AUDIO, TIME_SECONDS_RECORDING_AUDIO);
     uartUsb.write("Su mensaje de voz ha sido enviado.\r\n\r\n", 38);
     uartUsb.write("Se ha habilitado el microfono del otro lado.\r\n\r\n", 48);
     uartUsb.write("Esperando respuesta...\r\n\r\n", 25);
     keepLedOnForTime(recordingAudioLed, TIME_SECONDS_RECORDING_AUDIO);
}

void keepLedOnForTime(DigitalOut& led, float timeInSeconds) {
    led = ON;  // Encender el LED
    delay(timeInSeconds * 1000);  // Mantenerlo encendido por el tiempo especificado (convertido a milisegundos)
    led = OFF;  // Apagar el LED después del tiempo transcurrido
}
