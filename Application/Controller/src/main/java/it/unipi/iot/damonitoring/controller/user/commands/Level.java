package it.unipi.iot.damonitoring.controller.user.commands;
import it.unipi.iot.damonitoring.DataManager;
import it.unipi.iot.damonitoring.entities.WaterLevel;
import it.unipi.iot.damonitoring.exceptions.PersistenceException;
import picocli.CommandLine.Command;

import java.text.SimpleDateFormat;

@Command(
        name = "Level",
        description = "Show the current water level",
        mixinStandardHelpOptions = true,
        version = "Controller 1.0"
)
public class Level implements BaseCommand {

    private static final SimpleDateFormat formatter = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");

    @Override
    public Integer call() throws Exception {
        lastRecord();
        return 0;
    }

    private void lastRecord() throws PersistenceException {
        WaterLevel waterLevel = DataManager.getInstance().retrieveWaterLevel();

        if(waterLevel == null){
            System.out.println("No available data");
            return;
        }
        System.out.printf("Water level at %s = %.2f [m]%n", formatter.format(waterLevel.getTimestamp()),
                waterLevel.getValue());
    }
}
