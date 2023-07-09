package it.unipi.iot.damonitoring.collector;

import it.unipi.iot.damonitoring.DataManager;
import it.unipi.iot.damonitoring.exceptions.PersistenceException;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class Collector {

    private static final Logger logger = LoggerFactory.getLogger(Collector.class);
    private static Subscriber subscriber;

    public static void main(String[] args) {

        try {
            // Eagle initialization
            DataManager.getInstance();

            // Start registration server
            RegistrationServer.getInstance().start();

            // Start MQTT client
            subscriber = new Subscriber();

            // Set shutdown handler
            Runtime.getRuntime().addShutdownHook(new Thread(Collector::shutdown, "shutdown"));
        }
        catch (PersistenceException | MqttException  ex){
            logger.error("Something went wrong: {}", ex.getMessage());
        }
    }

    public static void shutdown(){

        // Stop registration server
        RegistrationServer.getInstance().stop();

        // Close MQTT connection
        subscriber.disconnect();
        logger.info("MQTT disconnection completed");

        // Close database connection
        try {
            DataManager.close();
            logger.info("Database disconnection completed");
        } catch (PersistenceException ex) {
            ex.printStackTrace();
            logger.error("Database connection failed to close: {}", ex.getMessage());
        }
    }
}
