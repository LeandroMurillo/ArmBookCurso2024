//=====[Libraries]=============================================================

#include "mbed.h"
#include "arm_book_lib.h"
#include <vector> // Incluir la librería para vectores

//=====[Defines]===============================================================

#define TIME_INCREMENT_MS 10
#define WAIT_TIME 10000 //en ms

#define BLINKING_TIME_TAKING_PICTURE 250 
#define BLINKING_TIME_PLAYING_AUDIO 500
#define BLINKING_TIME_RECORDING_AUDIO 500

#define TIME_SECONDS_TAKING_PICTURE 1
#define TIME_SECONDS_PLAYING_AUDIO 3
#define TIME_SECONDS_RECORDING_AUDIO 3

#define POTENTIOMETER_OVER_VOICE_LEVEL 0.5

//=====[Declaration and initialization of public global objects]===========
  
DigitalIn doorBellButton(BUTTON1);

AnalogIn potentiometer(A0);

DigitalOut cameraLed(LED1);
DigitalOut playingAudioLed(LED2);
DigitalOut recordingAudioLed(LED3); 

UnbufferedSerial uartUsb(USBTX, USBRX, 115200);

std::vector<const char*> optionsMenuMessages = {
    "\r\nElija una de las opciones disponibles:\r\n",
    "\r\nPresione '1' para enviar una respuesta pre-grabada\r\n",
    "\r\nPresione '2' para enviar un mensaje de voz\r\n"
};

std::vector<const char*> option1MenuMessages = {
        "\r\n\r\nUsted ha elegido la opción 1. Elija una de las respuestas disponibles:\r\n\r\n",
        "'1' - Buen día, si ¿qué necesita?\r\n\r\n",
        "'2' - El señor #### lo atenderá en un momento\r\n\r\n",
        "'3' - La señora #### lo atenderá en un momento\r\n\r\n",
        "'0' - VOLVER AL MENU PRINCIPAL\r\n\r\n"
};

// =====[Declaration and initialization of public global variables]===========

bool buttonState = OFF;
bool doorBellState = OFF;
bool voiceDetected = OFF;

int wait_timer = 0;

float potentiometerReading = 0.0;

// =====[Declaration (prototypes) of public functions]===========

void inputsInit();
void outputsInit();
void updateDoorBellSystem();

void checkDoorBellBottonPress();
void startCameraLed();
void startWaitTimer();
void chooseOption();
void resetDoorBellSystem();

void option1();
void option2();
void sendInvalidOptionMessage(char receivedChar);

void option1Menu();
void blinkLedForTime(DigitalOut& led, int blinkingTime, float totalTimeInSeconds);

void handleVoiceMessage(const char* successMessage, const char* failMessage);
void updateVoiceDetected();

bool isWaitTimerOver();
void showMessages(const std::vector<const char*>& messages);

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
}

void outputsInit()
{
    cameraLed = OFF;
    playingAudioLed = OFF;
    recordingAudioLed = OFF; 
}

void updateDoorBellSystem()
{
    checkDoorBellBottonPress();
    
    if(buttonState){
        startCameraLed();
        startWaitTimer();
        chooseOption(); 
        resetDoorBellSystem();
    }
}

void checkDoorBellBottonPress()
{
    if(doorBellButton){ 
        buttonState = ON; 
    }
}

void startCameraLed()
{
    if(buttonState){
        blinkLedForTime(cameraLed, BLINKING_TIME_TAKING_PICTURE, TIME_SECONDS_TAKING_PICTURE);
        showMessages({"\r\n\r\nHola! tenes a una nueva visita en la puerta."});
        buttonState = OFF;
    }
}

void startWaitTimer()
{
    cameraLed = ON; 
    wait_timer = wait_timer + TIME_INCREMENT_MS; 
}

void chooseOption() 
{
    int maxWaitTime = WAIT_TIME; // Tiempo máximo de espera para la interacción

    if(!isWaitTimerOver()) {
        doorBellState = ON;
        showMessages(optionsMenuMessages);

        char receivedChar = '\0';
        int elapsedTime = 0; // Reiniciar el tiempo para esta opción

        // Controla el tiempo mientras espera la entrada del usuario
        while (elapsedTime < maxWaitTime && receivedChar == '\0') {
            if (uartUsb.readable()) {
                uartUsb.read(&receivedChar, 1);
            }
            delay(TIME_INCREMENT_MS); // Incremento de tiempo
            elapsedTime += TIME_INCREMENT_MS;
        }

        if (receivedChar != '\0') {
            switch (receivedChar) {
                case '1':  // Opción 1
                    option1();
                    break;

                case '2':  // Opción 2
                    option2();
                    break;

                default:  // Opción inválida
                    sendInvalidOptionMessage(receivedChar);
                    chooseOption(); // Volver a elegir opción si es inválida
                    break;
            }
        } else {
            // Si el tiempo se acaba sin recibir entrada
            showMessages({"\r\n\r\nPerdon, su tiempo de atencion ha finalizado."});
            doorBellState = OFF;
        }
    } else {
        showMessages({"\r\n\r\nPerdon, su tiempo de atencion ha finalizado."});;
        doorBellState = OFF;
    }
}

void resetDoorBellSystem()
{
    if(isWaitTimerOver() || doorBellState == OFF){ 
        cameraLed = OFF;
        wait_timer = 0;
        buttonState = OFF;
        showMessages({"\r\n\r\nMuchas gracias por su visita. Nos vemos pronto."});
    }
}

void sendInvalidOptionMessage(char receivedChar) 
{
    char errorMessage[50];  // Buffer para almacenar el mensaje
    sprintf(errorMessage, "El caracter '%c' no es una opción válida\r\n\r\n", receivedChar);
    showMessages({errorMessage});  // Enviar el mensaje con el carácter ingresado
}

void option1()
{
    option1Menu();

    char receivedChar = '\0';
    int maxWaitTime = WAIT_TIME;
    int elapsedTime = 0; // Reiniciar el tiempo para esta opción

    // Controla el tiempo mientras espera la entrada del usuario
    while (elapsedTime < maxWaitTime && receivedChar == '\0') {
        if (uartUsb.readable()) {
            uartUsb.read(&receivedChar, 1);
        }
        delay(TIME_INCREMENT_MS);
        elapsedTime += TIME_INCREMENT_MS;
    }

    if (receivedChar == '1' || receivedChar == '2' || receivedChar == '3') {
        blinkLedForTime(playingAudioLed, BLINKING_TIME_PLAYING_AUDIO, TIME_SECONDS_PLAYING_AUDIO);
        doorBellState = OFF;

    } else if (receivedChar == '0') {
        chooseOption();

    } else {
        sendInvalidOptionMessage(receivedChar);
        option1(); // Volver a la opción 1 si es inválido
    }
}

void option2()
{
    // Le pide al usuario marcar '0' en el teclado para grabar un mensaje
    showMessages({"\r\n\r\nHa seleccionado la opcion '2'. Presione la tecla '0' para grabar un mensaje de voz."});

    // Manejamos la grabación del primer mensaje
    handleVoiceMessage("\r\n\r\nSu mensaje de voz ha sido enviado.",
                       "\r\n\r\nLo siento, no se ha podido grabar el mensaje de voz.");

    // Le pide al usuario marcar '0' para escuchar el mensaje que se grabó en la entrada.
    showMessages({"\r\nSe ha habilitado el microfono del otro lado. Presione la tecla '0' para conocer la respuesta."});

    // Manejamos la grabación del segundo mensaje
    handleVoiceMessage("\r\n\r\nTiene un nuevo mensaje de voz en bandeja de entrada.",
                       "\r\n\r\nNo se ha escuchado nada al grabar el mensaje de voz en la entrada.");

    doorBellState = OFF;
}

void option1Menu()
{
    showMessages(option1MenuMessages);
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

void handleVoiceMessage(const char* successMessage, const char* failMessage) 
{
    char receivedChar = '\0';
    int blinkingTime = BLINKING_TIME_RECORDING_AUDIO;
    int maxWaitTime = WAIT_TIME;
    int elapsedTime = 0; // Reiniciar el tiempo para este proceso

    // Parpadea el LED mientras espera la entrada del usuario
    while (receivedChar != '0' && elapsedTime < maxWaitTime) {
        recordingAudioLed = !recordingAudioLed;
        delay(blinkingTime);
        elapsedTime += blinkingTime;

        if (uartUsb.readable()) {
            uartUsb.read(&receivedChar, 1);
        }
    }

    recordingAudioLed = OFF;

    if (receivedChar == '0') {
        updateVoiceDetected();

        if(voiceDetected) {
            showMessages({successMessage});
        } else {
            showMessages({failMessage});
        }
    } else {
        showMessages({"\r\n\r\nLo siento, no se ha podido grabar el mensaje de voz."});
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

bool isWaitTimerOver()
{
    if(wait_timer>=WAIT_TIME){
        return true;
    }
    return false;
}

// Función para mostrar un vector de mensajes
void showMessages(const std::vector<const char*>& messages) {
    for (const auto& message : messages) {
        uartUsb.write(message, strlen(message));  // Envía cada mensaje por UART
    }
}

