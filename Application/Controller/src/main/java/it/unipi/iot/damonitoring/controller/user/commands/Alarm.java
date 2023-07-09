package it.unipi.iot.damonitoring.controller.user.commands;

import it.unipi.iot.damonitoring.DataManager;
import it.unipi.iot.damonitoring.controller.coap.CoapManager;
import it.unipi.iot.damonitoring.entities.Resource;
import picocli.CommandLine.*;

@Command(
        name = "Alarm",
        mixinStandardHelpOptions = true,
        description = "Show the current alarm status.",
        version = "Controller 1.0"
)
public class Alarm implements BaseCommand {

    @Override
    public Integer call() throws Exception {
        for(Resource alarm: DataManager.getInstance().alarmResources()){
            System.out.printf("* %s: %s%n",
                    alarm.getName().toUpperCase(),
                    CoapManager.getAlarmStatus(alarm.getUri()) ? "On": "Off");
        }

        return 0;
    }
}
