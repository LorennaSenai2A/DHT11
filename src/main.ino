#include <WiFi.h> //Biblioteca para conexão Wifi
#include <PubSubClient.h> //Biblioteca para conexão MQTT
#include <DHT.h> //Biblioteca para módulo DHT11/DTH22

//Configurações para rede Wifi
const char* ssid = "Rede";
const char* password = "123456789";

//Configuração do Broker MQTT
const char* mqttServer = "broker.hivemq.com"; //Host do Broker MQTT publico
const int mqttPort = 1883; //Porta de Broker MQTT publico
const char* mqttTopic = "AlunaLoh/lerSensor"; //Topico onde os dados serão publicados

//Configuração do DHT22
#define DHTPIN 15 //Pino onde o sensor DHT22 está conectado
#define DHTTYPE DHT11 //Tipo sensor
DHT dht(DHTPIN, DHTTYPE); //Cria objeto 'dht' com o pino e tipo do sensor

const int LED_PIN = 2; //Pino do LED embutido no ESP32 (GPIO2)

//Criação dos objetos de rede e MQTT
WiFiClient espClient; //Objeto do cliente Wifi
PubSubClient client(espClient); //Objeto do client MQTT utilizando o cliente Wifi

unsigned long lastMsg = 0; //Armazena o tempo da última mensagem enviada
const long interval = 2000; //Intervalo entre publicações (2 segundos)

void setup_wifi() {
  delay(10);  // Pequeno atraso inicial
  Serial.println("Conectando ao WiFi...");
  WiFi.begin(ssid, password); // Inicia conexão Wi-Fi
  while (WiFi.status() != WL_CONNECTED) { // Aguarda até conectar
    delay(500); Serial.print(".");
  }
  Serial.println("\n✅ WiFi conectado. IP: " + WiFi.localIP().toString());
  digitalWrite(LED_PIN, HIGH); // Acende o LED para indicar conexão WiFi
}

// Função para reconectar ao Broker MQTT (Servidor), se desconectado.
void reconnect(){
  // ID Randomico
  String clientId = "Esp32_" + String(random(0xffff), HEX);
  while(!client.connected()){
    Serial.println("Tentando conectar ao MQTT...");
    if(client.connect(clientId.c_str())){ // Tenta fazer a conexao com o ID Esp32Client 
      Serial.println("Conectado ao Broker.");
      client.subscribe(mqttTopic, 0); // Inscrever-se no tópico com QoS 0
    }else{
      Serial.println("Falha ao conectar-se... Tentando novamente em 5s...");
      Serial.print("Código da Falha: ");
      Serial.println(client.state()); // Exibe codigo do erro
      delay(5000);  // Espera para tentar novamente
    }
  }
}

void setup(){
  Serial.begin(115200); //Inicia comunicação serial
  dht.begin(); //Inicia o sensor DHT22
  setup_wifi(); //Inicia e conecta ao wifi
  client.setServer(mqttServer, mqttPort); //Define o servidor MQTT e porta
  pinMode(LED_PIN, OUTPUT); //Configura o pino do LED como saída
}

void loop(){
  if(!client.connected()){ //Verifica se está conectado ao broker
  reconnect(); //Se não estiver, tenta reconectar
  }
  client.loop(); //matém a conexão MQTT ativa

 unsigned long now = millis(); //Pega o tempo atual
 if(now - lastMsg > interval){ //Verifica se já passou o intervalo de envio
  lastMsg = now; //Atualiza o tempo da última mensagem

//Pegar os valores de temperatura e umidade do sensor
float h = dht.readHumidity(); //Lê a umidade
float t = dht.readTemperature(); //Lê a temperatura

if(isnan(h) || isnan(t)){
  Serial.println("Erro ao ler Sensor DHT11");
  return; //Sai da função se houver erro
}

//Cria um variavel no formato JSON
String payload = "{";
payload += "\"temperatura\": " + String(t,1) + ","; //Um dígito decimal
payload += "\"umidade\": " + String(h,1); //Um dígito decimal
payload += "}";

Serial.print("Publicado: "); //Mostra o que será enviado
Serial.println((payload));

client.publish(mqttTopic, payload.c_str()); //Publica no topico definido
}
}





