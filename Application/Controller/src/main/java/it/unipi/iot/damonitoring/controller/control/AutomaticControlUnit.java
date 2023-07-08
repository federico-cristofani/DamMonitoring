package it.unipi.iot.damonitoring.controller.control;

import it.unipi.iot.damonitoring.DataManager;
import it.unipi.iot.damonitoring.controller.coap.exceptions.CoapException;
import it.unipi.iot.damonitoring.controller.control.enumerates.OperativeMode;
import it.unipi.iot.damonitoring.entities.Resource;
import it.unipi.iot.damonitoring.entities.WaterLevel;
import it.unipi.iot.damonitoring.exceptions.PersistenceException;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.IOException;
import java.io.InputStream;
import java.time.LocalDateTime;
import java.time.OffsetDateTime;
import java.time.temporal.ChronoUnit;
import java.util.Date;
import java.util.List;
import java.util.Properties;

import static it.unipi.iot.damonitoring.controller.control.ControlLogic.*;


public class AutomaticControlUnit implements Runnable {

    private static final String CONFIG_FILE = "/controller.properties";
    private static final Integer STEP = 5;
    private static final Integer LONG_STEP = 10;
    private static final Integer SHORT_STEP = 1;
    private static final String INFLOW = "inflow";
    private static final Logger logger = LoggerFactory.getLogger(AutomaticControlUnit.class);
    private static final Integer maxStale;

    static {
        try (InputStream inputStream = AutomaticControlUnit.class.getResourceAsStream(CONFIG_FILE)){
            Properties properties = new Properties();
            properties.load(inputStream);

            maxStale = Integer.parseInt(properties.getProperty("maxStale"));
        }
        catch (IOException | IllegalArgumentException ex){
            System.out.println("Error during initialization: " + ex.getMessage());
            throw new RuntimeException(ex);
        }
    }

    public static void init(){ }
    @Override
    public void run() {
        try {
            // Retrieve water level
            WaterLevel waterLevel = DataManager.getInstance().retrieveWaterLevel();
            Float inflowRate = DataManager.getInstance().retrieveFlowRate(INFLOW).getValue();
            Float outFlowRate = DataManager.getInstance().retrieveOutFlowRate();

            // Check data timestamp
            Date fiveMinutesAgo = Date.from(LocalDateTime
                    .now()
                    .minus(maxStale, ChronoUnit.MINUTES)
                    .toInstant(OffsetDateTime.now().getOffset()));

            // Do nothing if no fresh data are available
            if(waterLevel == null || waterLevel.getTimestamp().before(fiveMinutesAgo)){
                logger.warn("No fresh data");
                return;
            }

            logger.info("Water level: {}", waterLevel);
            logger.info("Inflow: {}, Outflow: {}", inflowRate, outFlowRate);

            // Turn on\off alarm
            ControlLogic.setAlarmStatus(waterLevel.getValue() > ControlLogic.getSafeThreshold());

            // If running mode is auto not action is taken
            if(ControlLogic.getRunningMode().equals(OperativeMode.MANUAL)){
                return;
            }

            // Retrieve available gates
            List<Resource> onlineGates = DataManager.getInstance().gateResources();

            if(waterLevel.getValue() > ControlLogic.getSafeThreshold()){
                // Open all gates (those not already opened)
                for(Resource gate: onlineGates){
                    ControlLogic.setGateOpening(gate.getName(), FULL_OPEN);
                }
            }
            else if(waterLevel.getValue() > ControlLogic.getMaxThreshold()){

                int step = inflowRate > outFlowRate ? LONG_STEP:STEP;

                for(Resource gate: onlineGates){
                    if(gate.getTag().equalsIgnoreCase("emergency")){
                        // Close emergency gates (those not already closed)
                        ControlLogic.setGateOpening(gate.getName(), FULL_CLOSE);
                    }
                    else{
                        // Increase opening level of primary gates (if not already to 100)
                        ControlLogic.increaseGateOpening(gate.getName(), step);
                    }
                }
            }
            else if (waterLevel.getValue() < ControlLogic.getMinThreshold()) {

                int step = inflowRate < outFlowRate ? LONG_STEP:STEP;

                for(Resource gate: onlineGates){
                    // Close emergency gates (those not already closed)
                    if(gate.getTag().equalsIgnoreCase("emergency")){
                        ControlLogic.setGateOpening(gate.getName(), FULL_CLOSE);
                    }
                    else{
                        // Decrease opening level of primary gates (if not already to 0)
                        ControlLogic.decreaseGateOpening(gate.getName(), step);
                    }
                }
            }
            else {
                int step;
                if(Math.abs(outFlowRate - inflowRate) > 20){
                    step = LONG_STEP;
                }
                else if (Math.abs(outFlowRate - inflowRate) > 10){
                    step = STEP;
                }
                else {
                    step = SHORT_STEP;
                }

                for(Resource gate: onlineGates){
                    // Close emergency gates (those not already closed)
                    if(gate.getTag().equalsIgnoreCase("emergency")){
                        ControlLogic.setGateOpening(gate.getName(), FULL_CLOSE);
                    }
                    else{
                        if(outFlowRate < inflowRate - 1){
                            // Increase opening level of primary gates (if not already to 100)
                            ControlLogic.increaseGateOpening(gate.getName(), step);
                        }
                        else if(outFlowRate > inflowRate + 1){
                            // Reduce opening level of primary gates (if not already to 0)
                            ControlLogic.decreaseGateOpening(gate.getName(), step);
                        }
                    }
                }
            }
        } catch (PersistenceException ex) {
            logger.error("Database error: {}", ex.getMessage());
        }
        catch (CoapException ex){
            logger.error("Server error: {}", ex.getMessage());
        }
        catch (Exception ex){
            logger.error("Error: {}", ex.getMessage());
        }
    }
}
