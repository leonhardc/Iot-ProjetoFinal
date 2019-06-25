// Bibliotecas
#include <ESP8266WiFi.h>
#include <DHT.h>
#include <WiFiClientSecure.h> 
#include "Gsender.h"


// Dados da Rede
const char* ssid     = "FUTURE HOUSE NEW";
const char* password = "casadofuturoufc123";

const char* host = "api.pushbullet.com";
const int httpsPort = 443;
const char* PushBulletAPIKEY = "o.hjuNUbvhPH4R6KdJfr696OsuDgGjNUqy";



// Seta Porta 80 para o Servidor Web
WiFiServer server(80);
// Variável que guarda requisição HTTP
String header;

#define DHTTYPE DHT11
#define dht_dpin 0
const short buzzerPin = 13;
const short ledPin = 5;
const short ldrPin = 4;
const short soundPin = 14;
const short firePin = 2;
bool estadoSensores = true;

DHT dht(dht_dpin, DHTTYPE);

String lumi(bool a){
  if(digitalRead(ldrPin) == 1 && a == true){
    return "Luminosidade adequada";
  } else if(a == true) {
    WiFiClientSecure client;
    Serial.print("Conectado - ");
    Serial.println(host);
    if (!client.connect(host, httpsPort)) {
    Serial.println("Falhou ao conectar ao pushbullet");
    }
    pushBullet(client, "Alerta: Luminosidade inadequada");
    return "Luminosidade inadequada";
  } else if(a == false){
    return "Sensor desativado";
  }
}

String som(bool a){
  if(analogRead(A0)<650 && digitalRead(soundPin) == 0 && a == true){
    return "Níveis acusticos abaixo de 65dB";
  }else if(a == true){
    WiFiClientSecure client;
    Serial.print("Conectado - ");
    Serial.println(host);
    if (!client.connect(host, httpsPort)) {
      Serial.println("Falhou ao conectar ao pushbullet");
    }
  pushBullet(client, "Alerta: Níveis acusticos acima de 65dB");
  return "Níveis acusticos acima de 65dB";
  } else if(a == false){
    return "Sensor desativado";
  }
}

String umiTemp(float t, float h, bool a){
  if(t <= 23 && t >= 20 && h >= 40 && a == true){
    return "Temperatura e/ou umidade adequada";
  }else if (a == true){
    return "Temperatura e/ou umidade inadequada";
  } else if(a == false){
    return "Sensor desativado";
  }
}

String fire(bool a){
  if(digitalRead(firePin) == 1 && a == true){
    digitalWrite(ledPin,LOW);
    tone(buzzerPin, 0);
    return "Não há indicios de fogo";
  }else if (a == true){
    int n=0;
    digitalWrite(ledPin,HIGH);
      while(n < 5){
      tone(buzzerPin,450);
      delay(1000);
      tone(buzzerPin, 350);
      delay(300);
    n++;
    }
    tone(buzzerPin,450);
    Gsender *gsender = Gsender::Instance();    // Getting pointer to class instance
    String subject = "SaMCAT - Emergência";
    if(gsender->Subject(subject)->Send("fbrenomoura@gmail.com", "Fogo detectado")) {
        Serial.println("Message send.");
    } else {
        Serial.print("Error sending message: ");
        Serial.println(gsender->getError());
    }
    tone(buzzerPin, 350);    
    WiFiClientSecure client;
    Serial.print("Conectado - ");
    Serial.println(host);
    if (!client.connect(host, httpsPort)) {
      Serial.println("Falhou ao conectar ao pushbullet");
    }
  tone(buzzerPin,450);
  pushBullet(client, "Alerta: FOGO DETECTADO");

  tone(buzzerPin, 350);  
      return "FOGO DETECTADO";
  } else if (a == false){
    return "Sensor desativado";
  }
}

void pushBullet(WiFiClientSecure client, String mensagem){

  String url = "/v2/pushes";
  String messagebody = "{\"type\": \"note\", \"title\": \"SaMCAT\", \"body\": \""+mensagem+"\"}\r\n";
  Serial.print("requesting URL: ");
  Serial.println(url);
  //send a simple note
  client.print(String("POST ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Authorization: Bearer " + PushBulletAPIKEY + "\r\n" +
               "Content-Type: application/json\r\n" +
               "Content-Length: " +
               String(messagebody.length()) + "\r\n\r\n");
  client.print(messagebody);
  //send a link 

  Serial.println("request sent");
  //print the response
}



void setup() {
  Serial.begin(115200);
  dht.begin();

  // Inicializa variáveis de saída como saídas no Node
  pinMode(buzzerPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(ldrPin, INPUT);
  pinMode(soundPin, INPUT);
  pinMode(firePin, INPUT);
  
  // Seta saídas como LOW
  digitalWrite(ledPin, LOW);
  digitalWrite(buzzerPin, LOW);

  // Conecta ao Wi-Fi com SSID e Senha
  Serial.print("Conectando-se a ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print do endereço de IP local e inicializa o Servidor Web
  Serial.println("");
  Serial.println("Wi-Fi conectado.");
  Serial.println("Endereço de IP: ");
  Serial.println(WiFi.localIP());
  server.begin();

  Serial.println("\n\n INICIALIZANDO PUSHBULLET \n\n");

WiFiClientSecure client;
  Serial.print("Conectando - ");
  Serial.println(host);
if (!client.connect(host, httpsPort)) {
    Serial.println("Falhou ao conectar ao pushbullet");
    return;
  }

  pushBullet(client, "SaMCAT está operacional");

  Serial.println("\n\n\n\n INICIALIZANDO SaMCAT \n\n");
}

//Informações sobre o Servidor Web
void loop(){
  WiFiClient client = server.available();   // Espera de novos Clientes

  if (client) {                             // Se um novo Cliente se conecta,
    Serial.println("Novo Cliente.");        // imprime uma mensagem no Serial
    String currentLine = "";                // cria uma String que guarda dados do Cliente ingressante
    while (client.connected()) {            // loop enquanto o Cliente está conectado
      if (client.available()) {             // se há bytes a serem lidos do cliente,
        char c = client.read();             // lê-se um byte e
        Serial.write(c);                    // o imprime no Serial
        header += c;
        if (c == '\n') {                    // se o byte é um caractere de nova linha
          // e se a linha atual está em branco, você terá duas novas linhas de caracteres em seguida.
          // fim da requisição Cliente HTTP, enviando a resposta:
          if (currentLine.length() == 0) {
            // headers HTTP sempre começar com um código de resposta (e.g. HTTP/1.1 200 OK)
            // e o tipo de conteúdo, assim o Cliente saberá o que está por vir. Depois, uma linha em branco:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

           
            if (header.indexOf("GET /D1/on") >= 0) {
              Serial.println("Sensores ligados");
              estadoSensores = true;
            } else if (header.indexOf("GET /D1/off") >= 0) {
              Serial.println("Sensores desligados");
              estadoSensores = false;
            }
            
         
            
            // Mostra a página HTML
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta http-equiv=\"refresh\" charset=\"UTF-8\" content=\"3\" name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS usado para o estilo dos botões de ON e OFF. Pode ser mudado para alterar o visual da página.
            client.println("<style>html { font-family: Arial; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #42b5b1; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");
            
            
            // Título da Página Web
            client.println("<body><h1>SaMCAT</h1>");
            client.println("<body><h2>Sistema de Monitoramento de Condições Ambientais de Trabalho</h2>");
             
            client.println("<h3><br />Estado de luminosidade</h3>");
            client.println("<p>"+lumi(estadoSensores)+"</p>");

            float h = dht.readHumidity();
            float t = dht.readTemperature() - 2; 
            client.println("<h3><br />Temperatura e umidade</h3>");
            
            if(estadoSensores == true){
              client.println("<p>Temperatura: "+String(t)+"C - Umidade: "+String(h)+"%</p>");
              client.println("<p>"+umiTemp(t,h,estadoSensores)+"</p>");
            } else {
              client.println("<p>Sensor desativado</p>");
            }

            client.println("<h3><br />Níveis sonoros</h3>");
            client.println("<p>"+som(estadoSensores)+"</p>");           

            client.println("<h3><br />Detecção de incêndio</h3>");
            client.println("<p>"+fire(estadoSensores)+"</p>");

            if (estadoSensores == false) {
              client.println("<p><a href=\"/D1/on\"><button class=\"button\">ATIVAR SENSORES</button></a></p>");
            } else {
              client.println("<p><a href=\"/D1/off\"><button class=\"button button2\">DESATIVAR SENSORES</button></a></p>");
            } 
            
            client.println("</body></html>");

            WiFiClientSecure client;
            Serial.print("Conectado - ");
            Serial.println(host);
            if (!client.connect(host, httpsPort)) {
            Serial.println("Falhou ao conectar ao pushbullet");
            }
            pushBullet(client, "Telemetria: "+lumi(estadoSensores)+"-- "+umiTemp(t,h,estadoSensores)+"-- "+som(estadoSensores)+"-- "+fire(estadoSensores));
            

            // A resposta HTTP termina com outra linha em branco
            client.println();
            // Sai do loop
            break;
          } else { // se há uma nova linha, limpa a linha atual
            currentLine = "";
          }
        } else if (c != '\r') {  // se há retorno de qualquer coisa que não seja caractere,
          currentLine += c;      // adiciona-o ao fim da linha atual.
        }
      }
    }
    // Limpa a variável header
    header = "";
    // Termina conexão.
    client.stop();
    Serial.println("Cliente desconectado.");
    Serial.println("");
  }
}
