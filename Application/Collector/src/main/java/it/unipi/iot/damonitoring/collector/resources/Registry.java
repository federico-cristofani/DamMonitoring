package it.unipi.iot.damonitoring.collector.resources;

import com.fasterxml.jackson.databind.ObjectMapper;
import it.unipi.iot.damonitoring.DataManager;
import it.unipi.iot.damonitoring.entities.Resource;
import it.unipi.iot.damonitoring.exceptions.PersistenceException;
import org.eclipse.californium.core.CoapResource;
import org.eclipse.californium.core.coap.CoAP;
import org.eclipse.californium.core.server.resources.CoapExchange;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.IOException;
import java.util.List;

public class Registry extends CoapResource{
    private static final Logger logger = LoggerFactory.getLogger(Registry.class);

    public Registry(String name) {
        super(name);
    }

    @Override
    public void handlePOST(CoapExchange exchange) {
        logger.info("Registration request from: <{}>", exchange.getSourceAddress());
        try {
            ObjectMapper mapper = new ObjectMapper();
            List<Resource> resources = mapper.readValue(
                    exchange.getRequestPayload(),
                    mapper.getTypeFactory().constructCollectionType(List.class, Resource.class)
            );
            DataManager.getInstance().addResources(resources);
            exchange.respond(CoAP.ResponseCode.CREATED);
        } catch (IOException | PersistenceException ex) {
            logger.error("Registration: {}", ex.getMessage());
            exchange.respond(CoAP.ResponseCode.INTERNAL_SERVER_ERROR);
        }
    }
}
