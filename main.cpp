#include "mbed.h"
#include "arm_book_lib.h"

// =====[Declaration and initialization of public global variables]===========

#define TIME_MS 100 //lo vamos a tener que hacer bloqueante a proposito...
#define VISIT_TIME 5000 // habría que ver cómo podemos hacer para mandarle 2 minutos. Sin mucho más drama. Creería que se puede
#define BLINKING_TIME_TAKING_PICTURE 500 //si le ponemos medio segundo podriamos más o menos determinar cuantas veces en un segundo tiene que parpadear
#define BLINKING_TIME_PLAYING_AUDIO 500
#define BLINKING_TIME_RECORDING_AUDIO 500

//DigitalIn doorBellButton(D7);
DigitalIn doorBellButton(BUTTON1);

DigitalOut ringBellLed(LED1); //esto ya lo vamos a hacer con el Doxygen
DigitalOut playingAudioLed(LED2);
DigitalOut recordingAudioLed(LED3); 

UnbufferedSerial uartUsb(USBTX, USBRX, 115200);

// =====[Declaration and initialization of public global variables]===========

bool doorBellButtonState = OFF;
bool optionState = OFF;

bool cameraState = OFF; //este estado tiene que activarse cuando ringBellState=ON
//este estado tiene que hacer parpadear el LED para indicar que se ha activado la camara

bool playingAudioState = OFF;
//se supone que 

bool recordingAudioState = OFF;

int ellapsed_time = 0;

const int mensajesLengths[] = {45, 48, 47};  // Longitudes de las cadenas de texto


// =====[Declaration (prototypes) of public functions]===========

void inputsInit();
void outputsInit();

void testStates();

void doorbellActivationUpdate();
void optionsMenu();
void chooseOption();
void option1();
void option1Menu();
void option2();

// =====[Main function, the program entry point after power on or reset]===========

int main()
{
    inputsInit();
    outputsInit();
    while (true) {
        doorbellActivationUpdate();
        delay(TIME_MS);
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

void doorbellActivationUpdate()
{

    if(doorBellButton){ //todo este sistema se dispara cuando alguien presiona el timbre por primera vez
        doorBellButtonState = ON; //la primera vez que alguien toca activamos esta variable
    }
    
    if(doorBellButtonState){
        ellapsed_time = ellapsed_time + TIME_MS; //si ya hemos presionado el botón empieza a contar
        ringBellLed = ON; //se enciende el led porque ya ha empezado a contar
        
        
        if(optionState){
            chooseOption();
            //optionState = OFF;
        }
        

        if(ellapsed_time>=VISIT_TIME){ //si nadie ha vuelto a presionar el timbre y ya se ha vencido el timer el sistema termina su ejecución
            ringBellLed = OFF;
            ellapsed_time = 0;
            doorBellButtonState = OFF;
            optionState = ON;
        }
    }
}

//okey ahora toca implementar los comandos de la UART y con eso ya estaríamos 

void chooseOption() 
{
    
    optionsMenu();

    char receivedChar = '\0';
    if( uartUsb.readable() ) {
        uartUsb.read( &receivedChar, 1 );

        switch (receivedChar) {
            case '1':  // Opción 1
                uartUsb.write("Elegiste la opcion 1\r\n\r\n", 25);
                break;

            case '2':  // Opción 2
                uartUsb.write("Elegiste la opcion 2\r\n\r\n", 25);
                break;

            default:  // Opción inválida
                uartUsb.write("Por favor, elija una opción válida.\r\n", 37);
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

void option1Menu()
{
    uartUsb.write( "Usted ha elegido la opción 1. Elija una de las respuestas disponibles:\r\n", 21 );
    uartUsb.write( "'1' - Buen día, si ¿qué necesita?\r\n\r\n", 36 );
    uartUsb.write( "'2' - otra respuesta generica...'\r\n\r\n", 36 );
    uartUsb.write( "'0' - VOLVER AL MENU PRINCIPAL\r\n\r\n", 36 );
}



/*void option1()
{
    option1Menu();

    char receivedChar = '\0';  // Variable para almacenar el carácter recibido
    int numMensajes = sizeof(mensajes) / sizeof(mensajes[0]);  // Número total de mensajes

    if (uartUsb.readable()) {
        uartUsb.read(&receivedChar, 1);  // Leer un carácter de UART

        int index = receivedChar - '1';  // Convertir el carácter a índice ('1' -> 0, '2' -> 1, etc.)

        if (index >= 0 && index < numMensajes) {
            uartUsb.write(mensajes[index], mensajesLengths[index]);  // Usar el array de longitudes
            playingAudioLed = ON;  // Encender el LED de reproducción
            delay(BLINKING_TIME_PLAYING_AUDIO);  // Parpadear LED
            playingAudioLed = OFF;  // Apagar el LED
        } 
        else if (receivedChar == '0') {
            chooseOption();  // Volver al menú principal
        } 
        else {
            uartUsb.write("Por favor, elija una opción válida.\r\n", 35);
            option1();  // Volver a llamar a option1
        }
    }
}

void chooseOption() 
{
    optionState = OFF;
    optionsMenu();

    char receivedChar = '\0';  // Variable para almacenar el carácter recibido
    
    if (uartUsb.readable()) {
        uartUsb.read(&receivedChar, 1);  // Leer un carácter de UART

        switch (receivedChar) {
            case '1':  // Opción 2
                uartUsb.write("Elegiste la opcion 1\r\n", 35);
                // Aquí puedes agregar el código necesario para manejar la opción 2
                break;

            case '2':  // Opción 2
                uartUsb.write("Elegiste la opcion 1\r\n", 35);
                // Aquí puedes agregar el código necesario para manejar la opción 2
                break;

            default:  // Cualquier opción inválida
                uartUsb.write("Por favor, elija una opción válida.\r\n", 35);
                //chooseOption(); //espero que no se rompa nada por hacerla recursiva.
                break;
        }
    }
}

void option1Menu2()
{
    uartUsb.write("Usted ha elegido la opción 1. Elija una de las respuestas disponibles:\r\n", 21);

    // Mostrar cada mensaje en el menú
    for (int i = 0; i < sizeof(mensajes) / sizeof(mensajes[0]); i++) {
        uartUsb.write("'", 1);
        uartUsb.write(std::to_string(i + 1).c_str(), 1);  // Mostrar el número del mensaje
        uartUsb.write("' - ", 4);
        uartUsb.write(mensajes[i], strlen(mensajes[i]));
    }
    
    uartUsb.write("'0' - VOLVER AL MENU PRINCIPAL\r\n", 36);
}




void option2()
{

}

void sendVoiceMessage()  
{
    uartUsb.write( "Se ha habilitado el microfono del otro lado. Esperando respuesta...\r\n", 21 );
    uartUsb.write( "'1' - Buen día, si ¿qué necesita?\r\n\r\n", 36 );
    uartUsb.write( "'2' - otra respuesta generica...'\r\n\r\n", 36 );
    uartUsb.write( "'0' - VOLVER AL MENU PRINCIPAL\r\n\r\n", 36 );
}
*/