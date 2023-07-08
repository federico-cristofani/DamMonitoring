package it.unipi.iot.damonitoring.controller.coap;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import it.unipi.iot.damonitoring.DataManager;
import it.unipi.iot.damonitoring.controller.coap.exceptions.CoapException;
import it.unipi.iot.damonitoring.entities.Resource;
import it.unipi.iot.damonitoring.exceptions.PersistenceException;

import org.eclipse.californium.core.CoapClient;
import org.eclipse.californium.core.CoapResponse;
import org.eclipse.californium.core.coap.CoAP;
import org.eclipse.californium.core.coap.MediaTypeRegistry;
import org.eclipse.californium.elements.exception.ConnectorException;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.IOException;

public final class CoapManager {

    private static final Logger logger = LoggerFactory.getLogger(CoapManager.class);
    private static final String ALARM = "alarm";
    private static final ObjectMapper mapper = new ObjectMapper();

    private CoapManager(){ }

    /**
     * Retrieve resource URI from registry stored in the database
     * @param  resName Resource friendly name
     * @return Resource URI
     * @throws CoapException If the resource is not available
     * @throws PersistenceException If the registry is not accessible
     */
    static private String URILookup(String resName) throws CoapException, PersistenceException {
        Resource resource = DataManager.getInstance().lookupResource(resName);
        if(resource == null)
            throw new CoapException("Resource unavailable");

        return resource.getUri();
    }

    /**
     * Retrieve the current status of the specified gate
     * @param gate Gate name
     * @return Opening level expressed in percentage (0-100)
     * @throws CoapException In case of failure of the request
     */
    public static int getGateOpening(String gate) throws CoapException {
        try {
            // Send GET request
            CoapClient client = new CoapClient(URILookup(gate));
            CoapResponse response = client.get();

            // Check response code
            if(response == null || !response.isSuccess())
                throw new CoapException("Request failed: " + ((response == null) ?
                        "no response from server" : response.getCode().name()));

            // Parse response
            JsonNode node = mapper.readTree(response.getPayload());
            String name = node.get("n").asText();
            String unit = node.get("u").asText();
            int value = node.get("v").asInt();

            // Check expected values
            if(!name.equals("opening_level") && !unit.equals("percent")){
                throw new CoapException("Bad record");
            }

            logger.info("Gate \"{}\" opening level {}", gate, value);

            return value;
        } catch (ConnectorException | IOException | PersistenceException ex) {
            throw new CoapException(gate.toUpperCase() + ": " + ex.getMessage());
        }
    }

    /**
     * Set the opening level of the specified gate
     * @param gate Gate name
     * @param level Target opening level
     * @return Success message received from the server
     * @throws CoapException In case of failure of the request
     */
    public static String setGateOpening(String gate, Integer level) throws CoapException{
        try {

            // Build request payload
            String requestPayload = mapper
                    .createObjectNode()
                    .put("opening_level", level)
                    .toString();

            // Send request
            String message = coapPutRequest(gate, requestPayload);

            logger.info("Gate \"{}\" opening level set to {}", gate, level);

            return message;

        } catch (ConnectorException | IOException | PersistenceException ex) {
            throw new CoapException(ex.getMessage());
        }
    }

    /**
     * Retrieve the current status of the alarm
     * @return Boolean value indicating if the status of the alarm (true = on, false = off)
     * @throws CoapException In case of failure of the request
     */
    public static Boolean getAlarmStatus() throws CoapException {
        try {
            // Send GET request
            CoapClient client = new CoapClient(URILookup(ALARM));
            CoapResponse response = client.get();

            // Check response code
            if(response == null || !response.isSuccess())
                throw new CoapException("Request failed: " + ((response == null) ?
                        "no response from server" : response.getCode().name()));

            // Parse response
            JsonNode node = mapper.readTree(response.getPayload());
            String name = node.get("n").asText();
            boolean value = node.get("vb").asBoolean();

            // Check expected values
            if(!name.equals("state")){
                throw new CoapException("Bad record");
            }

            return value;
        } catch (ConnectorException | IOException | PersistenceException ex) {
            throw new CoapException("Alarm: " + ex.getMessage());
        }
    }

    /**
     * Set the alarm status
     * @param state Target status of the alarm (true = on, false = off)
     * @return Success message received from the server
     * @throws CoapException In case of failure of the request
     */
    public static String setAlarmStatus(Boolean state) throws CoapException {
        try {

            // Build request payload
            String requestPayload = mapper
                    .createObjectNode()
                    .put("state", state)
                    .toString();

            // Send request
            String message = coapPutRequest(ALARM, requestPayload);

            logger.info("Alarm {}", state);

            return message;

        } catch (ConnectorException | IOException | PersistenceException ex) {
            throw new CoapException(ex.getMessage());
        }
    }

    /**
     * Utility function to send PUT requests
     * @param resource Resource URI
     * @param requestPayload Request payload
     * @return Success message received from the server
     * @throws CoapException In case of failure of the request
     * @throws IOException In case of failure of the request
     * @throws ConnectorException In case of failure of the request
     * @throws PersistenceException If the registry is not accessible
     */
    private static String coapPutRequest(String resource, String requestPayload) throws CoapException,
            PersistenceException, IOException, ConnectorException {

        CoapClient client = new CoapClient(URILookup(resource));

        // Send PUT request
        CoapResponse response = client.put(requestPayload, MediaTypeRegistry.APPLICATION_JSON);

        // Check response code
        String message;
        if(response != null && (response.isSuccess() || response.getCode().equals(CoAP.ResponseCode.BAD_REQUEST))){
            // Parse response
            message = mapper.readTree(response.getResponseText()).get("message").asText();
        }
        else { // Empty response
            message = "Request failed: " + ((response == null) ? "no response from server" : response.getCode().name());
        }

        if(response == null || !response.isSuccess()){
            throw new CoapException(message);
        }

        return message;

    }
}