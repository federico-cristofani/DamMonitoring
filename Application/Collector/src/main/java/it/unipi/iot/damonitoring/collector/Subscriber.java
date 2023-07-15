package it.unipi.iot.damonitoring.collector;

import com.fasterxml.jackson.databind.ObjectMapper;
import it.unipi.iot.damonitoring.DataManager;
import it.unipi.iot.damonitoring.collector.senml.Measurement;
import it.unipi.iot.damonitoring.collector.senml.SensorRecord;
import it.unipi.iot.damonitoring.entities.FlowRate;
import it.unipi.iot.damonitoring.exceptions.PersistenceException;
import org.eclipse.paho.client.mqttv3.*;
import org.eclipse.paho.client.mqttv3.persist.MqttDefaultFilePersistence;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.IOException;
import java.io.InputStream;
import java.time.LocalDateTime;
import java.time.OffsetDateTime;
import java.time.temporal.ChronoUnit;
import java.util.Date;
import java.util.HashMap;
import java.util.Properties;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;


public class Subscriber implements MqttCallback {
    private static final Logger logger = LoggerFactory.getLogger(Subscriber.class);
    private static final String CONFIG_FILE = "/collector.properties";

    /* Simulation only  */
    private static final String SIMULATION = "simulation";
    private static final float MAX_INFLOW = 80f;
    private static Float inflowGate = null;


    /* Private fields */
    private ScheduledExecutorService scheduler;
    private final MqttClient client;
    private final HashMap<String, Date> lastRecord;


    /* Configuration parameters */
    private final Integer reconnectionDelay;
    private final Integer reconnectionTimeout;
    private final TimeUnit unitOfTime;
    private final String[] topics;
    private final Integer minPeriod;

    public Subscriber() throws MqttException {

        try(InputStream configFileStream = Subscriber.class.getResourceAsStream(CONFIG_FILE)){

            logger.info("Initialization ...");
            Properties properties = new Properties();
            properties.load(configFileStream);

            // Load parameters
            reconnectionDelay = Integer.parseInt(properties.getProperty("reconnectionDelay"));
            reconnectionTimeout = Integer.parseInt(properties.getProperty("reconnectionTimeout"));
            unitOfTime = TimeUnit.valueOf(properties.getProperty("reconnectionTimeUnit"));
            minPeriod = Integer.parseInt(properties.getProperty("minPeriod"));

            // Open and configure MQTT connect
            client = new MqttClient(
                    properties.getProperty("brokerURI"),
                    properties.getProperty("clientID"),
                    new MqttDefaultFilePersistence(".mqtt")
            );
            client.setCallback(this);
            client.connect();

            // Subscribe topics
            topics = properties.getProperty("topics").split(",");
            for(String topic: topics)
                client.subscribe(topic);

            logger.info("Initialization completed");

        }
        catch (IOException ex){
            throw new RuntimeException(ex);
        }

        lastRecord = new HashMap<>();
    }

    public void reconnect(){
        try {
            if(!client.isConnected()){
                logger.info("Trying reconnecting ... ");
                client.reconnect();
            }
            else {
                logger.info("Reconnected");
                for(String topic: topics)
                    client.subscribe(topic);

                scheduler.shutdown();
            }
        } catch (MqttException ex) { logger.info("Disconnected"); }
    }

    public void disconnect() {
        if(client.isConnected()){
            try {
                client.disconnect();
                logger.info("Disconnection completed");
            } catch (MqttException ex) {
                logger.error("Error during disconnection, force closing");
            }
        }
    }

    @Override
    public void connectionLost(Throwable throwable) {
        scheduler = Executors.newSingleThreadScheduledExecutor();
        logger.error("Disconnected from broker");
        scheduler.scheduleAtFixedRate(
                this::reconnect,
                reconnectionDelay, reconnectionTimeout, unitOfTime
        );
    }

    @Override
    public void messageArrived(String topic, MqttMessage mqttMessage) {
        try {
            ObjectMapper mapper = new ObjectMapper();
            SensorRecord record = mapper.readValue(mqttMessage.getPayload(), SensorRecord.class);
            Date minNextTime = Date.from(LocalDateTime
                    .now()
                    .minus(minPeriod, ChronoUnit.SECONDS)
                    .toInstant(OffsetDateTime.now().getOffset()));

            // For each record (actually only one attended)
            for(Measurement measurement : record.getMeasurements()){

                logger.info("New measurement: {}", measurement);

                // Retrieve data from record
                String baseName = record.getBaseName();
                String name = measurement.getName();
                Float value = measurement.getValue();

                // Discard messages arrived roughly at the same time
                if(lastRecord.containsKey(baseName + name) && minNextTime.before(lastRecord.get(baseName + name))){
                    logger.warn("Measurement discarded (too fast): {}", measurement);
                    continue;
                }

                // Save data on database
                switch (topic){
                    case "flow-rate":
                        // Control unit of measurement
                        if(!record.getBaseUnit().equals("m3/s")){
                            throw new RuntimeException("Unit of measurement not supported");
                        }
                        // Store measurement
                        FlowRate flowRate = DataManager.getInstance().recordFlowRate(name, value);
                        lastRecord.put(baseName + name, flowRate.getTimestamp());

                        //Simulation only
                        simulate(flowRate);
                        break;
                    case "water-level":
                        // Control unit of measurement
                        if(!record.getBaseUnit().equals("m")){
                            throw new RuntimeException("Unit of measurement not supported");
                        }
                        // Store measurement
                        DataManager.getInstance().recordWaterLevel(value);
                        break;
                    default:
                        throw new RuntimeException("Message on unhandled topic");
                }
            }
        } catch (IOException | RuntimeException ex) {
            logger.error(String.format("Malformed record (%s): %s\n", ex.getMessage(),
                    new String(mqttMessage.getPayload())));
        } catch (PersistenceException ex) {
            logger.error("Data access: {}", ex.getMessage());
        }
    }

    @Override
    public void deliveryComplete(IMqttDeliveryToken iMqttDeliveryToken) { logger.info("Simulation message sent"); }

    private void simulate(FlowRate flowRate) {
        try {
            if(flowRate.getFlowChannel().equalsIgnoreCase("inflow")) {
                float newValue = Math.round(flowRate.getValue() / MAX_INFLOW * 100);
                if(inflowGate == null || inflowGate != newValue){
                    client.publish(SIMULATION, new MqttMessage(new ObjectMapper().createObjectNode()
                            .put("gate-inflow", (int)newValue).toString().getBytes()));
                    inflowGate = newValue;
                }
            }
        }
        catch (MqttException ex){
            logger.warn("Simulation: {}", ex.getMessage());
        }
    }
}
