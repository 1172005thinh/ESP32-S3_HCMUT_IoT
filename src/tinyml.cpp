#include "tinyml.h"

// Globals, for the convenience of one-shot setup.
namespace
{
    tflite::ErrorReporter *error_reporter = nullptr;
    const tflite::Model *model = nullptr;
    tflite::MicroInterpreter *interpreter = nullptr;
    TfLiteTensor *input = nullptr;
    TfLiteTensor *output = nullptr;
    constexpr int kTensorArenaSize = 4 * 1024; // Adjust size based on your model
    uint8_t tensor_arena[kTensorArenaSize];
} // namespace

void setupTinyML()
{
    Serial.println("TensorFlow Lite Init....");
    static tflite::MicroErrorReporter micro_error_reporter;
    error_reporter = &micro_error_reporter;

    model = tflite::GetModel(dht_anomaly_model_tflite); // g_model_data is from model_data.h
    if (model->version() != TFLITE_SCHEMA_VERSION)
    {
        error_reporter->Report("Model provided is schema version %d, not equal to supported version %d.",
                               model->version(), TFLITE_SCHEMA_VERSION);
        return;
    }

    static tflite::AllOpsResolver resolver;
    static tflite::MicroInterpreter static_interpreter(
        model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
    interpreter = &static_interpreter;

    TfLiteStatus allocate_status = interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk)
    {
        error_reporter->Report("AllocateTensors() failed");
        return;
    }

    input = interpreter->input(0);
    output = interpreter->output(0);

    Serial.println("TensorFlow Lite Micro initialized on ESP32.");
}

void tiny_ml_task(void *pvParameters)
{
    AppContext* ctx = (AppContext*)pvParameters;
    setupTinyML();
    SensorData currentData;

    while (1)
    {
        // Wait for new sensor data (blocks until data arrives)
        if (xQueueReceive(xSensorDataQueue, &currentData, portMAX_DELAY) == pdPASS) {
            
            // Normalize inputs
            float norm_temp = (currentData.temperature - 0.0) / (50.0 - 0.0); 
            float norm_hum = (currentData.humidity - 0.0) / (100.0 - 0.0);

            // Prepare input data
            input->data.f[0] = norm_temp;
            input->data.f[1] = norm_hum;

            // Run inference
            TfLiteStatus invoke_status = interpreter->Invoke();
            if (invoke_status != kTfLiteOk)
            {
                error_reporter->Report("Invoke failed");
                continue;
            }

            // Get and process output
            float result = output->data.f[0];
            bool isAnomaly = (result > 0.75); // 75% threshold
            
            Serial.print("Inference result: ");
            Serial.print(result);
            Serial.print(" Anomaly: ");
            Serial.println(isAnomaly ? "YES" : "NO");

            // Send result to other tasks
            xQueueSend(xAnomalyQueueLCD, &isAnomaly, 0); 
            xQueueSend(xAnomalyQueueIOT, &isAnomaly, 0);
        }
    }
}