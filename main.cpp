#include "mbed.h"
#include "arm_book_lib.h"

// =====[Declaration and initialization of public global variables]===========

#define VISIT_TIME 1000*60*2 // habría que ver cómo podemos hacer para mandarle 2 minutos. Sin mucho más drama. Creería que se puede
#define BLINKING_TIME_TAKING_PICTURE 500 //si le ponemos medio segundo podriamos más o menos determinar cuantas veces en un segundo tiene que parpadear
#define BLINKING_TIME_PLAYING_AUDIO 500
#define BLINKING_TIME_RECORDING_AUDIO 500

DigitalIn doorBellButtom(D7);

DigitalOut ringBellLed(LED1); //esto ya lo vamos a hacer con el Doxygen
DigitalOut playingAudioLed(LED2);
DigitalOut recordingAudioLed(LED3); 

UnbufferedSerial uartUsb(USBTX, USBRX, 115200);

// =====[Declaration and initialization of public global variables]===========

bool ringBellState = false; //la idea de esto es que no utilicemos el 
bool optionState = false; //si no se ha seleccionado ninguna opción todavía, está en cero

// =====[Declaration (prototypes) of public functions]===========

void inputsInit();
void outputsInit();

void testStates();

void doorbellActivationUpdate();

void uartTask();
void optionsMenu();
void option1();
void option2();
void availableCommandsAudioPreRecorded();


// =====[Main function, the program entry point after power on or reset]===========

int main()
{
    inputsInit();
    outputsInit();
    while (true) {
        doorbellActivationUpdate();
    }

}

// =====[Implementation of public functions]===========

void inputsInit()
{
    doorBellButtom.mode(PullDown); //la idea es que este botón solo haga las transiciones de estado
}

void outputsInit()
{
    ringBellLed = OFF; // podría hacer que el led parpadee y de ahi que se quede prendido
    playingAudioLed = OFF; // 
    recordingAudioLed = OFF; 
}

void doorbellActivationUpdate()
{
    if(VISIT_TIME) //revisamos que el timer aún no se haya vencido

    uartUsb.write("Hay alguien en la entrada principal.\r\n", 21 );
    uartTask();
    // y una vez que sale de uartTask ya debería apagar el led, dejar todo como estaba y terminar la secuencia.|
}


void uartTask() {
    
    optionsMenu();
    
    char receivedChar = '\0';  // Variable para almacenar el carácter recibido
    
    if (uartUsb.readable()) {
        uartUsb.read(&receivedChar, 1);  // Leer un carácter de UART

        switch (receivedChar) {
            case '1':  // Opción 2
                option1();
                optionState = true;
                // Aquí puedes agregar el código necesario para manejar la opción 2
                break;

            case '2':  // Opción 2
                option2();
                optionState = true;
                // Aquí puedes agregar el código necesario para manejar la opción 2
                break;

            default:  // Cualquier opción inválida
                uartUsb.write("Please enter a valid option (1 or 2)\r\n", 35);
                optionsMenu();  // Mostrar los comandos disponibles
                break;
        }
    }
}

void optionsMenu()
{
    uartUsb.write( "Elija una de las siguientes opciones:\r\n", 21 );
    uartUsb.write( "Presione '1' para enviar una respuesta pre-grabada\r\n\r\n", 36 );
    uartUsb.write( "Presione '2' para enviar un mensaje de voz\r\n\r\n", 36 );
}

void option1(){

    availableCommandsAudioPreRecorded();
    
    char receivedChar = '\0';  // Variable para almacenar el carácter recibido
    
    if (uartUsb.readable()) {
        uartUsb.read(&receivedChar, 1);  // Leer un carácter de UART

        switch (receivedChar) {
            case '1':  // Opción 2
                playingAudioLed = ON;
                delay(BLINKING_TIME_PLAYING_AUDIO);
                // Aquí puedes agregar el código necesario para manejar la opción 2
                break;

            case '2':  // Opción 2
                playingAudioLed = ON;
                delay(BLINKING_TIME_PLAYING_AUDIO);
                // Aquí puedes agregar el código necesario para manejar la opción 2
                break;

            case '0':  // Opción 2
                uartTask();
                // Aquí puedes agregar el código necesario para manejar la opción 2
                break;                

            default:  // Cualquier opción inválida
                uartUsb.write("Please enter a valid option (1, 2 or 0)\r\n", 35);
                availableCommandsAudioPreRecorded();  // Mostrar los comandos disponibles
                break;
        }
    }
}

void option2(){
    
}

void availableCommandsAudioPreRecorded()
{
    uartUsb.write( "Elija una de las respuestas disponibles:\r\n", 21 );
    uartUsb.write( "'1' - Buen día, si ¿qué necesita?\r\n\r\n", 36 );
    uartUsb.write( "'2' - otra respuesta generica...'\r\n\r\n", 36 );
    uartUsb.write( "'0' - VOLVER AL MENU PRINCIPAL\r\n\r\n", 36 );
}

/*habría que añadir una variable de estado para que ya no se pueda volver a seleccionar
una opción más de una vez*/