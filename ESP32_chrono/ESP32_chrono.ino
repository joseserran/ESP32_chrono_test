#include <LedControlMS.h>
#include <WiFi.h>
#include <WebServer.h>

// Crear una instancia del servidor en el puerto 80
WebServer server(80);

// Configuración para 4 matrices controladas por MAX7219
// Pin DataIn, Clock y Load
LedControl lc = LedControl(6, 4, 3, 4); // PARA ESP32-C3 MINI

unsigned long previousMillis = 0;
int decimas = 0;
int segundos = 0;
int minutos = 0;
int intervalo = 100; // 100 ms para las décimas de segundo

void setup() {
  pinMode(8, OUTPUT);
  Serial.begin(9600);

  // Configurar el ESP32 como un punto de acceso
  const char* ssid_ap = "ESP32_Access_Point";
  const char* password_ap = "12345678"; // Contraseña de la red Wi-Fi

  WiFi.softAP(ssid_ap, password_ap);
  Serial.println("Punto de acceso iniciado.");
  Serial.print("Dirección IP: ");
  Serial.println(WiFi.softAPIP());

  // Configurar la página principal del servidor web
  server.on("/", []() {
    server.send(200, "text/html", "<h1>Hola Mundo</h1>");
  });

  // Iniciar el servidor
  server.begin();
  Serial.println("Servidor web iniciado.");

  // Inicialización de las 4 matrices (índice 0-3)
  for (int i = 0; i < 4; i++) {
    lc.shutdown(i, false); // Salir del modo apagado
    lc.setIntensity(i, 8); // Brillo medio (0-15)
    lc.clearDisplay(i);     // Limpiar matriz
  }
}

void loop() {
  // Llamar al servidor para manejar las peticiones de los clientes
  server.handleClient();

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= intervalo) {
    previousMillis = currentMillis;
    
    // Incremento de décimas de segundo
    decimas++;
    if (decimas%2 == 0)
    {
        digitalWrite(8, HIGH);  // turn the LED on (HIGH is the voltage level)
    }
    else{
        digitalWrite(8, LOW);
    }
    if (decimas == 10) {
      decimas = 0;
      segundos++;
      enviarDatosPorSerie(minutos, segundos, decimas);
    }
    
    // Incremento de segundos
    if (segundos == 60) {
      segundos = 0;
      minutos++;
    }
    
    // Actualizar el cronómetro en las matrices
    mostrarCronometro(minutos, segundos, decimas);
  }
}

// Función para mostrar el cronómetro
void mostrarCronometro(int minutos, int segundos, int decimas) {
  lc.clearDisplay(0);
  lc.clearDisplay(1);
  lc.clearDisplay(2);
  lc.clearDisplay(3);

  // Dibujar minutos en la primera matriz (1 dígito si es menor a 10, 2 si es mayor)
  if (minutos < 10) {
    lc.setDigit(0, 7, minutos, false);
  } else {
    lc.setDigit(0, 7, minutos / 10, false);  // Decenas de minutos
    lc.setDigit(1, 7, minutos % 10, false);  // Unidades de minutos
  }

  // Dibujar ':' entre minutos y segundos (siempre en la posición fija)
  lc.setChar(1, 7, ':', false);

  // Dibujar segundos en la tercera matriz (dos dígitos siempre)
  lc.setDigit(2, 7, segundos / 10, false);  // Decenas de segundos
  lc.setDigit(3, 7, segundos % 10, false);  // Unidades de segundos

  // Dibujar décimas de segundo en la cuarta matriz
  lc.setDigit(3, 7, decimas, false);        // Décimas de segundo
}

// Función para enviar los datos del cronómetro por el puerto serie
void enviarDatosPorSerie(int minutos, int segundos, int decimas) {
  // Formatear y enviar la información
  if (minutos < 10) Serial.print("0");  // Añadir un 0 si los minutos son menores de 10
  Serial.print(minutos);
  Serial.print(":");
  
  if (segundos < 10) Serial.print("0");  // Añadir un 0 si los segundos son menores de 10
  Serial.print(segundos);
  Serial.print(".");
  
  // Enviar las décimas de segundo
  Serial.println(decimas);
}
