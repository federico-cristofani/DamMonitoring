package it.unipi.iot.damonitoring.controller.control;

import com.fasterxml.jackson.databind.ObjectMapper;
import it.unipi.iot.damonitoring.DataManager;
import it.unipi.iot.damonitoring.controller.coap.CoapManager;
import it.unipi.iot.damonitoring.controller.coap.exceptions.CoapException;
import it.unipi.iot.damonitoring.controller.control.enumerates.OperativeMode;
import it.unipi.iot.damonitoring.controller.control.exceptions.ControlException;
import it.unipi.iot.damonitoring.entities.Resource;
import it.unipi.iot.damonitoring.exceptions.PersistenceException;

import org.eclipse.paho.client.mqttv3.MqttClient;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.InputStream;
import java.util.Properties;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicReference;

public final class ControlLogic {

    private final static Logger logger = LoggerFactory.getLogger(ControlLogic.class);
    private static final String CONFIG_FILE = "/controller.properties";
    static final int FULL_OPEN = 100;
    static final int FULL_CLOSE = 0;

    /* Parameters */
    private static final AtomicReference<Float> maxThreshold;
    private static final AtomicReference<Float> minThreshold;
    private static final AtomicReference<Float> safeThreshold;
    private static final AtomicReference<OperativeMode> mode;


    /* Scheduled worker for automatic controller */
    private static final Integer taskPeriod;
    private static final TimeUnit taskUnitOfTime;
    private static final ScheduledExecutorService automaticControlUnit;


    /* Simulation */
    private static final String brokerURI;
    private static final String clientID;

    static {
        try (InputStream inputStream = ControlLogic.class.getResourceAsStream((CONFIG_FILE))){

            // Load config file
            Properties properties = new Properties();
            properties.load(inputStream);

            // Simulation only
            brokerURI = properties.getProperty("brokerURI");
            clientID = properties.getProperty("clientID");

            // Retrieve parameters
            maxThreshold = new AtomicReference<>(Float.parseFloat(properties.getProperty("maxThreshold")));
            minThreshold = new AtomicReference<>(Float.parseFloat(properties.getProperty("minThreshold")));
            safeThreshold = new AtomicReference<>(Float.parseFloat(properties.getProperty("safeThreshold")));
            taskPeriod = Integer.parseInt(properties.getProperty("taskPeriod"));
            taskUnitOfTime = TimeUnit.valueOf(properties.getProperty("taskUnitOfTime").toUpperCase());

            mode = new AtomicReference<>(OperativeMode.valueOf(properties.getProperty("mode").toUpperCase()));

            // Start automatic control unit scheduled daemon
            automaticControlUnit = Executors.newSingleThreadScheduledExecutor(r -> {
                Thread t = Executors.defaultThreadFactory().newThread(r);
                t.setDaemon(false);
                return t;
            });
            automaticControlUnit.scheduleAtFixedRate(new AutomaticControlUnit(), 0, taskPeriod, taskUnitOfTime);

        } catch (Exception ex) {
            System.out.println("Error during initialization: " + ex.getMessage());
            throw new RuntimeException(ex.getMessage());
        }
    }

    private ControlLogic(){ }

    /* Run-time parameter configuration  */
    public static float getMaxThreshold() {
        return maxThreshold.get();
    }

    public static void setMaxThreshold(float maxThreshold) throws ControlException {
        if(maxThreshold > safeThreshold.get() || maxThreshold < minThreshold.get())
            throw new ControlException("Value not consistent with the other thresholds");

        logger.info("Max threshold set to {}", maxThreshold);

        ControlLogic.maxThreshold.set(maxThreshold);
    }

    public static float getMinThreshold() {
        return minThreshold.get();
    }

    public static void setMinThreshold(float minThreshold) throws ControlException {
        if(minThreshold > maxThreshold.get() || minThreshold > safeThreshold.get())
            throw new ControlException("Value not consistent with the other thresholds");

        logger.info("Min threshold set to {}", minThreshold);

        ControlLogic.minThreshold.set(minThreshold);
    }

    public static float getSafeThreshold() {
        return safeThreshold.get();
    }

    public static void setSafeThreshold(float safeThreshold) throws ControlException {
        if(safeThreshold < minThreshold.get() || safeThreshold < maxThreshold.get())
            throw new ControlException("Value not consistent with the other thresholds");

        logger.info("Safe threshold set to {}", safeThreshold);

        ControlLogic.safeThreshold.set(safeThreshold);
    }


    /* Automatic control unit */
    public static OperativeMode startAutomaticControlUnit() throws ControlException {
        if(mode.get().equals(OperativeMode.AUTO)){
            throw new ControlException("Automatic control unit already started");
        }
        mode.set(OperativeMode.AUTO);

        logger.info("Control mode set to AUTO");

        return mode.get();
    }

    public static OperativeMode stopAutomaticControlUnit() throws ControlException {
        if(mode.get().equals(OperativeMode.MANUAL)){
            throw new ControlException("Automatic control unit already stopped");
        }
        mode.set(OperativeMode.MANUAL);
        logger.info("Control mode set to MANUAL");

        return mode.get();
    }

    public static void shutdown() throws ControlException{
        if(automaticControlUnit != null && !automaticControlUnit.isShutdown()){
            automaticControlUnit.shutdown();
            try {
                boolean shutdown = automaticControlUnit.awaitTermination(taskPeriod, taskUnitOfTime);
                if(!shutdown)
                    throw new ControlException("Some tasks were still running");
            } catch (InterruptedException ex) {
                throw new ControlException(ex.getMessage());
            }
        }
    }

    public static OperativeMode getRunningMode(){
        return mode.get();
    }


    /* Actuators control methods */

    /**
     * Set the opening level of the specified gate, sending the command to the gate controller only if the new level
     * is different from the current
     * @param gateName Name of the target gate
     * @param openingLevel Opening level to set
     * @throws PersistenceException In case of registry unavailable
     * @throws CoapException In case of failure of the request
     */
    public static void setGateOpening(String gateName, Integer openingLevel) throws PersistenceException, CoapException {
        Integer currentOpeningLevel = DataManager.getInstance().getResourceValue(gateName);
        if(!currentOpeningLevel.equals(openingLevel)){
            CoapManager.setGateOpening(gateName, openingLevel); // Update actuator
            DataManager.getInstance().setResourceValue(gateName, openingLevel); // Update database value (used by grafana)
            simulate(gateName, openingLevel); // Simulation only purpose
        }
    }

    /**
     * Decrease the opening level of the specified gate down to 0, sending the command to the gate controller only if
     * is possible to further decrease the opening level
     * @param gateName Name of the target gate
     * @param step Decrease step level
     * @throws PersistenceException In case of unavailable resource
     * @throws CoapException In case of failure of the request
     */
    public static void decreaseGateOpening(String gateName, Integer step) throws PersistenceException, CoapException {
        Integer currentOpeningLevel = DataManager.getInstance().getResourceValue(gateName);
        if(!currentOpeningLevel.equals(FULL_CLOSE) && step > 0){
            int newLevel = Math.max(currentOpeningLevel - step, FULL_CLOSE);

            // Update actuator
            CoapManager.setGateOpening(gateName, newLevel);

            // Update database value (used by grafana)
            DataManager.getInstance().setResourceValue(gateName, newLevel);

            // Simulation only purpose
            simulate(gateName, newLevel);
        }
    }

    /**
     * Increase the opening level of the specified gate up to 100, sending the command to the gate controller only if
     * is possible to further increase the opening level
     * @param gateName Name of the target gate
     * @param step Increase step level
     * @throws PersistenceException In case of unavailable resource
     * @throws CoapException In case of failure of the request
     */
    public static void increaseGateOpening(String gateName, Integer step) throws PersistenceException, CoapException {
        Integer currentOpeningLevel = DataManager.getInstance().getResourceValue(gateName);
        if(!currentOpeningLevel.equals(FULL_OPEN) && step > 0){
            int newLevel = Math.min(currentOpeningLevel + step, FULL_OPEN);

            // Update actuator
            CoapManager.setGateOpening(gateName, newLevel);

            // Update database value (used by grafana)
            DataManager.getInstance().setResourceValue(gateName, newLevel);

            // Simulation only purpose
            simulate(gateName, newLevel);
        }
    }

    /**
     * Set the status of the alarm to the desired value, sending the command to alarm controller only if the status
     * is different from the current value
     * @param status Status to set (true = on, false = off)
     * @throws PersistenceException In case of registry unavailable
     * @throws CoapException In case of failure of the request
     */
    public static void setAlarmStatus(Boolean status) throws PersistenceException, CoapException {
        for(Resource alarm: DataManager.getInstance().alarmResources()){
            boolean currentStatus = DataManager.getInstance().getResourceValue(alarm.getName()) == 1;
            if(currentStatus != status){
                CoapManager.setAlarmStatus(status);
                DataManager.getInstance().setResourceValue(alarm.getName(), status ? 1:0);
            }
        }
    }


    /* Simulation only */
    private static void simulate(String gate, int level){
        try(MqttClient client = new MqttClient(brokerURI, clientID)) {
            client.connect();
            client.publish("simulation",
                    new MqttMessage(new ObjectMapper().createObjectNode()
                            .put("gate-" + gate, level)
                            .toString()
                            .getBytes()));
            client.disconnect();
        } catch (MqttException ex) { logger.warn("Simulation: {}", ex.getMessage()); }
    }
}
