#include "MQTT.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "mqtt_client.h"
#include <cJSON.h>
#include "HMI.h"
#include <math.h>
#include "esp_timer.h"
#include "../../sclib/alarms/alarms.h"

#ifndef CONFIG_MQTT_LWT_RETAIN
#define CONFIG_MQTT_LWT_RETAIN 0
#endif

#ifndef CONFIG_MQTT_BIRTH_RETAIN
#define CONFIG_MQTT_BIRTH_RETAIN 0
#endif

#ifndef CONFIG_MQTT_DISABLE_CLEAN_SESSION
#define CONFIG_MQTT_DISABLE_CLEAN_SESSION 0
#endif

#ifndef CONFIG_MQTT_HOMEASSISTANT
#define CONFIG_MQTT_HOMEASSISTANT 0
#endif

#if CONFIG_MQTT_HOMEASSISTANT
#include "home_assistant/home_assistant.h"
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

static const char *TAG = "MQTT";
static char command_topic[256];
static char feedback_topic[256];

int64_t last_update_time = 0;

static size_t array_length = ARRAY_SIZE(id);

static esp_mqtt_client_config_t mqtt_cfg = {
    .broker.address.uri = CONFIG_MQTT_BROKER_URL,
    .broker.address.port = CONFIG_MQTT_BROKER_PORT,
    .credentials.username = CONFIG_MQTT_USERNAME,
    .credentials.authentication.password = CONFIG_MQTT_PASSWORD,
    .session.keepalive = CONFIG_MQTT_KEEP_ALIVE,
    .session.disable_clean_session = CONFIG_MQTT_DISABLE_CLEAN_SESSION,
    .network.reconnect_timeout_ms = CONFIG_MQTT_RECONNECT_TIMEOUT * 1000,
    .session.last_will.topic = CONFIG_MQTT_LWT_TOPIC,
    .session.last_will.qos = CONFIG_MQTT_LWT_QOS,
    .session.last_will.retain = CONFIG_MQTT_LWT_RETAIN,
};
static esp_mqtt_client_handle_t client;


/*
    Funzione di ping.
    Viene inviato un messaggio di ping al broker.
*/
void mqtt_ping() {
    // Crea il payload come stringa
    char payload[64]; // Assicurati che il buffer sia abbastanza grande
    snprintf(payload, sizeof(payload), "{\"id\": %d, \"value\": %d, \"deviceId\": \"%s\"}", 0, 3, CONFIG_MQTT_CLIENT_ID);

    // Pubblica il payload tramite MQTT
    esp_mqtt_client_publish(client, feedback_topic, payload, 0, CONFIG_MQTT_BIRTH_QOS, 0);
}

/*
    Funzione di richiesta dell'offset UTC.
    Viene inviato un messaggio di risposta alla richiesta del time UTC al broker.
*/
void mqtt_utc_offset() {
    // Crea il payload come stringa
    char payload[128]; // Assicurati che il buffer sia abbastanza grande
    int64_t current_time = esp_timer_get_time() / 1000; // Ottieni il timestamp corrente in millisecondi
    snprintf(payload, sizeof(payload), "{\"id\": %d, \"value\": %d, \"utc_offset\": %lld, \"deviceId\": \"%s\"}", 0, 5, current_time, CONFIG_MQTT_CLIENT_ID);

    // Pubblica il payload tramite MQTT
    esp_mqtt_client_publish(client, feedback_topic, payload, 0, CONFIG_MQTT_BIRTH_QOS, 0);
};

/*
    Funzione di receive di una system call.
    Se l'ID è 0 allora si tratta di una system call.
    Per esempio la system call 1 è la richiesta di un reboot.
    la system call 2 è la richiesta di refresh di tutte le variabili. 
    la system call 3 è la richiesta di ping.
*/

static void mqtt_system_call(uint8_t call_id) {
    switch (call_id) {
        case 1:
            ESP_LOGI(TAG, "Rebooting...");
            esp_restart();
            break;
        case 2:
            ESP_LOGI(TAG, "Refreshing all variables...");
            mqtt_updHMI(true);
            break;
        case 3:
            ESP_LOGI(TAG, "Pinging...");
            mqtt_ping();
            break;            
        case 4:
            ESP_LOGI(TAG, "Ack received...");
            alarms_ack();
        break;
        case 5:
            ESP_LOGI(TAG, "Set date and time...");
            mqtt_utc_offset();
        break;
        default:
            ESP_LOGE(TAG, "Unknown system call");
            break;
    }
}

/*
    Funzione di receive.
    Le variabili HMI sono organizzate su 3 arrays:
    id, type, pointer
    Quando ricevi un command:
    {"id":xxx, "value":yyy}
    Vai a cercare l'id nel rispettivo array. 
    La sua posizione (il cursore i della for) verrà utilizzata per andare ad incrociare negli altri due array
    il relativo tipo ed il puntatore alla variabile nella memoria dell'ESP32.
    Questo perché l'array pointer è un array di puntatori generici, e quindi devi fare il cast col tipo corretto
    della variabile che puntano prima di eseguire la scrittura.
*/
static void mqtt_receive(esp_mqtt_event_handle_t event){
  ///va a leggere gli array e scrive i le variabili
    cJSON *root = cJSON_Parse(event->data);
    if (root == NULL) {
        ESP_LOGE(TAG, "Error parsing JSON\n");
        return;
    }

    cJSON *id_json = cJSON_GetObjectItem(root, "id");
    cJSON *value_json = cJSON_GetObjectItem(root, "value");

    if (!cJSON_IsNumber(id_json) || !cJSON_IsNumber(value_json)) {
        ESP_LOGE(TAG, "Invalid JSON format\n");
        cJSON_Delete(root);
        return;
    }
    
    
    uint64_t id_value = id_json->valueint;
    double value = value_json->valuedouble;
    
    ESP_LOGI(TAG, "Received ID: %llu", id_value);
    ESP_LOGI(TAG, "Received Value: %f", value);

    // System call: ID = 0
    if (id_value == 0) {
        mqtt_system_call((uint8_t)value);
        cJSON_Delete(root);
        return;
    }

    // HMI variable update
    for (int i = 0; i < ARRAY_SIZE(id); i++) {
        if (id[i] == id_value) {
            switch (type[i]) {
                case REAL:
                    *(float *)HMI_pointer[i] = (float)value;
                    *(float *)PLC_pointer[i] = (float)value;
                    break;
                case INT:
                    *(int *)HMI_pointer[i] = (int)value;
                    *(int *)PLC_pointer[i] = (int)value;
                    break;
                case BOOL:
                    *(int *)HMI_pointer[i] = (value != 0);
                    *(int *)PLC_pointer[i] = (value != 0);
                    break;
                case STRING:
                    value_json = cJSON_GetObjectItem(root, "value");
                    if (cJSON_IsString(value_json)) {
                        strncpy((char *)HMI_pointer[i], value_json->valuestring, 256);
                        strncpy((char *)PLC_pointer[i], value_json->valuestring, 256);
                        //ESP_LOGI(TAG, "Updated STRING value for ID %llu: %s", id[i], value_json->valuestring);
                    } else {
                        ESP_LOGE(TAG, "Invalid STRING value for ID %llu", id[i]);
                    }
                    break;
                case TIMESTAMP:
                    value_json = cJSON_GetObjectItem(root, "value");
                    if (cJSON_IsNumber(value_json)) {
                        *(time_t *)HMI_pointer[i] = (time_t)value_json->valuedouble;
                        *(time_t *)PLC_pointer[i] = (time_t)value_json->valuedouble;
                        //ESP_LOGI(TAG, "Updated TIMESTAMP value for ID %llu: %lld", id[i], (int64_t)value_json->valuedouble);
                    } else {
                        ESP_LOGE(TAG, "Invalid TIMESTAMP value for ID %llu", id[i]);
                    }
                    break;
                default:
                    ESP_LOGE(TAG, "Unknown type\n");
                    break;
            }
            break;
        }
    }

    cJSON_Delete(root);
}

//Event handler che va a gestire gli eventi legati all'MQTT
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    //Compongo il command topic: /CONFIG_MQTT_COMMAND_TOPIC/CONFIG_MQTT_CLIENT_ID
    snprintf(command_topic, sizeof(command_topic), "%s/%s", CONFIG_MQTT_COMMAND_TOPIC, CONFIG_MQTT_CLIENT_ID);
    //Compongo il feedback topic: /CONFIG_MQTT_FEEDBACK_TOPIC/CONFIG_MQTT_CLIENT_ID
    snprintf(feedback_topic, sizeof(feedback_topic), "%s/%s", CONFIG_MQTT_FEEDBACK_TOPIC, CONFIG_MQTT_CLIENT_ID);
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            // ALLA CONNESSIONE, ESEGUE LE SUBSCRIPTIONS
            msg_id = esp_mqtt_client_subscribe(client, command_topic, CONFIG_MQTT_QOS);
            msg_id = esp_mqtt_client_subscribe(client, CONFIG_MQTT_COMMAND_TOPIC_BROADCAST, CONFIG_MQTT_QOS);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
            // INVIA IL MESSAGGIO DI BIRTH
            char birth_message[64]; // Assicurati che il buffer sia abbastanza grande
            snprintf(birth_message, sizeof(birth_message), "{\"deviceId\": \"%s\"}", CONFIG_MQTT_CLIENT_ID);
            msg_id = esp_mqtt_client_publish(client, CONFIG_MQTT_BIRTH_TOPIC, birth_message, 0, CONFIG_MQTT_BIRTH_QOS, CONFIG_MQTT_BIRTH_RETAIN);
            ESP_LOGI(TAG, "sent birth message, msg_id=%d", msg_id);
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);
            //AGGIORNAMENTO DEI VALORI SULLA RECEZIONE DEI COMANDI DA HMI
            mqtt_receive(event);
            break;
        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }
}

// Funzione di configurazione della sessione MQTT
void mqtt_setup(){
    // Crea il lwt_message come stringa
    char lwt_message[64]; // Assicurati che il buffer sia abbastanza grande
    snprintf(lwt_message, sizeof(lwt_message), "{\"deviceId\": \"%s\"}", CONFIG_MQTT_CLIENT_ID);
    // aggiorna mqtt_cfg con il messaggio lwt
    mqtt_cfg.session.last_will.msg = lwt_message;
    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
#if CONFIG_MQTT_HOMEASSISTANT
    home_assistant_setup();
#endif
}

/*
    Funzione di aggiornamento dei valori sull'HMI.
    Al momento vengono spediti messaggi in json col seguente formato:
    {"id":xxx, "value":yyyy}
    Sto pensando di passare ad una serie di coppie <id,value> in CSV, per alleggerire:
    xxx,yyy
    zzz,aaa
*/
void mqtt_updHMI(bool force) {

    int64_t current_time = esp_timer_get_time(); // Ottieni il timestamp corrente in microsecondi

    // Controlla se sono passati almeno 250 ms (CONFIG_MQTT_REFRESH_INTERVAL microsecondi) dall'ultimo aggiornamento
    if (!force && (current_time - last_update_time < CONFIG_MQTT_REFRESH_INTERVAL)) {
        return; // Esci dalla funzione se non è necessario aggiornare
    }

    // Aggiorna il timestamp dell'ultimo aggiornamento
    last_update_time = current_time;

    for (size_t i = 0; i < array_length; i++) {
        switch (type[i]) {
            case REAL: {
                // Controlla se il valore è cambiato o se è forzato
                if (*(float *)HMI_pointer[i] != *(float *)PLC_pointer[i] || force) {
                    cJSON *root = cJSON_CreateObject();
                    cJSON_AddNumberToObject(root, "id", id[i]);
                    *(float *)HMI_pointer[i] = *(float *)PLC_pointer[i];
                    cJSON_AddNumberToObject(root, "value", *(float *)PLC_pointer[i]);
                    char *payload = cJSON_Print(root);
                    esp_mqtt_client_publish(client, feedback_topic, payload, 0, CONFIG_MQTT_BIRTH_QOS, 0);
                    cJSON_Delete(root);
                    free(payload);
                }
                break;
            }
            case INT: {
                if (*(int *)HMI_pointer[i] != *(int *)PLC_pointer[i] || force) {
                    cJSON *root = cJSON_CreateObject();
                    cJSON_AddNumberToObject(root, "id", id[i]);
                    *(int *)HMI_pointer[i] = *(int *)PLC_pointer[i];
                    cJSON_AddNumberToObject(root, "value", (double)*(int *)PLC_pointer[i]);
                    char *payload = cJSON_Print(root);
                    esp_mqtt_client_publish(client, feedback_topic, payload, 0, CONFIG_MQTT_BIRTH_QOS, 0);
                    cJSON_Delete(root);
                    free(payload);
                }
                break;
            }
            case BOOL: {
                // Log per debug valore BOOL
                int plc_val = (*(int *)PLC_pointer[i]) & 1;
                int hmi_val = (*(int *)HMI_pointer[i]) & 1;
                bool bool_value = plc_val ? true : false;
                if (hmi_val != plc_val || force) {
                    cJSON *root = cJSON_CreateObject();
                    cJSON_AddNumberToObject(root, "id", id[i]);
                    *(int *)HMI_pointer[i] = plc_val;
                    cJSON_AddBoolToObject(root, "value", bool_value);
                    char *payload = cJSON_Print(root);
                    esp_mqtt_client_publish(client, feedback_topic, payload, 0, CONFIG_MQTT_BIRTH_QOS, 0);
                    cJSON_Delete(root);
                    free(payload);
                }
                break;
            }
            case STRING:
                if (strcmp((char *)HMI_pointer[i], (char *)PLC_pointer[i]) != 0 || force) {
                    cJSON *root = cJSON_CreateObject();
                    cJSON_AddNumberToObject(root, "id", id[i]);
                    strcpy((char *)HMI_pointer[i], (char *)PLC_pointer[i]); // Aggiorna la stringa
                    cJSON_AddStringToObject(root, "value", (char *)PLC_pointer[i]); // Aggiungi la stringa al JSON
                    char *payload = cJSON_Print(root);
                    esp_mqtt_client_publish(client, feedback_topic, payload, 0, CONFIG_MQTT_BIRTH_QOS, 0);
                    cJSON_Delete(root);
                    free(payload);
                }
                break;
            case TIMESTAMP:
                if (*(time_t *)HMI_pointer[i] != *(time_t *)PLC_pointer[i] || force) {
                    cJSON *root = cJSON_CreateObject();
                    cJSON_AddNumberToObject(root, "id", id[i]);
                    *(time_t *)HMI_pointer[i] = *(time_t *)PLC_pointer[i];
                    cJSON_AddNumberToObject(root, "value", (int64_t)*(time_t *)HMI_pointer[i]);
                    char *payload = cJSON_Print(root);
                    esp_mqtt_client_publish(client, feedback_topic, payload, 0, CONFIG_MQTT_BIRTH_QOS, 0);
                    cJSON_Delete(root);
                    free(payload);
                }
                break;
            default:
                ESP_LOGE(TAG, "Unknown type");
                return;
        }
        
#if CONFIG_MQTT_HOMEASSISTANT
        //home_assistant_update(id[i], type[i], ptrToValue);
#endif
    }
}