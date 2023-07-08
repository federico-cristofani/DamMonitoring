package it.unipi.iot.damonitoring;

import it.unipi.iot.damonitoring.exceptions.PersistenceException;
import org.junit.Test;

public class DataManagerTest{

    @Test
    public void connectionTest() {
        try {
            DataManager dataManager = DataManager.getInstance();
            System.out.println(dataManager.retrieveOutFlowRate());
            DataManager.close();
        }
        catch (PersistenceException | RuntimeException ex){
            ex.printStackTrace();
            throw new RuntimeException("Exception: " + ex.getMessage());
        }
    }
}
