package it.unipi.iot.damonitoring.collector;

import it.unipi.iot.damonitoring.collector.resources.Registry;
import org.eclipse.californium.core.CoapServer;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class RegistrationServer {
    private static final Logger logger = LoggerFactory.getLogger(RegistrationServer.class);
    private static final String REGISTRY  = "registry";
    private static RegistrationServer instance;
    private final CoapServer server;
    public static RegistrationServer getInstance(){
        if(instance == null){
            instance = new RegistrationServer();
        }
        return instance;
    }
    private RegistrationServer(){
        server = new CoapServer();
        server.add(new Registry(REGISTRY));
    }
    public void start(){
        server.start();
        logger.info("Registration server started");
    }

    public void stop(){
        server.stop();
        logger.info("Registration server stopped");
    }
}
