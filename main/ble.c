#include "ble.h"
#include "esp_nimble_hci.h"
#include "esp_system.h"
#include "host/ble_gatt.h"
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "esp_pm.h"
#include "host/ble_gap.h"

#define CONN_ITVL_MIN_MS   500   // 500ms mínimo
#define CONN_ITVL_MAX_MS  1000   // 1000ms máximo  
#define CONN_LATENCY         4   // puede saltear 4 eventos
#define CONN_TIMEOUT_MS   6000   // supervision timeout

static const char *TAG = "BLE";
uint8_t ble_addr_type;
static const char *manuf_name = MANUFACTURER;
static const char *model_num = MODEL;
void ble_app_advertise(void);

static int ble_data_cb(uint16_t conn_handle, uint16_t attr_handle,
                        struct ble_gatt_access_ctxt *ctxt, void *arg);

static const struct ble_gatt_svc_def gatt_svcs[] = {
    {                                                   // Service: Device Information
     .type = BLE_GATT_SVC_TYPE_PRIMARY,
     .uuid = BLE_UUID16_DECLARE(GATT_DEVICE_INFO_UUID),
     .characteristics =
         (struct ble_gatt_chr_def[]){
             {
              .uuid = BLE_UUID16_DECLARE(GATT_MANUFACTURER_NAME_UUID),
              .access_cb = ble_data_cb,
              .flags = BLE_GATT_CHR_F_READ},
             {
              .uuid = BLE_UUID16_DECLARE(GATT_FIRMWARE_UUID),
              .access_cb = ble_data_cb,
              .flags = BLE_GATT_CHR_F_READ},
             {
              .uuid = BLE_UUID16_DECLARE(GATT_MODEL_NUMBER_UUID),
              .access_cb = ble_data_cb,
              .flags = BLE_GATT_CHR_F_READ},
             {
                 .uuid = 0, /* No more characteristics in this service */
             },
         }},
    {                                                       // Service: CRC 
     .type = BLE_GATT_SVC_TYPE_PRIMARY,
     .uuid = BLE_UUID16_DECLARE(GATT_CSC_SERVICE_UUID),
     .characteristics =
         (struct ble_gatt_chr_def[]){
             {/* Characteristic: * data */
              .uuid = BLE_UUID16_DECLARE(GATT_CSC_MEASUREMENT_CHAR_UUID),
              .flags = BLE_GATT_CHR_F_NOTIFY,
              .val_handle = &handle.cscs,
              .access_cb = ble_data_cb},
             {/* Characteristic: info */
              .uuid = BLE_UUID16_DECLARE(GATT_CSC_FEATURE_CHAR_UUID),
              .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
              .access_cb = ble_data_cb},
             {
                 .uuid = 0, /* No more characteristics in this service */
             },
         }},                                           
    {
        .type = BLE_GATT_SVC_TYPE_END // No more services
    },
};

void ble_notify_new_data(uint32_t wheel_revs, uint16_t wheel_time,
                           uint32_t crank_revs, uint16_t crank_time){
  if (!connected) {
        return; // no hay conexión activa, no hay mbuf que crear
  }
  // Construir paquete de medición
    // Formato: Flags (1) + Wheel Data (6) + Crank Data (6) = 13 bytes mínimo
    uint8_t measurement[13];
    
    // Flags: Bit 0 = Wheel present, Bit 1 = Crank present
    #define CSCS_WHEEL_DATA_PRESENT    (1 << 0)
    #define CSCS_CRANK_DATA_PRESENT    (1 << 1)
    measurement[0] = CSCS_WHEEL_DATA_PRESENT | CSCS_CRANK_DATA_PRESENT;
    
    // Cumulative Wheel Revolutions (4 bytes, little-endian)
    measurement[1] = wheel_revs & 0xFF;
    measurement[2] = (wheel_revs >> 8) & 0xFF;
    measurement[3] = (wheel_revs >> 16) & 0xFF;
    measurement[4] = (wheel_revs >> 24) & 0xFF;
    
    // Last Wheel Event Time (2 bytes, little-endian, 1/100s)
    measurement[5] = wheel_time & 0xFF;
    measurement[6] = (wheel_time >> 8) & 0xFF;
    
    // Cumulative Crank Revolutions (4 bytes, little-endian)
    measurement[7] = crank_revs & 0xFF;
    measurement[8] = (crank_revs >> 8) & 0xFF;
    measurement[9] = (crank_revs >> 16) & 0xFF;
    measurement[10] = (crank_revs >> 24) & 0xFF;
    
    // Last Crank Event Time (2 bytes, little-endian, 1/1024s)
    measurement[11] = crank_time & 0xFF;
    measurement[12] = (crank_time >> 8) & 0xFF;
    
	struct os_mbuf *om = ble_hs_mbuf_from_flat(measurement, 13);
  if (om == NULL) {
      return; // fallo la alocación, nada que liberar
  }
  int rc = ble_gatts_notify_custom(handle.conn, handle.cscs, om);	
  if (rc != 0) {
    ESP_LOGW(TAG,"hubo un error al notificar del tipo: %d", rc);
      os_mbuf_free_chain(om); // liberar si no fue consumido
  }
}

static int ble_data_cb(uint16_t conn_handle, uint16_t attr_handle,
                        struct ble_gatt_access_ctxt *ctxt, void *arg) {
  ESP_LOGI(TAG, "%04X callback", ble_uuid_u16(ctxt->chr->uuid));
  int rc = 1;
  switch (ble_uuid_u16(ctxt->chr->uuid)) {
  case GATT_MODEL_NUMBER_UUID:
    rc = os_mbuf_append(ctxt->om, model_num, strlen(model_num));
    break;
  case GATT_MANUFACTURER_NAME_UUID:
    rc = os_mbuf_append(ctxt->om, manuf_name, strlen(manuf_name));
    break;
  case GATT_FIRMWARE_UUID:
    rc = os_mbuf_append(ctxt->om, version, strlen(version));
    break;
  case GATT_CSC_FEATURE_CHAR_UUID:
	  {
	  	if (ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR) {
			    ESP_LOGI(TAG, "");
      } else if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
          ESP_LOGI(TAG, "");
          rc = 0;
      } else {
          rc = 0;
          ESP_LOGW(TAG,"NEWDATA ni READ ni WRITE, en que quedamos??\n");
      }
      break; 
    }
  default:
    break;
  }
  if (rc != 0){
    ESP_LOGW(TAG,"BLE_ATT_ERR_INSUFFICIENT_RES");
  }
  return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
}

static void request_low_power_conn_params(uint16_t conn_handle) {
    struct ble_gap_upd_params params = {
        .itvl_min      = CONN_ITVL_MIN_MS * 8 / 5,  // unidades de 1.25ms
        .itvl_max      = CONN_ITVL_MAX_MS * 8 / 5,
        .latency       = CONN_LATENCY,
        .supervision_timeout = CONN_TIMEOUT_MS / 10, // unidades de 10ms
        .min_ce_len    = 0,
        .max_ce_len    = 0,
    };
    ble_gap_update_params(conn_handle, &params);
}

static int ble_gap_event(struct ble_gap_event *event, void *arg) {
  switch (event->type) {
  // Advertise if connected
  case BLE_GAP_EVENT_CONNECT:
  connected = true;
    printf("BLE GAP EVENT CONNECT %s\n", event->connect.status == 0 ? "OK!" : "FAILED!");
    ESP_LOGI(TAG, "BLE GAP EVENT CONNECT %s",
             event->connect.status == 0 ? "OK!" : "FAILED!");
    if (event->connect.status == 0) {
      request_low_power_conn_params(handle.conn);  // <-- agregá esto
    } else {
      ble_app_advertise();
    }
    handle.conn = event->connect.conn_handle; 
    break;
  // Advertise if disconnect
  case BLE_GAP_EVENT_DISCONNECT:
    ESP_LOGI(TAG, "BLE GAP EVENT DISCONNECT");
    connected = false;
    ble_app_advertise();
    break;
  // Advertise again after completion of the event
  case BLE_GAP_EVENT_ADV_COMPLETE:
    ESP_LOGI(TAG, "BLE GAP EVENT");
    ble_app_advertise();
    break;
  case BLE_GAP_EVENT_SUBSCRIBE:
    ESP_LOGI(TAG, "subscribe event. expected handler: %d, attr_handle=%d, cur=%d\n",
             handle.cscs, event->subscribe.attr_handle, event->subscribe.cur_notify);
    break;
  case BLE_GAP_EVENT_MTU:
    ESP_LOGI(TAG,"MTU = %d", event->mtu.value);
    break;
  default:
    // ESP_LOGW("GAP", "UNSPECTED BLE GAP EVENT: [%d]", event->type);
    break;
  }
  return 0;
}

void ble_app_advertise(void) // Define the BLE connection
{
  // GAP - device name definition
  struct ble_hs_adv_fields fields;
  const char *device_name;
  memset(&fields, 0, sizeof(fields));
  device_name = ble_svc_gap_device_name(); // Read the BLE device name
  fields.name = (uint8_t *)device_name;
  fields.name_len = strlen(device_name);
  fields.name_is_complete = 1;
  
  fields.appearance = CSCS_APPEARANCE;
  fields.appearance_is_present = 1;

  ble_uuid16_t uuids16[] = { BLE_UUID16_INIT(GATT_CSC_SERVICE_UUID) };
  fields.uuids16 = (ble_uuid16_t *)uuids16;
  fields.num_uuids16 = 1;
  fields.uuids16_is_complete = 1;


  ble_gap_adv_set_fields(&fields);

  // GAP - device connectivity definition
  struct ble_gap_adv_params adv_params;
  memset(&adv_params, 0, sizeof(adv_params));
  adv_params.conn_mode =
      BLE_GAP_CONN_MODE_UND; // BLE_GAP_CONN_MODE_DIR; // connectable or
                             // non-connectable
  adv_params.disc_mode =
      BLE_GAP_DISC_MODE_GEN; // discoverable or non-discoverable
  ble_gap_adv_start(ble_addr_type, NULL, BLE_HS_FOREVER, &adv_params,
                    ble_gap_event, NULL);
}

static void ble_app_on_sync(void) // The application
{
  ble_hs_id_infer_auto(
      0, &ble_addr_type); // Determines the best address type automatically
  ble_app_advertise();    // Define the BLE connection
}

static void host_task(void *param) {
  nimble_port_run(); // This function will return only when nimble_port_stop()
                     // is executed
  nimble_port_freertos_deinit();
}

void ble_init() {
  //nvs_flash_init(); // 1 - Initialize NVS flash using
  // esp_nimble_hci_init();                      // 2 - Initialize ESP
  // controller:  esp_nimble_hci_and_controller_init();
  nimble_port_init();
  // Configurar nombre de dispositivo
  const char *device_name = "ESP32 CSC Sensor";
  ble_svc_gap_device_name_set(device_name); // 4 - Initialize NimBLE configuration - server name
  ble_att_set_preferred_mtu(23);          // from 23 to BLE_ATT_MTU_MAX = 527
  ble_svc_gap_init();                      // 4 - Initialize NimBLE configuration - gap service
  ble_svc_gatt_init();                     // 4 - Initialize NimBLE configuration - gatt service
  ble_gatts_count_cfg(gatt_svcs);          // 4 - Initialize NimBLE configuration - config gatt services
  ble_gatts_add_svcs(gatt_svcs);           // 4 - Initialize NimBLE configuration - queues gatt services
  ble_hs_cfg.sync_cb = ble_app_on_sync;    // 5 - Initialize application
  nimble_port_freertos_init(host_task);    // 6 - Run the thread
}