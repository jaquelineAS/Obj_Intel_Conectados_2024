#include <WiFi.h>           // Biblioteca para conectar ao Wi-Fi
#include <PubSubClient.h>   // Biblioteca para conectar ao servidor MQTT

// Configurações da rede Wi-Fi
const char* ssid = "FABIANA";       // Nome da rede Wi-Fi
const char* password = "5341aeiou"; // Senha da rede Wi-Fi

// Configurações do servidor MQTT
const char* mqtt_server = "192.168.100.13"; // IP do servidor MQTT
const int mqtt_port = 1883;                // Porta do servidor MQTT
const char* topic = "sensores/luminosidade"; // Tópico onde as mensagens serão publicadas

WiFiClient espClient;        // Criação de um cliente Wi-Fi
PubSubClient client(espClient); // Criação de um cliente MQTT usando o cliente Wi-Fi

int ldrPin = A0;     // Pino onde o LDR está conectado
int ldrVal = 0;      // Variável para armazenar o valor lido do LDR
int ledPin = 2;      // Pino onde o LED está conectado

// Constantes usadas para calcular a luminosidade
const float GAMMA = 0.7;
const float RL10 = 50;

// Função para configurar a conexão Wi-Fi
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password); // Inicia a conexão com a rede Wi-Fi

  // Aguarda até que a conexão Wi-Fi seja estabelecida
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Imprime informações sobre a conexão Wi-Fi estabelecida
  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

// Função para reconectar ao servidor MQTT, caso a conexão seja perdida
void reconnect() {
  // Loop até conseguir conectar
  while (!client.connected()) {
    Serial.print("Tentando conexão MQTT...");
    // Tenta conectar ao servidor MQTT
    if (client.connect("ESP32Client")) {
      Serial.println("conectado");
    } else {
      Serial.print("falhou, rc=");
      Serial.print(client.state());
      Serial.println(" tentando novamente em 5 segundos");
      delay(5000); // Aguarda 5 segundos antes de tentar novamente
    }
  }
}

// Função de configuração, executada uma vez no início
void setup() {
  Serial.begin(9600);        // Inicializa a comunicação serial a 9600 bps
  pinMode(ledPin, OUTPUT);   // Configura o pino do LED como saída
  pinMode(ldrPin, INPUT);    // Configura o pino do LDR como entrada

  setup_wifi();              // Chama a função para conectar ao Wi-Fi
  client.setServer(mqtt_server, mqtt_port); // Configura o servidor MQTT
}

// Função principal, executada repetidamente
void loop() {
  // Verifica a conexão ao servidor MQTT e reconecta se necessário
  if (!client.connected()) {
    reconnect();
  }
  client.loop(); // Mantém a comunicação com o servidor MQTT

  ldrVal = analogRead(ldrPin); // Lê o valor do LDR
  // Calcula a voltagem, resistência e luminosidade
  float voltage = ldrVal / 1024.0 * 5.0;
  float resistance = 2000.0 * voltage / (1.0 - voltage / 5.0);
  float lux = pow(RL10 * 1e3 * pow(10, GAMMA) / resistance, (1.0 / GAMMA));

  // Imprime os valores lidos e calculados na porta serial
  Serial.print("LDR Value: ");
  Serial.println(ldrVal);
  Serial.print("Voltage: ");
  Serial.println(voltage);
  Serial.print("Resistance: ");
  Serial.println(resistance);
  Serial.print("Lux: ");
  Serial.println(lux);

  // Acende ou apaga o LED baseado na luminosidade medida
  if (lux < 200) {
    digitalWrite(ledPin, HIGH); // Acende o LED se a luminosidade for menor que 200 lux
  } else {
    digitalWrite(ledPin, LOW);  // Apaga o LED se a luminosidade for maior ou igual a 200 lux
  }

  // Prepara a mensagem para ser enviada ao servidor MQTT
  char msg[50];
  snprintf(msg, 50, "Luminosidade: %.2f lux", lux);
  client.publish(topic, msg); // Publica a mensagem no tópico especificado

  delay(1000); // Aguarda 1 segundo antes de repetir o loop
}