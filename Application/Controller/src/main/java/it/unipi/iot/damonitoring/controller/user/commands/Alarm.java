package it.unipi.iot.damonitoring.controller.user.commands;

import it.unipi.iot.damonitoring.controller.coap.CoapManager;
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
        System.out.println("Alarm: " + (CoapManager.getAlarmStatus() ? "On":"Off"));
        return 0;
    }
}
